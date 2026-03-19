"""
Roboterarm - KI-Training
==========================
Training in zwei Modi:

  [1] Virtuell trainieren   (Seg + Pose auf Blender-Daten)
  [2] Echt trainieren       (Seg + Pose auf echten Kamerabildern)
  [3] Validierung           (Metriken für bestes Modell)
  [4] Export                (ONNX für Edge-Deployment)

Datenaufbereitung → siehe Umrisse_in_Polygone.py

Aufruf:
  python EdgeAI.py          → Menüauswahl
  python EdgeAI.py 1        → Direkt Schritt 1
"""

import os
import sys
import shutil
import random
from pathlib import Path
from typing import Optional

from ultralytics import YOLO

# Intel XPU ist in ultralytics 8.4.x noch nicht vollständig unterstützt.
DEVICE = "cpu"

# --- KONFIGURATION ---

# Pfade
DATASET_DIR = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\Segmentierung per Blender\dataset")
YOLO_DATASET_DIR = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\Segmentierung per Blender\yolo_dataset")
DATASET_YAML = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\Segmentierung per Blender\dataset.yaml")
MODEL_OUTPUT_DIR = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Trained_Models\AI-Models")
EXPORT_DIR = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Edge Export")

# Pose (Blender)
POSE_DATASET_DIR = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\Segmentierung per Blender\pose_dataset")
POSE_YAML = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\Segmentierung per Blender\pose_dataset.yaml")

# Echt (Seg Fine-Tuning)
PRETRAINED_SEG = MODEL_OUTPUT_DIR / "roboterarm_seg" / "weights" / "best.pt"
REAL_YOLO_DIR = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\real_yolo_dataset")
REAL_YAML = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\real_dataset.yaml")
REAL_DATASET_DIR = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\real_dataset")

# Echt (Pose)
PRETRAINED_POSE = MODEL_OUTPUT_DIR / "roboterarm_pose" / "weights" / "best.pt"
REAL_POSE_YAML = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\real_pose_dataset.yaml")
REAL_POSE_DATASET_DIR = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\real_pose_dataset")

# Allgemeine Parameter
IMG_SIZE = 640
BATCH_SIZE = 16
VAL_SPLIT = 0.3
SEED = 42
PRUNE_RATIO = 0.20
PRUNE_RECOVERY_EPOCHS = 10
CALIBRATION_IMAGES = 64

# ---------------------


# ═════════════════════════════════════════════
# HILFSFUNKTIONEN
# ═════════════════════════════════════════════

def _best_weights(run_name: str) -> Path:
    return MODEL_OUTPUT_DIR / run_name / "weights" / "best.pt"


def _ask_optimization_mode() -> bool:
    """Fragt den Trainingsmodus ab: Standard oder mit Optimierungen."""
    print("\nTrainingsmodus:")
    print("  [1] Standard (wie bisher)")
    print("  [2] Optimiert (Structured Pruning + INT8 ONNX)")
    raw = input("Modus wählen (1/2, Enter=1): ").strip()
    return raw == "2"


def _resolve_opt_flag_from_args(args: list[str]) -> Optional[bool]:
    """Liest optionale CLI-Flags für Optimierungsmodus."""
    if "--opt" in args:
        return True
    if "--standard" in args:
        return False
    return None


