"""
Roboterarm - Datenvorbereitung
================================
Daten vorbereiten in zwei Modi:

  [1] Virtuell — Daten vorbereiten  (Masken→Labels + Pose-Labels)
  [2] Echt — Daten vorbereiten      (Auto-Annotation + Seg→Pose-Labels)
  [3] Winkelberechnung              (Seg oder Pose → Gelenkwinkel)

Training → siehe EdgeAI.py

Aufruf:
  python Umrisse_in_Polygone.py          → Menüauswahl
  python Umrisse_in_Polygone.py 1        → Direkt Schritt 1
"""

import cv2
import numpy as np
import os
import sys
import glob
import shutil
import random
from pathlib import Path

# === KONFIGURATION ===

# Schritt 1: Blender-Masken → Labels
BLENDER_DATASET_DIR = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\Segmentierung per Blender\dataset")

# Schritt 2: CSV → Pose-Labels
BLENDER_CSV = BLENDER_DATASET_DIR / "joint_data.csv"
POSE_DATASET_DIR = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\Segmentierung per Blender\pose_dataset")
POSE_YAML = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\Segmentierung per Blender\pose_dataset.yaml")
KEYPOINT_PADDING = 30

# Schritt 3: Auto-Annotation
PRETRAINED_MODEL = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Trained_Models\AI-Models\roboterarm_seg\weights\best.pt")
REAL_IMAGES_DIR = Path(r"C:\Dev\Image_Capture_Python\dataset\images\angepasstes_seitenv_20260311_170850")
REAL_DATASET_DIR = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\real_dataset")
CONF_THRESHOLD = 0.3

# Schritt 4: Winkelberechnung
MODEL_OUTPUT_DIR = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Trained_Models\AI-Models")
GROUND_TRUTH_CSV = Path(r"C:\Dev\Image_Capture_Python\dataset\labels_angepasstes_seitenv_20260311_170850.csv")
WINKEL_OUTPUT_DIR = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\winkel_output")

