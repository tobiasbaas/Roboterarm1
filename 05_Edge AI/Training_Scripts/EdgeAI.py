import os
import sys
import shutil
import random
import subprocess
import json
import re
from datetime import datetime
from pathlib import Path
from typing import Optional
from ultralytics import YOLO


def _auto_select_device() -> str:
    """Bevorzugt NVIDIA-GPU (CUDA), ansonsten CPU."""
    forced_device = os.getenv("YOLO_DEVICE", "").strip()
    if forced_device:
        print(f"Device Override via YOLO_DEVICE: {forced_device}")
        return forced_device

    try:
        import torch

        if torch.cuda.is_available() and torch.cuda.device_count() > 0:
            gpu_name = torch.cuda.get_device_name(0)
            print(f"NVIDIA GPU erkannt: {gpu_name} -> nutze cuda:0")
            return "cuda:0"
    except Exception:
        # Fallback unten prueft optional nvidia-smi.
        pass

    try:
        result = subprocess.run(
            ["nvidia-smi", "--query-gpu=name", "--format=csv,noheader"],
            capture_output=True,
            text=True,
            check=False,
            timeout=3,
        )
        if result.returncode == 0 and result.stdout.strip():
            first_gpu = result.stdout.strip().splitlines()[0]
            print(f"NVIDIA GPU via nvidia-smi erkannt: {first_gpu} -> nutze cuda:0")
            return "cuda:0"
    except Exception:
        pass

    print("Keine NVIDIA-GPU erkannt -> nutze CPU")
    return "cpu"


# Intel XPU ist in ultralytics 8.4.x noch nicht vollständig unterstützt.
DEVICE = _auto_select_device()

# --- KONFIGURATION ---

SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent


def _path_for_yaml(target: Path, yaml_file: Path) -> str:
    """Nutze relative Pfade in YAML, damit Projekte portabel bleiben."""
    try:
        return target.resolve().relative_to(yaml_file.parent.resolve()).as_posix()
    except ValueError:
        return target.resolve().as_posix()


def _yaml_with_absolute_dataset_path(yaml_path: Path) -> Path:
    """Erzeugt eine temporäre YAML mit absolutem `path:` für Ultralytics."""
    yaml_path = yaml_path.resolve()
    if not yaml_path.exists():
        return yaml_path

    lines = yaml_path.read_text(encoding="utf-8").splitlines(keepends=True)
    rewritten = []
    replaced = False

    for line in lines:
        stripped = line.lstrip()
        if stripped.startswith("path:") and not replaced:
            indent = line[: len(line) - len(stripped)]
            raw = stripped.split(":", 1)[1].strip().strip('"').strip("'")
            candidate = Path(raw)
            if not candidate.is_absolute():
                candidate = (yaml_path.parent / candidate).resolve()
            else:
                candidate = candidate.resolve()
            rewritten.append(f"{indent}path: {candidate.as_posix()}\n")
            replaced = True
        else:
            rewritten.append(line)

    if not replaced:
        rewritten.insert(0, f"path: {yaml_path.parent.as_posix()}\n")

    tmp_dir = SCRIPT_DIR / ".ultralytics_tmp"
    tmp_dir.mkdir(parents=True, exist_ok=True)
    out_path = tmp_dir / f"{yaml_path.stem}.abs.yaml"
    out_path.write_text("".join(rewritten), encoding="utf-8")
    return out_path


def _normalize_yolo_label_file(src: Path, dst: Path) -> None:
    """Schreibt YOLO-Labels mit Dezimalpunkt statt Dezimalkomma."""
    text = src.read_text(encoding="utf-8", errors="ignore")
    normalized = text.replace(",", ".")
    dst.write_text(normalized, encoding="utf-8")

# Pfade
DATASET_DIR = SCRIPT_DIR / "Segmentierung per Blender" / "dataset"
YOLO_DATASET_DIR = SCRIPT_DIR / "Segmentierung per Blender" / "yolo_dataset"
DATASET_YAML = SCRIPT_DIR / "Segmentierung per Blender" / "dataset.yaml"
MODEL_OUTPUT_DIR = PROJECT_ROOT / "Trained_Models" / "AI-Models"
EXPORT_DIR = PROJECT_ROOT / "Edge Export"
RUNS_INDEX_FILE = MODEL_OUTPUT_DIR / "runs_index.json"
YOLO11N_SEG = PROJECT_ROOT / "yolo11n-seg.pt"
YOLO11N_POSE = PROJECT_ROOT / "yolo11n-pose.pt"