def _apply_structured_pruning_and_recover(
    source_weights: Path,
    data_yaml: Path,
    output_run_name: str,
    prune_ratio: float = PRUNE_RATIO,
    recovery_epochs: int = PRUNE_RECOVERY_EPOCHS,
) -> Optional[Path]:
    """Pruned Conv-Kanäle (structured) und trainiert kurz zur Erholung nach."""
    if not source_weights.exists():
        print(f"WARNUNG: Pruning übersprungen, Modell fehlt: {source_weights}")
        return None

    try:
        import torch
        import torch.nn.utils.prune as prune
    except ImportError:
        print("WARNUNG: torch/prune nicht verfügbar, Pruning wird übersprungen.")
        return None

    print("\n" + "-" * 50)
    print(f"Pruning startet: {source_weights.name} (ratio={prune_ratio:.2f})")
    print("-" * 50)

    model = YOLO(str(source_weights))
    conv_count = 0

    for module in model.model.modules():
        if isinstance(module, torch.nn.Conv2d):
            prune.ln_structured(module, name="weight", amount=prune_ratio, n=2, dim=0)
            prune.remove(module, "weight")
            conv_count += 1

    if conv_count == 0:
        print("WARNUNG: Keine Conv2d-Layer gefunden, Pruning ohne Effekt.")

    print(f"Pruning auf {conv_count} Conv-Layern angewendet.")
    print("Recovery-Fine-Tuning startet...")

    model.train(
        data=str(data_yaml),
        device=DEVICE,
        epochs=recovery_epochs,
        imgsz=IMG_SIZE,
        batch=BATCH_SIZE,
        project=str(MODEL_OUTPUT_DIR),
        name=output_run_name,
        exist_ok=True,
        seed=SEED,
        lr0=0.0005,
        lrf=0.01,
        mosaic=0.0,
        patience=max(5, recovery_epochs // 2),
    )

    result = _best_weights(output_run_name)
    if result.exists():
        print(f"Pruning+Recovery abgeschlossen: {result}")
        return result

    print("WARNUNG: Kein best.pt nach Pruning-Recovery gefunden.")
    return None


def _export_int8_onnx(source_weights: Path, calibration_dir: Path, export_name: str) -> Optional[Path]:
    """Exportiert ONNX und quantisiert mit ONNX Runtime auf INT8 (static)."""
    if not source_weights.exists():
        print(f"WARNUNG: Quantisierung übersprungen, Modell fehlt: {source_weights}")
        return None

    if not calibration_dir.exists():
        print(f"WARNUNG: Kalibrierbilder fehlen, Quantisierung übersprungen: {calibration_dir}")
        return None

    try:
        import cv2
        import numpy as np
        import onnxruntime as ort
        from onnxruntime.quantization import (
            CalibrationDataReader,
            CalibrationMethod,
            QuantFormat,
            QuantType,
            quantize_static,
        )
    except ImportError:
        print("WARNUNG: onnxruntime/cv2/numpy nicht vollständig verfügbar, INT8 wird übersprungen.")
        return None

    print("\n" + "-" * 50)
    print(f"ONNX-Export + INT8-Quantisierung: {source_weights.name}")
    print("-" * 50)

    model = YOLO(str(source_weights))
    onnx_fp_path = Path(model.export(format="onnx", imgsz=IMG_SIZE, opset=20, simplify=True))
    EXPORT_DIR.mkdir(parents=True, exist_ok=True)

    quant_out = EXPORT_DIR / f"{export_name}_int8.onnx"

    images = []
    for ext in ("*.png", "*.jpg", "*.jpeg", "*.bmp"):
        images.extend(sorted(calibration_dir.glob(ext)))
    images = images[:CALIBRATION_IMAGES]

    if not images:
        print("WARNUNG: Keine Kalibrierbilder gefunden, INT8 wird übersprungen.")
        return None

    session = ort.InferenceSession(str(onnx_fp_path), providers=["CPUExecutionProvider"])
    input_name = session.get_inputs()[0].name

    class _Reader(CalibrationDataReader):
        def __init__(self, image_paths, input_tensor_name):
            self.image_paths = image_paths
            self.input_name = input_tensor_name
            self.index = 0

        def _preprocess(self, path):
            img = cv2.imread(str(path))
            if img is None:
                return None
            img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
            img = cv2.resize(img, (IMG_SIZE, IMG_SIZE), interpolation=cv2.INTER_LINEAR)
            img = img.astype(np.float32) / 255.0
            img = np.transpose(img, (2, 0, 1))
            return np.expand_dims(img, axis=0)

        def get_next(self):
            while self.index < len(self.image_paths):
                arr = self._preprocess(self.image_paths[self.index])
                self.index += 1
                if arr is not None:
                    return {self.input_name: arr}
            return None

    reader = _Reader(images, input_name)

    quantize_static(
        model_input=str(onnx_fp_path),
        model_output=str(quant_out),
        calibration_data_reader=reader,
        quant_format=QuantFormat.QDQ,
        activation_type=QuantType.QUInt8,
        weight_type=QuantType.QInt8,
        per_channel=True,
        reduce_range=False,
        calibrate_method=CalibrationMethod.MinMax,
    )

    print(f"FP-ONNX: {onnx_fp_path}")
    print(f"INT8-ONNX: {quant_out}")
    return quant_out


def _run_optimization_pipeline(
    model_name: str,
    source_weights: Path,
    data_yaml: Path,
    calibration_dir: Path,
    optimized_run_name: str,
) -> None:
    """Führt optionales Pruning + INT8-Export für ein trainiertes Modell aus."""
    print("\n" + "=" * 50)
    print(f"OPTIMIERUNG: {model_name}")
    print("=" * 50)

    optimized_weights = _apply_structured_pruning_and_recover(
        source_weights=source_weights,
        data_yaml=data_yaml,
        output_run_name=optimized_run_name,
    )
    if optimized_weights is None:
        print("Optimierung abgebrochen (kein pruned Modell).")
        return

    _export_int8_onnx(
        source_weights=optimized_weights,
        calibration_dir=calibration_dir,
        export_name=optimized_run_name,
    )

def _prepare_seg_dataset():
    """Erstellt YOLO-Ordnerstruktur und verteilt Bilder + Labels in Train/Val."""
    random.seed(SEED)

    for split in ["train", "val"]:
        (YOLO_DATASET_DIR / "images" / split).mkdir(parents=True, exist_ok=True)
        (YOLO_DATASET_DIR / "labels" / split).mkdir(parents=True, exist_ok=True)

    image_files = sorted(DATASET_DIR.glob("image_*.png"))
    print(f"Gefunden: {len(image_files)} Bilder")

    if len(image_files) == 0:
        raise FileNotFoundError(f"Keine Bilder in {DATASET_DIR} gefunden!")

    indices = list(range(len(image_files)))
    random.shuffle(indices)
    val_count = int(len(indices) * VAL_SPLIT)
    val_indices = set(indices[:val_count])

    train_count = 0
    val_count_actual = 0

    for i, img_path in enumerate(image_files):
        label_name = img_path.stem + ".txt"
        label_path = DATASET_DIR / label_name

        if not label_path.exists():
            print(f"WARNUNG: Kein Label für {img_path.name}, überspringe...")
            continue

        split = "val" if i in val_indices else "train"
        shutil.copy2(img_path, YOLO_DATASET_DIR / "images" / split / img_path.name)
        shutil.copy2(label_path, YOLO_DATASET_DIR / "labels" / split / label_name)

        if split == "train":
            train_count += 1
        else:
            val_count_actual += 1

    print(f"Dataset aufgeteilt: {train_count} Train, {val_count_actual} Val")


def _train_seg_blender():
    """Trainiert YOLO-Seg auf Blender-Daten."""
    print("\n" + "=" * 50)
    print("SEGMENTIERUNG — Blender-Daten")
    print("=" * 50)

    print("Dataset wird vorbereitet...")
    _prepare_seg_dataset()

    print("\nTraining wird gestartet...")
    model = YOLO("yolo11n-seg.pt")
    model.train(
        data=str(DATASET_YAML),
        device=DEVICE,
        epochs=40,
        imgsz=IMG_SIZE,
        batch=BATCH_SIZE,
        project=str(MODEL_OUTPUT_DIR),
        name="roboterarm_seg",
        exist_ok=True,
        seed=SEED,
        hsv_h=0.01, hsv_s=0.3, hsv_v=0.3,
        degrees=15.0, translate=0.1, scale=0.3,
        fliplr=0.5, flipud=0.0,
        mosaic=0.5,
        patience=20,
    )
    print(f"Seg-Modell fertig: {MODEL_OUTPUT_DIR / 'roboterarm_seg'}")


def _train_pose_blender():
    """Trainiert YOLO-Pose auf Blender-Daten."""
    print("\n" + "=" * 50)
    print("POSE — Blender-Daten")
    print("=" * 50)

    if not POSE_YAML.exists():
        print(f"FEHLER: {POSE_YAML} nicht gefunden!")
        print("Bitte zuerst Umrisse_in_Polygone.py [1] ausführen.")
        return

    train_imgs = list((POSE_DATASET_DIR / "images" / "train").glob("*.png"))
    val_imgs = list((POSE_DATASET_DIR / "images" / "val").glob("*.png"))
    if not train_imgs:
        print("FEHLER: Keine Trainingsbilder in pose_dataset/!")
        print("Bitte zuerst Umrisse_in_Polygone.py [1] ausführen.")
        return

    print(f"Dataset: {len(train_imgs)} Train, {len(val_imgs)} Val")

    model = YOLO("yolo11n-pose.pt")
    model.train(
        data=str(POSE_YAML),
        device=DEVICE,
        epochs=40,
        imgsz=IMG_SIZE,
        batch=BATCH_SIZE,
        project=str(MODEL_OUTPUT_DIR),
        name="roboterarm_pose",
        exist_ok=True,
        seed=SEED,
        lr0=0.01, lrf=0.01,
        degrees=15.0, translate=0.1, scale=0.3,
        fliplr=0.5,
        mosaic=0.5,
        patience=20,
    )
    print(f"Pose-Modell fertig: {MODEL_OUTPUT_DIR / 'roboterarm_pose'}")


def _finetune_seg_real():
    """Fine-Tuning: Blender-Seg-Modell auf echten Daten."""
    print("\n" + "=" * 50)
    print("SEGMENTIERUNG — Echte Bilder (Fine-Tuning)")
    print("=" * 50)

    if not PRETRAINED_SEG.exists():
        print(f"FEHLER: Kein vortrainiertes Seg-Modell unter {PRETRAINED_SEG}")
        print("Bitte zuerst [1] Virtuell trainieren.")
        return False

    yaml_content = f"""path: {REAL_YOLO_DIR}
train: images/train
val: images/val

names:
  0: Base
  1: Joint_1
  2: Joint_2
  3: Joint_3
  4: Joint_4_Finger
"""
    with open(REAL_YAML, "w") as f:
        f.write(yaml_content)

    random.seed(SEED)
    src_images = REAL_DATASET_DIR / "images" / "all"
    src_labels = REAL_DATASET_DIR / "labels" / "all"

    if not src_images.exists():
        print(f"FEHLER: {src_images} nicht gefunden!")
        print("Bitte zuerst Umrisse_in_Polygone.py [2] ausführen.")
        return False

    valid_images = []
    for img_path in sorted(src_images.glob("*.png")) + sorted(src_images.glob("*.jpg")):
        label_path = src_labels / (img_path.stem + ".txt")
        if label_path.exists() and label_path.stat().st_size > 0:
            valid_images.append(img_path)

    print(f"Bilder mit Labels: {len(valid_images)}")
    if len(valid_images) == 0:
        print("FEHLER: Keine annotierten Bilder gefunden!")
        return False

    random.shuffle(valid_images)
    val_count = int(len(valid_images) * VAL_SPLIT)
    val_images = valid_images[:val_count]
    train_images = valid_images[val_count:]

    for split, images in [("train", train_images), ("val", val_images)]:
        img_out = REAL_YOLO_DIR / "images" / split
        lbl_out = REAL_YOLO_DIR / "labels" / split
        img_out.mkdir(parents=True, exist_ok=True)
        lbl_out.mkdir(parents=True, exist_ok=True)

        for img_path in images:
            shutil.copy2(img_path, img_out / img_path.name)
            shutil.copy2(src_labels / (img_path.stem + ".txt"), lbl_out / (img_path.stem + ".txt"))

    print(f"Train: {len(train_images)}, Val: {len(val_images)}")

    model = YOLO(str(PRETRAINED_SEG))
    model.train(
        data=str(REAL_YAML),
        device=DEVICE,
        epochs=5,
        imgsz=IMG_SIZE,
        batch=BATCH_SIZE,
        project=str(MODEL_OUTPUT_DIR),
        name="roboterarm_real",
        exist_ok=True,
        seed=SEED,
        lr0=0.001, lrf=0.01,
        freeze=10,
        hsv_h=0.015, hsv_s=0.5, hsv_v=0.4,
        degrees=10.0, translate=0.15, scale=0.4,
        fliplr=0.5, flipud=0.0,
        mosaic=0.8, mixup=0.1,
        patience=15,
    )
    print(f"Seg-Modell (Real) fertig: {MODEL_OUTPUT_DIR / 'roboterarm_real'}")
    return True


def _train_pose_real():
    """Trainiert YOLO-Pose auf echten Bildern."""
    print("\n" + "=" * 50)
    print("POSE — Echte Bilder")
    print("=" * 50)

    if not REAL_POSE_YAML.exists():
        print(f"FEHLER: {REAL_POSE_YAML} nicht gefunden!")
        print("Bitte zuerst Umrisse_in_Polygone.py [2] ausführen.")
        return

    train_imgs = list((REAL_POSE_DATASET_DIR / "images" / "train").glob("*.png"))
    val_imgs = list((REAL_POSE_DATASET_DIR / "images" / "val").glob("*.png"))
    if not train_imgs:
        print("FEHLER: Keine Trainingsbilder in real_pose_dataset/!")
        print("Bitte zuerst Umrisse_in_Polygone.py [2] ausführen.")
        return

    print(f"Dataset: {len(train_imgs)} Train, {len(val_imgs)} Val")

    if PRETRAINED_POSE.exists():
        base_model = str(PRETRAINED_POSE)
        print(f"Starte von Blender-Pose-Modell: {base_model}")
        freeze = 10
        lr0 = 0.001
    else:
        base_model = "yolo11n-pose.pt"
        print(f"Kein Blender-Pose-Modell gefunden, starte von: {base_model}")
        freeze = 0
        lr0 = 0.01

    model = YOLO(base_model)
    model.train(
        data=str(REAL_POSE_YAML),
        device=DEVICE,
        epochs=50,
        imgsz=IMG_SIZE,
        batch=BATCH_SIZE,
        project=str(MODEL_OUTPUT_DIR),
        name="roboterarm_pose_real",
        exist_ok=True,
        seed=SEED,
        lr0=lr0, lrf=0.01,
        freeze=freeze,
        degrees=10.0, translate=0.1, scale=0.3,
        fliplr=0.5,
        mosaic=0.5,
        patience=15,
    )
    print(f"Pose-Modell (Real) fertig: {MODEL_OUTPUT_DIR / 'roboterarm_pose_real'}")


# ═════════════════════════════════════════════
# HAUPTFUNKTIONEN (Menü-Einträge)
# ═════════════════════════════════════════════

def train_virtuell(enable_optimizations: bool = False):
    """[1] Virtuell: Seg + Pose auf Blender-Daten nacheinander trainieren."""
    print("\n" + "#" * 50)
    print("  VIRTUELL TRAINIEREN (Seg + Pose)")
    print("#" * 50)

    _train_seg_blender()
    _train_pose_blender()

    if enable_optimizations:
        _run_optimization_pipeline(
            model_name="Seg (Blender)",
            source_weights=_best_weights("roboterarm_seg"),
            data_yaml=DATASET_YAML,
            calibration_dir=YOLO_DATASET_DIR / "images" / "train",
            optimized_run_name="roboterarm_seg_opt",
        )
        _run_optimization_pipeline(
            model_name="Pose (Blender)",
            source_weights=_best_weights("roboterarm_pose"),
            data_yaml=POSE_YAML,
            calibration_dir=POSE_DATASET_DIR / "images" / "train",
            optimized_run_name="roboterarm_pose_opt",
        )

    print("\n" + "#" * 50)
    print("  VIRTUELL KOMPLETT!")
    print(f"  Seg:  {MODEL_OUTPUT_DIR / 'roboterarm_seg'}")
    print(f"  Pose: {MODEL_OUTPUT_DIR / 'roboterarm_pose'}")
    print("#" * 50)


def train_echt(enable_optimizations: bool = False):
    """[2] Echt: Seg Fine-Tuning + Pose auf echten Bildern nacheinander trainieren."""
    print("\n" + "#" * 50)
    print("  ECHT TRAINIEREN (Seg + Pose)")
    print("#" * 50)

    success = _finetune_seg_real()
    if success is False:
        print("\nSeg-Training fehlgeschlagen — Pose wird übersprungen.")
        return

    _train_pose_real()

    if enable_optimizations:
        _run_optimization_pipeline(
            model_name="Seg (Real)",
            source_weights=_best_weights("roboterarm_real"),
            data_yaml=REAL_YAML,
            calibration_dir=REAL_YOLO_DIR / "images" / "train",
            optimized_run_name="roboterarm_real_opt",
        )
        _run_optimization_pipeline(
            model_name="Pose (Real)",
            source_weights=_best_weights("roboterarm_pose_real"),
            data_yaml=REAL_POSE_YAML,
            calibration_dir=REAL_POSE_DATASET_DIR / "images" / "train",
            optimized_run_name="roboterarm_pose_real_opt",
        )

    print("\n" + "#" * 50)
    print("  ECHT KOMPLETT!")
    print(f"  Seg:  {MODEL_OUTPUT_DIR / 'roboterarm_real'}")
    print(f"  Pose: {MODEL_OUTPUT_DIR / 'roboterarm_pose_real'}")
    print("#" * 50)


def validate_model():
    """[3] Validiert das beste verfügbare Modell."""
    models = [
        ("Pose (Real, Opt)", _best_weights("roboterarm_pose_real_opt"), REAL_POSE_YAML),
        ("Pose (Real)", MODEL_OUTPUT_DIR / "roboterarm_pose_real" / "weights" / "best.pt", REAL_POSE_YAML),
        ("Pose (Blender, Opt)", _best_weights("roboterarm_pose_opt"), POSE_YAML),
        ("Pose (Blender)", MODEL_OUTPUT_DIR / "roboterarm_pose" / "weights" / "best.pt", POSE_YAML),
        ("Seg (Real, Opt)", _best_weights("roboterarm_real_opt"), REAL_YAML),
        ("Seg (Real)", MODEL_OUTPUT_DIR / "roboterarm_real" / "weights" / "best.pt", REAL_YAML),
        ("Seg (Blender, Opt)", _best_weights("roboterarm_seg_opt"), DATASET_YAML),
        ("Seg (Blender)", PRETRAINED_SEG, DATASET_YAML),
    ]

    for name, path, yaml in models:
        if path.exists() and yaml.exists():
            print(f"Validiere: {name} ({path})")
            model = YOLO(str(path))
            metrics = model.val(data=str(yaml))

            print(f"\n=== {name} — Validierungsergebnisse ===")
            if hasattr(metrics, 'seg') and metrics.seg is not None:
                print(f"mAP50 (Box):     {metrics.box.map50:.4f}")
                print(f"mAP50 (Mask):    {metrics.seg.map50:.4f}")
                print(f"mAP50-95 (Mask): {metrics.seg.map:.4f}")
            elif hasattr(metrics, 'pose') and metrics.pose is not None:
                print(f"mAP50 (Box):     {metrics.box.map50:.4f}")
                print(f"mAP50 (Pose):    {metrics.pose.map50:.4f}")
                print(f"mAP50-95 (Pose): {metrics.pose.map:.4f}")
            else:
                print(f"mAP50 (Box):     {metrics.box.map50:.4f}")
                print(f"mAP50-95 (Box):  {metrics.box.map:.4f}")
            return

    print("Kein trainiertes Modell gefunden. Bitte zuerst trainieren.")


def export_model():
    """[4] Exportiert das beste Modell nach ONNX."""
    candidates = [
        _best_weights("roboterarm_pose_real_opt"),
        MODEL_OUTPUT_DIR / "roboterarm_pose_real" / "weights" / "best.pt",
        _best_weights("roboterarm_pose_opt"),
        MODEL_OUTPUT_DIR / "roboterarm_pose" / "weights" / "best.pt",
        _best_weights("roboterarm_real_opt"),
        MODEL_OUTPUT_DIR / "roboterarm_real" / "weights" / "best.pt",
        _best_weights("roboterarm_seg_opt"),
        PRETRAINED_SEG,
    ]

    for path in candidates:
        if path.exists():
            print(f"Exportiere: {path}")
            model = YOLO(str(path))
            EXPORT_DIR.mkdir(parents=True, exist_ok=True)
            exported_path = Path(model.export(format="onnx", imgsz=IMG_SIZE, opset=20))
            target_path = EXPORT_DIR / exported_path.name

            # Ultralytics exportiert standardmaessig neben dem .pt Modell.
            # Datei wird zusaetzlich in den gewuenschten Edge-Export-Ordner kopiert.
            if exported_path.resolve() != target_path.resolve():
                shutil.copy2(exported_path, target_path)

            print(f"ONNX erzeugt: {exported_path}")
            print(f"ONNX exportiert nach: {target_path}")
            return

    print("Kein trainiertes Modell gefunden. Bitte zuerst trainieren.")


# ─────────────────────────────────────────────
# HAUPTPROGRAMM
# ─────────────────────────────────────────────

if __name__ == "__main__":
    steps = {
        "1": ("Virtuell trainieren (Seg + Pose)", train_virtuell),
        "2": ("Echt trainieren (Seg + Pose)", train_echt),
        "3": ("Validierung", validate_model),
        "4": ("Export (ONNX)", export_model),
    }

    cli_mode = _resolve_opt_flag_from_args(sys.argv[2:]) if len(sys.argv) > 1 else None

    if len(sys.argv) > 1 and sys.argv[1] in steps:
        choice = sys.argv[1]
    else:
        print("=" * 50)
        print("Roboterarm - KI-Training")
        print("=" * 50)
        for key, (desc, _) in steps.items():
            print(f"  [{key}] {desc}")
        print()
        choice = input("Schritt wählen (1-4): ").strip()

    if choice in steps:
        desc, func = steps[choice]
        print(f"\n>>> {desc} <<<\n")
        print(f"Device: {DEVICE}")

        if choice in {"1", "2"}:
            enable_optimizations = cli_mode if cli_mode is not None else _ask_optimization_mode()
            print(f"Optimierungsmodus: {'AN' if enable_optimizations else 'AUS'}")
            func(enable_optimizations=enable_optimizations)
        else:
            func()
    else:
        print("Ungültige Auswahl.")