# Schritt 5: Seg → Pose-Labels (echte Bilder)
REAL_POSE_DATASET_DIR = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\real_pose_dataset")
REAL_POSE_YAML = Path(r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\real_pose_dataset.yaml")

# Allgemein
IMG_SIZE = 640
VAL_SPLIT = 0.3
SEED = 42
DEFAULT_POSE_KEYPOINT_COUNT = 6
SEGMENT_KEYPOINT_IDS = [0, 1, 2, 3, 4]
POSE_CONNECTIONS = [(0, 1), (1, 2), (2, 3), (3, 4), (4, 5)]

# Klassen
CLASS_NAMES = {0: "Base", 1: "Joint_1", 2: "Joint_2", 3: "Joint_3", 4: "Joint_4_Finger"}

# Farbbereiche im HSV-Farbraum für Blender-Masken
COLOR_RANGES = {
    0:     [np.array([0, 50, 50]),   np.array([10, 255, 255])],      # Rot (Base) - Teil 1
    '0_b': [np.array([170, 50, 50]), np.array([179, 255, 255])],     # Rot (Base) - Teil 2
    1:     [np.array([40, 50, 50]),  np.array([85, 255, 255])],      # Grün (Joint 1)
    2:     [np.array([90, 50, 50]),  np.array([135, 255, 255])],     # Blau (Joint 2)
    3:     [np.array([20, 50, 50]),  np.array([35, 255, 255])],      # Gelb (Joint 3)
    4:     [np.array([140, 50, 50]), np.array([165, 255, 255])],     # Magenta (Joint 4 / Finger)
}

# =====================


# ═════════════════════════════════════════════
# HILFSFUNKTIONEN
# ═════════════════════════════════════════════

def _infer_pose_keypoint_count(label_files, fallback=DEFAULT_POSE_KEYPOINT_COUNT):
    """Liest die Keypoint-Anzahl aus YOLO-Pose-Labels aus."""
    detected_counts = set()

    for label_path in label_files:
        try:
            with open(label_path, "r", encoding="utf-8") as f:
                for line in f:
                    stripped = line.strip()
                    if not stripped:
                        continue
                    values = stripped.split()
                    if len(values) < 5:
                        continue
                    remainder = len(values) - 5
                    if remainder % 3 != 0:
                        raise ValueError(
                            f"Ungültiges Pose-Label in {label_path}: {len(values)} Werte gefunden"
                        )
                    detected_counts.add(remainder // 3)
                    break
        except OSError:
            continue

    if not detected_counts:
        return fallback

    if len(detected_counts) > 1:
        counts_text = ", ".join(str(count) for count in sorted(detected_counts))
        raise ValueError(f"Inkonsistente Keypoint-Anzahlen gefunden: {counts_text}")

    return detected_counts.pop()


def _create_pose_yaml(dataset_dir, keypoint_count):
    flip_idx = ", ".join(str(idx) for idx in range(keypoint_count))
    return f"""path: {dataset_dir}
train: images/train
val: images/val

kpt_shape: [{keypoint_count}, 3]
flip_idx: [{flip_idx}]

names:
  0: robot_arm
"""


def _joint_angle(reference_vector, target_vector):
    return np.degrees(np.arctan2(
        reference_vector[0] * target_vector[1] - reference_vector[1] * target_vector[0],
        np.dot(reference_vector, target_vector)
    ))


def _vector_between(joint_points, start_idx, end_idx):
    if start_idx not in joint_points or end_idx not in joint_points:
        return None
    return joint_points[end_idx] - joint_points[start_idx]


def _predict_joint_angles(joint_points):
    """Berechnet bis zu vier Gelenkwinkel aus der Keypoint-Kette."""
    predicted = {}
    ref_up = np.array([0.0, -1.0])

    base_vector = _vector_between(joint_points, 0, 1)
    if base_vector is not None:
        predicted[0] = _joint_angle(ref_up, base_vector)

    joint_2_vector = _vector_between(joint_points, 1, 2)
    if base_vector is not None and joint_2_vector is not None:
        predicted[1] = _joint_angle(base_vector, joint_2_vector)

    joint_3_vector = _vector_between(joint_points, 2, 3)
    if joint_2_vector is not None and joint_3_vector is not None:
        predicted[2] = _joint_angle(joint_2_vector, joint_3_vector)

    joint_4_vector = _vector_between(joint_points, 3, 4)
    tcp_vector = _vector_between(joint_points, 4, 5)
    if joint_4_vector is not None and tcp_vector is not None:
        predicted[3] = _joint_angle(joint_4_vector, tcp_vector)
    elif joint_3_vector is not None and joint_4_vector is not None:
        predicted[3] = _joint_angle(joint_3_vector, joint_4_vector)

    return predicted

def blender_masken_zu_labels():
    """Konvertiert farbige Blender-Masken in YOLO-Segmentierungs-Labels."""
    dataset_dir = str(BLENDER_DATASET_DIR)
    mask_files = glob.glob(os.path.join(dataset_dir, "mask_*.png"))

    print(f"Gefunden: {len(mask_files)} Masken-Dateien")

    for mask_path in mask_files:
        img_bgr = cv2.imread(mask_path)
        if img_bgr is None:
            continue

        img_hsv = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2HSV)
        height, width = img_hsv.shape[:2]

        base_name = os.path.basename(mask_path).replace("mask_", "image_").replace(".png", ".txt")
        txt_path = os.path.join(dataset_dir, base_name)

        with open(txt_path, "w") as f:
            for class_id in [0, 1, 2, 3, 4]:
                if class_id == 0:
                    mask1 = cv2.inRange(img_hsv, COLOR_RANGES[0][0], COLOR_RANGES[0][1])
                    mask2 = cv2.inRange(img_hsv, COLOR_RANGES['0_b'][0], COLOR_RANGES['0_b'][1])
                    binary_mask = cv2.bitwise_or(mask1, mask2)
                else:
                    binary_mask = cv2.inRange(img_hsv, COLOR_RANGES[class_id][0], COLOR_RANGES[class_id][1])

                contours, _ = cv2.findContours(binary_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

                for contour in contours:
                    if cv2.contourArea(contour) < 50:
                        continue

                    polygon = []
                    for point in contour:
                        x = point[0][0] / width
                        y = point[0][1] / height
                        polygon.append(f"{x:.6f} {y:.6f}")

                    if polygon:
                        f.write(f"{class_id} " + " ".join(polygon) + "\n")

    print(f"Fertig! {len(mask_files)} Label-Dateien generiert.")




def csv_zu_pose_labels():
    """Erstellt YOLO-Pose Dataset aus vorgenerierten Labels in labels_pose/ + Train/Val Split.

    Die Pose-Labels werden direkt von Blender generiert
    und liegen bereits als YOLO-Format in dataset/labels_pose/.
    Die joint_data.csv enthält nur Winkel, keine Pixel-Koordinaten.
    """

    labels_dir = BLENDER_DATASET_DIR / "labels_pose"
    if not labels_dir.exists():
        print(f"FEHLER: {labels_dir} nicht gefunden!")
        return

    label_files = sorted(labels_dir.glob("*.txt"))
    print(f"Gefunden: {len(label_files)} Pose-Labels in labels_pose/")

    if not label_files:
        print("FEHLER: Keine Label-Dateien gefunden!")
        return

    keypoint_count = _infer_pose_keypoint_count(label_files)
    print(f"Keypoints pro Label: {keypoint_count}")

    # Nur Bilder mit vorhandenem Label verwenden
    valid_pairs = []
    for lbl_path in label_files:
        img_path = BLENDER_DATASET_DIR / (lbl_path.stem + ".png")
        if img_path.exists():
            valid_pairs.append((img_path, lbl_path))

    print(f"Bilder mit Labels: {len(valid_pairs)}")

    # Ausgabe-Ordner erstellen
    for split in ["train", "val"]:
        (POSE_DATASET_DIR / "images" / split).mkdir(parents=True, exist_ok=True)
        (POSE_DATASET_DIR / "labels" / split).mkdir(parents=True, exist_ok=True)

    # Shuffle + Train/Val Split
    indices = list(range(len(valid_pairs)))
    random.seed(SEED)
    random.shuffle(indices)
    val_count = int(len(indices) * VAL_SPLIT)
    splits = {
        "val": indices[:val_count],
        "train": indices[val_count:],
    }

    for split, idxs in splits.items():
        for idx in idxs:
            img_path, lbl_path = valid_pairs[idx]
            shutil.copy2(img_path, POSE_DATASET_DIR / "images" / split / img_path.name)
            shutil.copy2(lbl_path, POSE_DATASET_DIR / "labels" / split / lbl_path.name)

    print(f"Train: {len(splits['train'])}, Val: {len(splits['val'])}")

    yaml_content = _create_pose_yaml(POSE_DATASET_DIR, keypoint_count)
    with open(POSE_YAML, "w") as f:
        f.write(yaml_content)

    print(f"Dataset YAML: {POSE_YAML}")
    print(f"\n→ Training starten mit: python EdgeAI.py 1")




def auto_annotate():
    """Nutzt das vortrainierte Modell um echte Bilder automatisch zu labeln."""
    from ultralytics import YOLO

    model = YOLO(str(PRETRAINED_MODEL))

    img_dir = REAL_DATASET_DIR / "images" / "all"
    lbl_dir = REAL_DATASET_DIR / "labels" / "all"
    img_dir.mkdir(parents=True, exist_ok=True)
    lbl_dir.mkdir(parents=True, exist_ok=True)

    image_files = sorted(REAL_IMAGES_DIR.glob("*.png")) + sorted(REAL_IMAGES_DIR.glob("*.jpg"))
    print(f"Gefunden: {len(image_files)} echte Bilder")

    annotated_count = 0
    empty_count = 0

    for img_path in image_files:
        results = model(str(img_path), conf=CONF_THRESHOLD, verbose=False)
        r = results[0]

        label_lines = []
        if r.masks is not None and len(r.masks) > 0:
            h, w = r.orig_shape
            for mask, cls in zip(r.masks.xy, r.boxes.cls):
                class_id = int(cls)
                polygon = mask / np.array([w, h])
                coords = " ".join(f"{x:.6f} {y:.6f}" for x, y in polygon)
                label_lines.append(f"{class_id} {coords}")

        shutil.copy2(img_path, img_dir / img_path.name)

        label_name = img_path.stem + ".txt"
        with open(lbl_dir / label_name, "w") as f:
            f.write("\n".join(label_lines))

        if label_lines:
            annotated_count += 1
        else:
            empty_count += 1

    print(f"\nErgebnis:")
    print(f"  {annotated_count} Bilder mit Detektionen")
    print(f"  {empty_count} Bilder ohne Detektionen (leere Labels)")
    print(f"\nGespeichert in: {REAL_DATASET_DIR}")
    print(f"\n→ Nächster Schritt: Labels in Label Studio korrigieren, dann EdgeAI.py [2]")




def winkel_berechnen():
    """Berechnet Gelenkwinkel aus den Segmentierungsmasken oder Keypoints.

    Erkennt automatisch ob ein Pose- oder Seg-Modell vorhanden ist:
      - Pose → Keypoints direkt auslesen (genauer)
      - Seg  → Schwerpunkte aus Masken berechnen
    """
    from ultralytics import YOLO

    # Modell-Priorität: Pose (Real) > Pose (Blender) > Fine-Tuned-Seg > Blender-Seg
    pose_real_path = MODEL_OUTPUT_DIR / "roboterarm_pose_real" / "weights" / "best.pt"
    pose_blender_path = MODEL_OUTPUT_DIR / "roboterarm_pose" / "weights" / "best.pt"
    finetuned = MODEL_OUTPUT_DIR / "roboterarm_real" / "weights" / "best.pt"

    if pose_real_path.exists():
        model_path = pose_real_path
        use_pose = True
        print("Modus: YOLO-Pose Real (Keypoint-basierte Winkel)")
    elif pose_blender_path.exists():
        model_path = pose_blender_path
        use_pose = True
        print("Modus: YOLO-Pose Blender (Keypoint-basierte Winkel)")
    elif finetuned.exists():
        model_path = finetuned
        use_pose = False
        print("Modus: Segmentierung (Schwerpunkt-basierte Winkel)")
    else:
        model_path = PRETRAINED_MODEL
        use_pose = False
        print("Modus: Segmentierung (Schwerpunkt-basierte Winkel)")

    print(f"Modell: {model_path}")
    model = YOLO(str(model_path))

    # Bilder laden
    images = sorted(REAL_IMAGES_DIR.glob("*.png")) + sorted(REAL_IMAGES_DIR.glob("*.jpg"))
    if not images:
        print("Keine Bilder gefunden!")
        return

    # Ground Truth CSV laden (optional)
    gt_data = {}
    if GROUND_TRUTH_CSV.exists():
        import pandas as pd
        df = pd.read_csv(GROUND_TRUTH_CSV)
        for _, row in df.iterrows():
            fname = Path(row["image_file"]).name
            gt_data[fname] = [
                row["joint1_deg"], row["joint2_deg"],
                row["joint3_deg"], row["joint4_deg"],
            ]
        print(f"Ground Truth geladen: {len(gt_data)} Einträge")

    WINKEL_OUTPUT_DIR.mkdir(exist_ok=True)
    all_results = []

    for img_path in images:
        result = model(str(img_path), verbose=False)[0]

        joint_points = {}  # {joint_num: (x, y)}

        if use_pose:
            # --- POSE-MODELL: Keypoints direkt auslesen ---
            if result.keypoints is None or len(result.keypoints) == 0:
                continue
            kps = result.keypoints[0].data[0]  # [N_keypoints, 3] (x, y, conf)
            for j in range(len(kps)):
                x, y, kp_conf = float(kps[j][0]), float(kps[j][1]), float(kps[j][2])
                if kp_conf > 0.3:
                    joint_points[j] = np.array([x, y])
        else:
            # --- SEG-MODELL: Schwerpunkte aus Masken ---
            # 5 Klassen: 0=Base, 1=Joint_1, 2=Joint_2, 3=Joint_3, 4=Joint_4
            if result.masks is None:
                continue
            segments = {}
            for mask_xy, cls, conf in zip(result.masks.xy, result.boxes.cls, result.boxes.conf):
                cid = int(cls)
                if cid not in segments or float(conf) > segments[cid][1]:
                    segments[cid] = (mask_xy, float(conf))
            for cid in SEGMENT_KEYPOINT_IDS:
                if cid in segments:
                    pts = segments[cid][0]
                    joint_points[cid] = np.array([np.mean(pts[:, 0]), np.mean(pts[:, 1])])

        if len(joint_points) < 2:
            continue

        chain = sorted(joint_points.keys())
        predicted = _predict_joint_angles(joint_points)

        # --- Visualisierung ---
        img = cv2.imread(str(img_path))
        colors = {
            0: (0, 0, 255),
            1: (0, 255, 0),
            2: (255, 0, 0),
            3: (0, 255, 255),
            4: (255, 0, 255),
            5: (255, 255, 0),
        }

        for start_idx, end_idx in POSE_CONNECTIONS:
            if start_idx in joint_points and end_idx in joint_points:
                p1 = tuple(int(x) for x in joint_points[start_idx])
                p2 = tuple(int(x) for x in joint_points[end_idx])
                cv2.line(img, p1, p2, (255, 255, 255), 2)

        for j in chain:
            cx, cy = joint_points[j]
            color = colors.get(j, (255, 255, 255))
            cv2.circle(img, (int(cx), int(cy)), 8, color, -1)
            label = f"J{j}"
            if j in predicted:
                label += f" {predicted[j]:.1f}"
            cv2.putText(img, label, (int(cx) + 12, int(cy) - 12),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)

        # GT-Vergleich: predicted[0..3] ↔ joint1_deg..joint4_deg
        gt = gt_data.get(img_path.name)
        if gt:
            y = 25
            cv2.putText(img, "Joint | GT | Pred", (10, y),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.45, (255, 255, 255), 1)
            for j in range(4):
                y += 22
                pred_str = f"{predicted[j]:.1f}" if j in predicted else "---"
                text = f"J{j + 1}: {gt[j]:+.1f} | {pred_str}"
                cv2.putText(img, text, (10, y),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 255, 255), 1)

        cv2.imwrite(str(WINKEL_OUTPUT_DIR / img_path.name), img)
        all_results.append({"file": img_path.name, "predicted": predicted, "gt": gt})

    # --- Zusammenfassung ---
    print(f"\n{len(all_results)} Bilder verarbeitet.")
    print(f"Visualisierungen: {WINKEL_OUTPUT_DIR}")

    if all_results and all_results[0]["gt"]:
        print(f"\nBeispiel: {all_results[0]['file']}")
        print(f"  {'Joint':<12} {'GT':>8} {'Pred':>8} {'Diff':>8}")
        print(f"  {'-' * 38}")
        gt = all_results[0]["gt"]
        for j in range(4):
            gt_val = gt[j]
            pred_val = all_results[0]["predicted"].get(j)
            if pred_val is not None:
                diff = pred_val - gt_val
                print(f"  Joint_{j + 1:<7} {gt_val:+8.1f} {pred_val:+8.1f} {diff:+8.1f}")
            else:
                print(f"  Joint_{j + 1:<7} {gt_val:+8.1f}      ---      ---")

        errors = {j: [] for j in range(4)}
        for r in all_results:
            if r["gt"] is None:
                continue
            for j in range(4):
                if j in r["predicted"]:
                    errors[j].append(abs(r["predicted"][j] - r["gt"][j]))
        print(f"\n  Mittlerer absoluter Fehler (MAE) über {len(all_results)} Bilder:")
        for j in range(4):
            if errors[j]:
                mae = np.mean(errors[j])
                print(f"  Joint_{j + 1}: {mae:.1f}°")




def seg_zu_pose_labels_real():
    """Generiert YOLO-Pose-Labels aus Seg-Modell-Inferenz auf echten Bildern.

    Nutzt die Schwerpunkte der segmentierten Bereiche als Keypoints.
    Das funktionierende Seg-Modell dient als 'Lehrer' für die Pose-Label-Erzeugung.

    Voraussetzung: Trainiertes Seg-Modell (Blender oder Fine-Tuned).
    """
    from ultralytics import YOLO
    import pandas as pd

    # Seg-Modell laden (Priorität: Fine-Tuned > Blender)
    finetuned = MODEL_OUTPUT_DIR / "roboterarm_real" / "weights" / "best.pt"
    model_path = finetuned if finetuned.exists() else PRETRAINED_MODEL

    if not model_path.exists():
        print(f"FEHLER: Kein Seg-Modell unter {model_path}")
        print("Bitte zuerst EdgeAI.py [1] Virtuell trainieren ausführen.")
        return

    print(f"Seg-Modell: {model_path}")
    model = YOLO(str(model_path))

    # Bilder laden
    images = sorted(REAL_IMAGES_DIR.glob("*.png")) + sorted(REAL_IMAGES_DIR.glob("*.jpg"))
    if not images:
        print(f"Keine Bilder in {REAL_IMAGES_DIR}")
        return
    print(f"Gefunden: {len(images)} Bilder")

    # Ground Truth CSV laden (für Protokoll)
    gt_data = {}
    if GROUND_TRUTH_CSV.exists():
        df = pd.read_csv(GROUND_TRUTH_CSV)
        for _, row in df.iterrows():
            fname = Path(row["image_file"]).name
            gt_data[fname] = [
                row["joint1_deg"], row["joint2_deg"],
                row["joint3_deg"], row["joint4_deg"],
            ]
        print(f"Ground Truth geladen: {len(gt_data)} Einträge")

    # Ausgabe-Ordner
    for split in ["train", "val"]:
        (REAL_POSE_DATASET_DIR / "images" / split).mkdir(parents=True, exist_ok=True)
        (REAL_POSE_DATASET_DIR / "labels" / split).mkdir(parents=True, exist_ok=True)

    blender_pose_labels = sorted((BLENDER_DATASET_DIR / "labels_pose").glob("*.txt"))
    target_keypoint_count = _infer_pose_keypoint_count(blender_pose_labels)
    print(f"Ziel-Keypoint-Anzahl für Real-Datensatz: {target_keypoint_count}")

    # Inferenz + Keypoint-Extraktion
    valid_pairs = []
    skipped = 0

    for img_path in images:
        result = model(str(img_path), verbose=False)[0]

        if result.masks is None or len(result.masks) == 0:
            skipped += 1
            continue

        # Beste Detektion pro Klasse behalten
        segments = {}
        for mask_xy, cls, conf in zip(result.masks.xy, result.boxes.cls, result.boxes.conf):
            cid = int(cls)
            if cid not in segments or float(conf) > segments[cid][1]:
                segments[cid] = (mask_xy, float(conf))

        # Schwerpunkte als Keypoints aus den segmentierten Klassen
        keypoints = {}
        for cid in SEGMENT_KEYPOINT_IDS:
            if cid in segments:
                pts = segments[cid][0]
                keypoints[cid] = (np.mean(pts[:, 0]), np.mean(pts[:, 1]))

        if len(keypoints) >= 3:
            valid_pairs.append((img_path, keypoints))
        else:
            skipped += 1

    print(f"\n{len(valid_pairs)} Bilder mit ≥3 Joints erkannt")
    if skipped:
        print(f"{skipped} Bilder übersprungen (zu wenige Joints)")

    if not valid_pairs:
        print("FEHLER: Keine verwertbaren Bilder!")
        return

    # Train/Val Split
    random.seed(SEED)
    indices = list(range(len(valid_pairs)))
    random.shuffle(indices)
    val_count = int(len(indices) * VAL_SPLIT)
    splits = {"val": indices[:val_count], "train": indices[val_count:]}

    for split, idxs in splits.items():
        for idx in idxs:
            img_path, keypoints = valid_pairs[idx]

            # Bild kopieren
            shutil.copy2(img_path, REAL_POSE_DATASET_DIR / "images" / split / img_path.name)

            # Bildgröße
            img = cv2.imread(str(img_path))
            img_h, img_w = img.shape[:2]

            # Fehlende Keypoints mit 0/0/0 markieren, z. B. den TCP bei Real-Labels.
            kps = []
            for j in range(target_keypoint_count):
                if j in keypoints:
                    kx, ky = keypoints[j]
                    kps.append((kx / img_w, ky / img_h, 2))
                else:
                    kps.append((0.0, 0.0, 0))

            # BBox aus sichtbaren Keypoints
            visible_kps = [(kx * img_w, ky * img_h) for kx, ky, v in kps if v > 0]
            xs = [p[0] for p in visible_kps]
            ys = [p[1] for p in visible_kps]

            x_min = max(0, min(xs) - KEYPOINT_PADDING)
            x_max = min(img_w, max(xs) + KEYPOINT_PADDING)
            y_min = max(0, min(ys) - KEYPOINT_PADDING)
            y_max = min(img_h, max(ys) + KEYPOINT_PADDING)

            cx = ((x_min + x_max) / 2) / img_w
            cy = ((y_min + y_max) / 2) / img_h
            bw = (x_max - x_min) / img_w
            bh = (y_max - y_min) / img_h

            kp_str = " ".join(f"{kx:.6f} {ky:.6f} {int(v)}" for kx, ky, v in kps)
            label = f"0 {cx:.6f} {cy:.6f} {bw:.6f} {bh:.6f} {kp_str}"

            txt_name = img_path.stem + ".txt"
            with open(REAL_POSE_DATASET_DIR / "labels" / split / txt_name, "w") as f:
                f.write(label + "\n")

    print(f"Train: {len(splits['train'])}, Val: {len(splits['val'])}")

    yaml_content = _create_pose_yaml(REAL_POSE_DATASET_DIR, target_keypoint_count)
    with open(REAL_POSE_YAML, "w") as f:
        f.write(yaml_content)

    print(f"Dataset YAML: {REAL_POSE_YAML}")
    print(f"\n→ Training starten mit: python EdgeAI.py 2")


# ═════════════════════════════════════════════
# HAUPTFUNKTIONEN (Menü-Einträge)
# ═════════════════════════════════════════════

def daten_virtuell():
    """[1] Virtuell: Masken→Seg-Labels + Pose-Labels aufbereiten."""
    print("\n" + "#" * 50)
    print("  VIRTUELL — Daten vorbereiten")
    print("#" * 50)

    blender_masken_zu_labels()
    csv_zu_pose_labels()

    print("\n" + "#" * 50)
    print("  VIRTUELL KOMPLETT!")
    print(f"  Seg-Labels:  {BLENDER_DATASET_DIR}")
    print(f"  Pose-Labels: {POSE_DATASET_DIR}")
    print("#" * 50)
    print("\n→ Training starten mit: python EdgeAI.py 1")


def daten_echt():
    """[2] Echt: Auto-Annotation + Seg→Pose-Labels aufbereiten."""
    print("\n" + "#" * 50)
    print("  ECHT — Daten vorbereiten")
    print("#" * 50)

    auto_annotate()
    seg_zu_pose_labels_real()

    print("\n" + "#" * 50)
    print("  ECHT KOMPLETT!")
    print(f"  Seg-Labels:  {REAL_DATASET_DIR}")
    print(f"  Pose-Labels: {REAL_POSE_DATASET_DIR}")
    print("#" * 50)
    print("\n→ Training starten mit: python EdgeAI.py 2")


# ─────────────────────────────────────────────
# HAUPTPROGRAMM
# ─────────────────────────────────────────────

if __name__ == "__main__":
    steps = {
        "1": ("Virtuell — Daten vorbereiten (Seg + Pose)", daten_virtuell),
        "2": ("Echt — Daten vorbereiten (Seg + Pose)", daten_echt),
        "3": ("Winkelberechnung (Seg oder Pose)", winkel_berechnen),
    }

    if len(sys.argv) > 1 and sys.argv[1] in steps:
        choice = sys.argv[1]
    else:
        print("=" * 50)
        print("Roboterarm - Datenvorbereitung")
        print("=" * 50)
        for key, (desc, _) in steps.items():
            print(f"  [{key}] {desc}")
        print()
        choice = input("Schritt wählen (1-3): ").strip()

    if choice in steps:
        desc, func = steps[choice]
        print(f"\n>>> {desc} <<<\n")
        func()
    else:
        print("Ungültige Auswahl.")