# Pose (Blender)
POSE_DATASET_DIR = SCRIPT_DIR / "Segmentierung per Blender" / "pose_dataset"
POSE_YAML = SCRIPT_DIR / "Segmentierung per Blender" / "pose_dataset.yaml"

# Echt (Seg Fine-Tuning)
PRETRAINED_SEG = MODEL_OUTPUT_DIR / "roboterarm_seg" / "weights" / "best.pt"
REAL_YOLO_DIR = SCRIPT_DIR / "real_yolo_dataset"
REAL_YAML = SCRIPT_DIR / "real_dataset.yaml"
REAL_DATASET_DIR = SCRIPT_DIR / "real_dataset"

# Echt (Pose)
PRETRAINED_POSE = MODEL_OUTPUT_DIR / "roboterarm_pose" / "weights" / "best.pt"
REAL_POSE_YAML = SCRIPT_DIR / "real_pose_dataset.yaml"
REAL_POSE_DATASET_DIR = SCRIPT_DIR / "real_pose_dataset"

# Allgemeine Parameter
IMG_SIZE = 640
BATCH_SIZE = 16
VAL_SPLIT = 0.3
SEED = 42
PRUNE_RATIO = 0.20
PRUNE_RECOVERY_EPOCHS = 20
CALIBRATION_IMAGES = 64

# ---------------------


# ═════════════════════════════════════════════
# HILFSFUNKTIONEN
# ═════════════════════════════════════════════

def _best_weights(run_name: str) -> Path:
    """Erzeugt den erwarteten Pfad zur best.pt-Datei eines Trainingsruns."""
    return MODEL_OUTPUT_DIR / run_name / "weights" / "best.pt"


def _load_runs_index() -> dict:
    """Lädt den Lauf-Index (neueste Runs + Historie) aus JSON."""
    if not RUNS_INDEX_FILE.exists():
        return {"latest": {}, "history": {}}

    try:
        data = json.loads(RUNS_INDEX_FILE.read_text(encoding="utf-8"))
    except Exception:
        return {"latest": {}, "history": {}}

    if not isinstance(data, dict):
        return {"latest": {}, "history": {}}

    data.setdefault("latest", {})
    data.setdefault("history", {})
    return data


def _save_runs_index(data: dict) -> None:
    """Speichert den Lauf-Index im Modellordner."""
    MODEL_OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    RUNS_INDEX_FILE.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8")


def _next_version_for_base(base_name: str) -> int:
    """Bestimmt die nächste fortlaufende Version je Basismodell."""
    pattern = re.compile(rf"^{re.escape(base_name)}_v(\d+)_\d{{8}}$")
    max_version = 0

    if MODEL_OUTPUT_DIR.exists():
        for p in MODEL_OUTPUT_DIR.iterdir():
            if not p.is_dir():
                continue
            m = pattern.match(p.name)
            if m:
                max_version = max(max_version, int(m.group(1)))

    data = _load_runs_index()
    for run_name in data.get("history", {}).get(base_name, []):
        m = pattern.match(run_name)
        if m:
            max_version = max(max_version, int(m.group(1)))

    return max_version + 1


def _create_versioned_run_name(base_name: str) -> str:
    """Erzeugt Run-Namen mit Version + Datum, z.B. roboterarm_pose_v003_20260322."""
    version = _next_version_for_base(base_name)
    date_str = datetime.now().strftime("%Y%m%d")
    return f"{base_name}_v{version:03d}_{date_str}"


def _register_run(base_name: str, run_name: str) -> None:
    """Schreibt den letzten und historischen Run in den Lauf-Index."""
    data = _load_runs_index()
    data.setdefault("latest", {})
    data.setdefault("history", {})
    data["latest"][base_name] = run_name
    data["history"].setdefault(base_name, [])
    if run_name not in data["history"][base_name]:
        data["history"][base_name].append(run_name)
    _save_runs_index(data)


def _latest_run_name(base_name: str) -> Optional[str]:
    """Liefert den zuletzt registrierten oder den neuesten vorhandenen versionierten Run."""
    data = _load_runs_index()
    latest = data.get("latest", {}).get(base_name)
    if latest and (MODEL_OUTPUT_DIR / latest).exists():
        return latest

    prefix = f"{base_name}_v"
    matches = [p for p in MODEL_OUTPUT_DIR.glob(f"{prefix}*") if p.is_dir()]
    if not matches:
        return None

    newest = max(matches, key=lambda p: p.stat().st_mtime)
    return newest.name


def _best_weights_for_base(base_name: str, fallback: Optional[Path] = None) -> Optional[Path]:
    """Liefert best.pt für das neueste versionierte Basismodell (mit Fallback)."""
    latest = _latest_run_name(base_name)
    if latest:
        weights = _best_weights(latest)
        if weights.exists():
            return weights

    if fallback is not None and fallback.exists():
        return fallback

    return None


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


def _resolve_export_quantize_flag_from_args(args: list[str]) -> Optional[bool]:
    """Liest optionale CLI-Flags für Quantisierung im Export-Schritt."""
    if "--quantize" in args:
        return True
    if "--no-quantize" in args:
        return False
    return None


def _ask_yes_no(question: str, default_no: bool = True) -> bool:
    """Einfache Ja/Nein-Abfrage für interaktive Optionen."""
    suffix = "[j/N]" if default_no else "[J/n]"
    raw = input(f"{question} {suffix}: ").strip().lower()
    if not raw:
        return not default_no
    return raw in {"j", "ja", "y", "yes"}


def _calibration_dir_for_weights(weights_path: Path) -> Path:
    """Wählt den Kalibrier-Ordner passend zum Modelltyp."""
    run_name = weights_path.parent.parent.name.lower()

    if "pose_real" in run_name:
        return REAL_POSE_DATASET_DIR / "images" / "train"
    if "real" in run_name:
        return REAL_YOLO_DIR / "images" / "train"
    if "pose" in run_name:
        return POSE_DATASET_DIR / "images" / "train"
    return YOLO_DATASET_DIR / "images" / "train"


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

    data_yaml_abs = _yaml_with_absolute_dataset_path(data_yaml)
    model.train(
        data=str(data_yaml_abs),
        device=DEVICE,
        amp=False,
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
        # STM32 AI Studio akzeptiert hier kein unsigned-quantized ONNX.
        activation_type=QuantType.QInt8,
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

    # Versionierten Optimierungs-Run im Index registrieren.
    if "_v" in optimized_run_name:
        base_name = optimized_run_name.split("_v", 1)[0]
        _register_run(base_name, optimized_run_name)

def _prepare_seg_dataset():
    """Erstellt YOLO-Ordnerstruktur und verteilt Bilder + Labels in Train/Val."""
    random.seed(SEED)

    image_source = DATASET_DIR / "images"
    if not image_source.exists():
        # Legacy fallback (alte Struktur mit image_*.png direkt in dataset/)
        image_source = DATASET_DIR

    label_sources = [
        DATASET_DIR / "labels",  # neue moegliche Struktur
        DATASET_DIR,              # legacy Struktur
    ]

    existing_prepared_train = list((YOLO_DATASET_DIR / "images" / "train").glob("*.png"))
    existing_prepared_val = list((YOLO_DATASET_DIR / "images" / "val").glob("*.png"))

    for split in ["train", "val"]:
        img_split = YOLO_DATASET_DIR / "images" / split
        lbl_split = YOLO_DATASET_DIR / "labels" / split
        img_split.mkdir(parents=True, exist_ok=True)
        lbl_split.mkdir(parents=True, exist_ok=True)

        # Vorherige Inhalte aufraeumen, damit keine stale Dateien bleiben.
        for p in img_split.glob("*"):
            if p.is_file():
                p.unlink()
        for p in lbl_split.glob("*"):
            if p.is_file():
                p.unlink()

    image_files = sorted(image_source.glob("image_*.png"))
    print(f"Gefunden: {len(image_files)} Bilder")

    if len(image_files) == 0:
        raise FileNotFoundError(f"Keine Bilder in {image_source} gefunden!")

    indices = list(range(len(image_files)))
    random.shuffle(indices)
    val_count = int(len(indices) * VAL_SPLIT)
    val_indices = set(indices[:val_count])

    train_count = 0
    val_count_actual = 0
    missing_labels = 0

    for i, img_path in enumerate(image_files):
        label_name = img_path.stem + ".txt"
        label_path = None
        for label_source in label_sources:
            candidate = label_source / label_name
            if candidate.exists():
                label_path = candidate
                break

        if label_path is None:
            missing_labels += 1
            continue

        split = "val" if i in val_indices else "train"
        shutil.copy2(img_path, YOLO_DATASET_DIR / "images" / split / img_path.name)
        shutil.copy2(label_path, YOLO_DATASET_DIR / "labels" / split / label_name)

        if split == "train":
            train_count += 1
        else:
            val_count_actual += 1

    if train_count + val_count_actual == 0:
        # Wenn keine Labels im Raw-Dataset gefunden wurden, aber ein fertiges yolo_dataset
        # vorhanden ist, weitertrainieren statt hart zu failen.
        if existing_prepared_train or existing_prepared_val:
            print("WARNUNG: Keine Seg-Labels im Raw-Dataset gefunden.")
            print("Verwende vorhandenes yolo_dataset (images/labels train/val).")
            print(f"Vorhanden: {len(existing_prepared_train)} Train, {len(existing_prepared_val)} Val")
            return
        raise FileNotFoundError(
            "Keine Seg-Labels gefunden. Erwartet z.B. dataset/labels oder dataset/*.txt"
        )

    if missing_labels:
        print(f"WARNUNG: {missing_labels} Bilder ohne Seg-Label wurden übersprungen.")
    print(f"Dataset aufgeteilt: {train_count} Train, {val_count_actual} Val")


def _train_seg_blender():
    """Trainiert YOLO-Seg auf Blender-Daten."""
    print("\n" + "=" * 50)
    print("SEGMENTIERUNG — Blender-Daten")
    print("=" * 50)

    print("Dataset wird vorbereitet...")
    _prepare_seg_dataset()

    print("\nTraining wird gestartet...")
    if not YOLO11N_SEG.exists():
        raise FileNotFoundError(f"Nicht gefunden: {YOLO11N_SEG}")

    run_name = _create_versioned_run_name("roboterarm_seg")

    model = YOLO(str(YOLO11N_SEG))
    data_yaml_abs = _yaml_with_absolute_dataset_path(DATASET_YAML)
    model.train(
        data=str(data_yaml_abs),
        device=DEVICE,
        amp=False,
        epochs=100,
        imgsz=IMG_SIZE,
        batch=BATCH_SIZE,
        project=str(MODEL_OUTPUT_DIR),
        name=run_name,
        exist_ok=True,
        seed=SEED,
        hsv_h=0.01, hsv_s=0.3, hsv_v=0.3,
        degrees=15.0, translate=0.1, scale=0.3,
        fliplr=0.5, flipud=0.0,
        mosaic=0.5,
        patience=20,
    )
    _register_run("roboterarm_seg", run_name)
    print(f"Seg-Modell fertig: {MODEL_OUTPUT_DIR / run_name}")
    return run_name


def _train_pose_blender():
    """Trainiert YOLO-Pose auf Blender-Daten."""
    print("\n" + "=" * 50)
    print("POSE — Blender-Daten")
    print("=" * 50)

    if not POSE_YAML.exists():
        print(f"FEHLER: {POSE_YAML} nicht gefunden!")
        print("Bitte zuerst pose_dataset.yaml und pose_dataset (train/val) erzeugen.")
        return None

    train_imgs = list((POSE_DATASET_DIR / "images" / "train").glob("*.png"))
    val_imgs = list((POSE_DATASET_DIR / "images" / "val").glob("*.png"))
    if not train_imgs:
        print("FEHLER: Keine Trainingsbilder in pose_dataset/!")
        print("Bitte zuerst Blender_Daten in pose_dataset/images/train|val aufteilen.")
        return None

    print(f"Dataset: {len(train_imgs)} Train, {len(val_imgs)} Val")

    if not YOLO11N_POSE.exists():
        raise FileNotFoundError(f"Nicht gefunden: {YOLO11N_POSE}")

    run_name = _create_versioned_run_name("roboterarm_pose")

    model = YOLO(str(YOLO11N_POSE))
    data_yaml_abs = _yaml_with_absolute_dataset_path(POSE_YAML)
    model.train(
        data=str(data_yaml_abs),
        device=DEVICE,
        amp=False,
        epochs=100,
        imgsz=IMG_SIZE,
        batch=BATCH_SIZE,
        project=str(MODEL_OUTPUT_DIR),
        name=run_name,
        exist_ok=True,
        seed=SEED,
        lr0=0.01, lrf=0.01,
        # Conservative pose augmentations for more stable keypoint learning.
        degrees=8.0, translate=0.05, scale=0.2,
        fliplr=0.0,
        mosaic=0.0,
        patience=20,
    )
    _register_run("roboterarm_pose", run_name)
    print(f"Pose-Modell fertig: {MODEL_OUTPUT_DIR / run_name}")
    return run_name


def _finetune_seg_real():
    """Fine-Tuning: Blender-Seg-Modell auf echten Daten."""
    print("\n" + "=" * 50)
    print("SEGMENTIERUNG — Echte Bilder (Fine-Tuning)")
    print("=" * 50)

    pretrained_seg = _best_weights_for_base("roboterarm_seg", PRETRAINED_SEG)
    if pretrained_seg is None:
        print(f"FEHLER: Kein vortrainiertes Seg-Modell gefunden (Basis: roboterarm_seg)")
        print("Bitte zuerst [1] Virtuell trainieren.")
        return False

    yaml_content = f"""path: {_path_for_yaml(REAL_YOLO_DIR, REAL_YAML)}
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
        print("Bitte zuerst das reale Seg-Dataset unter real_dataset/images|labels/all vorbereiten.")
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

        # Alte Dateien entfernen, damit kein Mix aus alten/defekten Labels entsteht.
        for p in img_out.glob("*"):
            if p.is_file():
                p.unlink()
        for p in lbl_out.glob("*"):
            if p.is_file():
                p.unlink()

        for img_path in images:
            shutil.copy2(img_path, img_out / img_path.name)
            src_label = src_labels / (img_path.stem + ".txt")
            dst_label = lbl_out / (img_path.stem + ".txt")
            _normalize_yolo_label_file(src_label, dst_label)

    print(f"Train: {len(train_images)}, Val: {len(val_images)}")

    run_name = _create_versioned_run_name("roboterarm_real")

    model = YOLO(str(pretrained_seg))
    data_yaml_abs = _yaml_with_absolute_dataset_path(REAL_YAML)
    model.train(
        data=str(data_yaml_abs),
        device=DEVICE,
        amp=False,
        epochs=100,
        imgsz=IMG_SIZE,
        batch=BATCH_SIZE,
        project=str(MODEL_OUTPUT_DIR),
        name=run_name,
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
    _register_run("roboterarm_real", run_name)
    print(f"Seg-Modell (Real) fertig: {MODEL_OUTPUT_DIR / run_name}")
    return True


def _train_pose_real():
    """Trainiert YOLO-Pose auf echten Bildern."""
    print("\n" + "=" * 50)
    print("POSE — Echte Bilder")
    print("=" * 50)

    if not REAL_POSE_YAML.exists():
        print(f"FEHLER: {REAL_POSE_YAML} nicht gefunden!")
        print("Bitte zuerst real_pose_dataset.yaml und real_pose_dataset erzeugen.")
        return

    train_imgs = list((REAL_POSE_DATASET_DIR / "images" / "train").glob("*.png"))
    val_imgs = list((REAL_POSE_DATASET_DIR / "images" / "val").glob("*.png"))
    if not train_imgs:
        print("FEHLER: Keine Trainingsbilder in real_pose_dataset/!")
        print("Bitte zuerst echte Pose-Daten nach real_pose_dataset/images/train|val legen.")
        return

    print(f"Dataset: {len(train_imgs)} Train, {len(val_imgs)} Val")

    pretrained_pose = _best_weights_for_base("roboterarm_pose", PRETRAINED_POSE)
    run_name = _create_versioned_run_name("roboterarm_pose_real")

    if pretrained_pose is not None:
        base_model = str(pretrained_pose)
        print(f"Starte von Blender-Pose-Modell: {base_model}")
        freeze = 10
        lr0 = 0.001
    else:
        if not YOLO11N_POSE.exists():
            print(f"FEHLER: Basis-Posemodell fehlt: {YOLO11N_POSE}")
            return

        base_model = str(YOLO11N_POSE)
        print(f"Kein Blender-Pose-Modell gefunden, starte von: {base_model}")
        freeze = 0
        lr0 = 0.01

    model = YOLO(base_model)
    data_yaml_abs = _yaml_with_absolute_dataset_path(REAL_POSE_YAML)
    model.train(
        data=str(data_yaml_abs),
        device=DEVICE,
        amp=False,
        epochs=100,
        imgsz=IMG_SIZE,
        batch=BATCH_SIZE,
        project=str(MODEL_OUTPUT_DIR),
        name=run_name,
        exist_ok=True,
        seed=SEED,
        lr0=lr0, lrf=0.01,
        freeze=freeze,
        # Conservative pose augmentations for more stable keypoint learning.
        degrees=6.0, translate=0.05, scale=0.2,
        fliplr=0.0,
        mosaic=0.0,
        patience=15,
    )
    _register_run("roboterarm_pose_real", run_name)
    print(f"Pose-Modell (Real) fertig: {MODEL_OUTPUT_DIR / run_name}")


# ═════════════════════════════════════════════
# HAUPTFUNKTIONEN (Menü-Einträge)
# ═════════════════════════════════════════════

def train_virtuell(enable_optimizations: bool = False):
    """[1] Virtuell: Seg + Pose auf Blender-Daten nacheinander trainieren."""
    print("\n" + "#" * 50)
    print("  VIRTUELL TRAINIEREN (Seg + Pose)")
    print("#" * 50)

    seg_run_name = _train_seg_blender()
    pose_run_name = _train_pose_blender()

    if enable_optimizations:
        if seg_run_name:
            _run_optimization_pipeline(
                model_name="Seg (Blender)",
                source_weights=_best_weights(seg_run_name),
                data_yaml=DATASET_YAML,
                calibration_dir=YOLO_DATASET_DIR / "images" / "train",
                optimized_run_name=_create_versioned_run_name("roboterarm_seg_opt"),
            )

        if pose_run_name:
            _run_optimization_pipeline(
                model_name="Pose (Blender)",
                source_weights=_best_weights(pose_run_name),
                data_yaml=POSE_YAML,
                calibration_dir=POSE_DATASET_DIR / "images" / "train",
                optimized_run_name=_create_versioned_run_name("roboterarm_pose_opt"),
            )

    print("\n" + "#" * 50)
    print("  VIRTUELL KOMPLETT!")
    if seg_run_name:
        print(f"  Seg:  {MODEL_OUTPUT_DIR / seg_run_name}")
    if pose_run_name:
        print(f"  Pose: {MODEL_OUTPUT_DIR / pose_run_name}")
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
        real_seg = _best_weights_for_base("roboterarm_real")
        real_pose = _best_weights_for_base("roboterarm_pose_real")

        if real_seg is not None:
            _run_optimization_pipeline(
                model_name="Seg (Real)",
                source_weights=real_seg,
                data_yaml=REAL_YAML,
                calibration_dir=REAL_YOLO_DIR / "images" / "train",
                optimized_run_name=_create_versioned_run_name("roboterarm_real_opt"),
            )

        if real_pose is not None:
            _run_optimization_pipeline(
                model_name="Pose (Real)",
                source_weights=real_pose,
                data_yaml=REAL_POSE_YAML,
                calibration_dir=REAL_POSE_DATASET_DIR / "images" / "train",
                optimized_run_name=_create_versioned_run_name("roboterarm_pose_real_opt"),
            )

    print("\n" + "#" * 50)
    print("  ECHT KOMPLETT!")
    latest_real_seg = _best_weights_for_base("roboterarm_real")
    latest_real_pose = _best_weights_for_base("roboterarm_pose_real")
    if latest_real_seg:
        print(f"  Seg:  {latest_real_seg.parent.parent}")
    if latest_real_pose:
        print(f"  Pose: {latest_real_pose.parent.parent}")
    print("#" * 50)


def validate_model():
    """[3] Validiert das beste verfügbare Modell."""
    models = [
        ("Pose (Real, Opt)", _best_weights_for_base("roboterarm_pose_real_opt"), REAL_POSE_YAML),
        ("Pose (Real)", _best_weights_for_base("roboterarm_pose_real"), REAL_POSE_YAML),
        ("Pose (Blender, Opt)", _best_weights_for_base("roboterarm_pose_opt"), POSE_YAML),
        ("Pose (Blender)", _best_weights_for_base("roboterarm_pose"), POSE_YAML),
        ("Seg (Real, Opt)", _best_weights_for_base("roboterarm_real_opt"), REAL_YAML),
        ("Seg (Real)", _best_weights_for_base("roboterarm_real"), REAL_YAML),
        ("Seg (Blender, Opt)", _best_weights_for_base("roboterarm_seg_opt"), DATASET_YAML),
        ("Seg (Blender)", _best_weights_for_base("roboterarm_seg", PRETRAINED_SEG), DATASET_YAML),
    ]

    for name, path, yaml in models:
        if path is not None and path.exists() and yaml.exists():
            print(f"Validiere: {name} ({path})")
            model = YOLO(str(path))
            yaml_abs = _yaml_with_absolute_dataset_path(yaml)
            metrics = model.val(data=str(yaml_abs))

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


def export_model(quantize_override: Optional[bool] = None):
    """[4] Exportiert nur Real-Modelle (Seg + Pose), optional mit INT8-Quantisierung."""
    optimized_candidates = [
        ("Seg (Real, Opt)", _best_weights_for_base("roboterarm_real_opt")),
        ("Pose (Real, Opt)", _best_weights_for_base("roboterarm_pose_real_opt")),
    ]

    fallback_candidates = [
        ("Pose (Real)", _best_weights_for_base("roboterarm_pose_real")),
        ("Seg (Real)", _best_weights_for_base("roboterarm_real")),
    ]

    export_jobs = [(name, path) for name, path in optimized_candidates if path is not None and path.exists()]

    if not export_jobs:
        # Rueckfall: wie bisher wenigstens ein bestes nicht-optimiertes Modell exportieren.
        for name, path in fallback_candidates:
            if path is not None and path.exists():
                export_jobs = [(name, path)]
                break

    if not export_jobs:
        print("Kein trainiertes Modell gefunden. Bitte zuerst trainieren.")
        return

    do_quantize = quantize_override
    if do_quantize is None:
        do_quantize = _ask_yes_no("Zusätzlich INT8 quantisieren?")

    EXPORT_DIR.mkdir(parents=True, exist_ok=True)

    for name, path in export_jobs:
        print(f"Exportiere: {name} ({path})")
        model = YOLO(str(path))
        exported_path = Path(model.export(format="onnx", imgsz=IMG_SIZE, opset=20))
        target_path = EXPORT_DIR / exported_path.name

        # Ultralytics exportiert standardmaessig neben dem .pt Modell.
        # Datei wird zusaetzlich in den gewuenschten Edge-Export-Ordner kopiert.
        if exported_path.resolve() != target_path.resolve():
            shutil.copy2(exported_path, target_path)

        print(f"ONNX erzeugt: {exported_path}")
        print(f"ONNX exportiert nach: {target_path}")

        if do_quantize:
            calibration_dir = _calibration_dir_for_weights(path)
            run_name = path.parent.parent.name
            _export_int8_onnx(
                source_weights=path,
                calibration_dir=calibration_dir,
                export_name=run_name,
            )


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
    export_quant_mode = _resolve_export_quantize_flag_from_args(sys.argv[2:]) if len(sys.argv) > 1 else None

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
        elif choice == "4":
            func(quantize_override=export_quant_mode)
        else:
            func()
    else:
        print("Ungültige Auswahl.")
