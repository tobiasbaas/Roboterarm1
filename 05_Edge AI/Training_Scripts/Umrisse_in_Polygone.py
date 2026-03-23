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

SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent


def _path_for_yaml(target: Path, yaml_file: Path) -> str:
    """Nutze relative Pfade in YAML, damit Kopieren zwischen Rechnern robust bleibt."""
    try:
        return target.resolve().relative_to(yaml_file.parent.resolve()).as_posix()
    except ValueError:
        return target.resolve().as_posix()


def _find_image_capture_root() -> Path:
    """Sucht den Image_Capture_Python-Root automatisch oder nutzt ENV-Override."""
    env_root = os.getenv("IMAGE_CAPTURE_PYTHON_ROOT")
    if env_root:
        return Path(env_root).expanduser().resolve()

    for parent in [PROJECT_ROOT, *PROJECT_ROOT.parents]:
        candidate = parent / "Image_Capture_Python"
        if candidate.exists():
            return candidate.resolve()

    # Fallback: relative Standard-Annahme beibehalten.
    return (PROJECT_ROOT.parent / "Image_Capture_Python").resolve()


def _has_images(path: Path) -> bool:
    """Prüft, ob in einem Ordner PNG/JPG/JPEG-Bilder vorhanden sind."""
    return any(path.glob("*.png")) or any(path.glob("*.jpg")) or any(path.glob("*.jpeg"))


def _resolve_real_images_dir(images_base_dir: Path) -> Path:
    """Ermittelt den effektiv zu nutzenden Realbild-Ordner (neueste Session bevorzugt)."""
    env_images = os.getenv("IMAGE_CAPTURE_IMAGES_DIR")
    if env_images:
        return Path(env_images).expanduser().resolve()

    if _has_images(images_base_dir):
        return images_base_dir

    candidates = [
        p for p in images_base_dir.iterdir()
        if p.is_dir() and _has_images(p)
    ] if images_base_dir.exists() else []

    if candidates:
        # Neueste Session bevorzugen.
        return max(candidates, key=lambda p: p.stat().st_mtime)

    return images_base_dir


def _resolve_ground_truth_csv(dataset_dir: Path, images_dir: Path) -> Path:
    """Bestimmt die passendste Ground-Truth-CSV für den gewählten Bildordner."""
    env_csv = os.getenv("IMAGE_CAPTURE_LABELS_CSV")
    if env_csv:
        return Path(env_csv).expanduser().resolve()

    # Falls der Bildordner eine Session-ID im Namen trägt, passendes labels_<session>.csv bevorzugen.
    if images_dir.name and images_dir.name != "images":
        by_name = dataset_dir / f"labels_{images_dir.name}.csv"
        if by_name.exists():
            return by_name

    csv_candidates = sorted(dataset_dir.glob("labels*.csv"), key=lambda p: p.stat().st_mtime, reverse=True)
    if csv_candidates:
        return csv_candidates[0]

    return dataset_dir / "labels.csv"

# Schritt 1: Blender-Masken → Labels
BLENDER_DATASET_DIR = SCRIPT_DIR / "Segmentierung per Blender" / "dataset"

# Schritt 2: CSV → Pose-Labels
BLENDER_CSV = BLENDER_DATASET_DIR / "joint_data.csv"
POSE_DATASET_DIR = SCRIPT_DIR / "Segmentierung per Blender" / "pose_dataset"
POSE_YAML = SCRIPT_DIR / "Segmentierung per Blender" / "pose_dataset.yaml"
KEYPOINT_PADDING = 30

# Schritt 3: Auto-Annotation
PRETRAINED_MODEL = PROJECT_ROOT / "Trained_Models" / "AI-Models" / "roboterarm_seg" / "weights" / "best.pt"
IMAGE_CAPTURE_ROOT = _find_image_capture_root()
IMAGE_CAPTURE_DATASET_DIR = IMAGE_CAPTURE_ROOT / "dataset"
REAL_IMAGES_BASE_DIR = IMAGE_CAPTURE_DATASET_DIR / "images"
REAL_IMAGES_DIR = _resolve_real_images_dir(REAL_IMAGES_BASE_DIR)
REAL_DATASET_DIR = SCRIPT_DIR / "real_dataset"
CONF_THRESHOLD = 0.5

# Schritt 4: Winkelberechnung
MODEL_OUTPUT_DIR = PROJECT_ROOT / "Trained_Models" / "AI-Models"
GROUND_TRUTH_CSV = _resolve_ground_truth_csv(IMAGE_CAPTURE_DATASET_DIR, REAL_IMAGES_DIR)
WINKEL_OUTPUT_DIR = SCRIPT_DIR / "winkel_output"

# Schritt 5: Seg → Pose-Labels (echte Bilder)
REAL_POSE_DATASET_DIR = SCRIPT_DIR / "real_pose_dataset"
REAL_POSE_YAML = SCRIPT_DIR / "real_pose_dataset.yaml"

# Allgemein
IMG_SIZE = 640
VAL_SPLIT = 0.3
SEED = 42
SEGMENT_KEYPOINT_IDS = [0, 1, 2, 3, 4]
KP_IDX = {
    "base": 0,
    "base_z1": 1,
    "base_z2": 2,
    "j1": 3,
    "j1_z1": 4,
    "j2": 5,
    "j3": 6,
    "j4": 7,
    "tcp": 8,
}
DEFAULT_POSE_KEYPOINT_COUNT = 9

# Real-Pose Pseudo-Labeling: fehlende Keypoints heuristisch vorbesetzen,
# damit in Label Studio alle 9 Keypoint-Labels direkt sichtbar/editierbar sind.
SYNTHESIZE_EXTRA_REAL_POSE_KEYPOINTS = True

# Mapping von Segment-Klassen auf Pose-Keypoint-Indizes.
# Zusätzliche Z-Achsen-Referenzpunkte (base_z1/base_z2/j1_z1) sind in Segmentierung
# nicht direkt ableitbar und bleiben bei Seg->Pose als unsichtbar (0,0,0).
SEGMENT_CLASS_TO_KP_INDEX = {
    0: KP_IDX["base"],
    1: KP_IDX["j1"],
    2: KP_IDX["j2"],
    3: KP_IDX["j3"],
    4: KP_IDX["j4"],
}

POSE_CONNECTIONS = [
    (KP_IDX["base"], KP_IDX["base_z1"]),
    (KP_IDX["base"], KP_IDX["base_z2"]),
    (KP_IDX["j1"], KP_IDX["j1_z1"]),
    (KP_IDX["base"], KP_IDX["j1"]),
    (KP_IDX["j1"], KP_IDX["j2"]),
    (KP_IDX["j2"], KP_IDX["j3"]),
    (KP_IDX["j3"], KP_IDX["j4"]),
    (KP_IDX["j4"], KP_IDX["tcp"]),
]

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
    """Erzeugt den YAML-Inhalt für ein YOLO-Pose-Dataset."""
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
    """Berechnet den signierten Winkel zwischen zwei 2D-Vektoren in Grad."""
    return np.degrees(np.arctan2(
        reference_vector[0] * target_vector[1] - reference_vector[1] * target_vector[0],
        np.dot(reference_vector, target_vector)
    ))


def _vector_between(joint_points, start_idx, end_idx):
    """Liefert den Verbindungsvektor zwischen zwei vorhandenen Keypoints."""
    if start_idx not in joint_points or end_idx not in joint_points:
        return None
    return joint_points[end_idx] - joint_points[start_idx]


def _normalize(vec):
    """Normalisiert einen Vektor; bei nahezu Nullvektor wird None zurückgegeben."""
    n = np.linalg.norm(vec)
    if n < 1e-6:
        return None
    return vec / n


def _perp(vec):
    """Erzeugt einen 2D-Senkrechtvektor (90° Rotation) zum Eingangsvektor."""
    return np.array([-vec[1], vec[0]], dtype=float)


def _infer_extra_pose_keypoints(keypoints):
    """Ergänzt fehlende 9er-Keypoints heuristisch als vis=1 (hinweisartig)."""
    out = dict(keypoints)

    base = out.get(KP_IDX["base"])
    j1 = out.get(KP_IDX["j1"])
    j2 = out.get(KP_IDX["j2"])
    j3 = out.get(KP_IDX["j3"])
    j4 = out.get(KP_IDX["j4"])

    if base is not None and j1 is not None:
        b = np.array(base[:2], dtype=float)
        j = np.array(j1[:2], dtype=float)
        dir_bj = _normalize(j - b)
        if dir_bj is not None:
            length = max(np.linalg.norm(j - b) * 0.25, 12.0)
            n = _perp(dir_bj)
            if KP_IDX["base_z1"] not in out:
                out[KP_IDX["base_z1"]] = (float(b[0] + n[0] * length), float(b[1] + n[1] * length), 1)
            if KP_IDX["base_z2"] not in out:
                out[KP_IDX["base_z2"]] = (float(b[0] - n[0] * length), float(b[1] - n[1] * length), 1)

    if j1 is not None and j2 is not None and KP_IDX["j1_z1"] not in out:
        j = np.array(j1[:2], dtype=float)
        k = np.array(j2[:2], dtype=float)
        dir_j1j2 = _normalize(k - j)
        if dir_j1j2 is not None:
            length = max(np.linalg.norm(k - j) * 0.2, 10.0)
            n = _perp(dir_j1j2)
            out[KP_IDX["j1_z1"]] = (float(j[0] + n[0] * length), float(j[1] + n[1] * length), 1)

    if j4 is not None and j3 is not None and KP_IDX["tcp"] not in out:
        p4 = np.array(j4[:2], dtype=float)
        p3 = np.array(j3[:2], dtype=float)
        dir_j3j4 = _normalize(p4 - p3)
        if dir_j3j4 is not None:
            length = max(np.linalg.norm(p4 - p3) * 0.8, 16.0)
            out[KP_IDX["tcp"]] = (float(p4[0] + dir_j3j4[0] * length), float(p4[1] + dir_j3j4[1] * length), 1)

    return out


def _predict_joint_angles(joint_points):
    """Berechnet bis zu vier Gelenkwinkel aus der Keypoint-Kette."""
    predicted = {}
    ref_up = np.array([0.0, -1.0])

    base_vector = _vector_between(joint_points, KP_IDX["base"], KP_IDX["j1"])
    if base_vector is not None:
        predicted[0] = _joint_angle(ref_up, base_vector)

    joint_2_vector = _vector_between(joint_points, KP_IDX["j1"], KP_IDX["j2"])
    if base_vector is not None and joint_2_vector is not None:
        predicted[1] = _joint_angle(base_vector, joint_2_vector)

    joint_3_vector = _vector_between(joint_points, KP_IDX["j2"], KP_IDX["j3"])
    if joint_2_vector is not None and joint_3_vector is not None:
        predicted[2] = _joint_angle(joint_2_vector, joint_3_vector)

    joint_4_vector = _vector_between(joint_points, KP_IDX["j3"], KP_IDX["j4"])
    tcp_vector = _vector_between(joint_points, KP_IDX["j4"], KP_IDX["tcp"])
    if joint_4_vector is not None and tcp_vector is not None:
        predicted[3] = _joint_angle(joint_4_vector, tcp_vector)
    elif joint_3_vector is not None and joint_4_vector is not None:
        predicted[3] = _joint_angle(joint_3_vector, joint_4_vector)

    return predicted

def blender_masken_zu_labels():
    """Konvertiert farbige Blender-Masken in YOLO-Segmentierungs-Labels."""
    masks_dir = BLENDER_DATASET_DIR / "masks"
    labels_dir = BLENDER_DATASET_DIR / "labels"
    labels_dir.mkdir(parents=True, exist_ok=True)

    mask_files = sorted(masks_dir.glob("mask_*.png"))

    print(f"Gefunden: {len(mask_files)} Masken-Dateien")

    if not mask_files:
        print(f"FEHLER: Keine Masken gefunden in {masks_dir}")
        return

    for mask_path in mask_files:
        img_bgr = cv2.imread(str(mask_path))
        if img_bgr is None:
            continue

        img_hsv = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2HSV)
        height, width = img_hsv.shape[:2]

        base_name = mask_path.name.replace("mask_", "image_").replace(".png", ".txt")
        txt_path = labels_dir / base_name

        with open(txt_path, "w", encoding="utf-8") as f:
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

    print(f"Fertig! {len(mask_files)} Label-Dateien generiert in: {labels_dir}")




def csv_zu_pose_labels():
    """Erstellt YOLO-Pose Dataset aus vorgenerierten Labels in labels_pose/ + Train/Val Split.

    Die Pose-Labels werden direkt von Blender generiert
    und liegen bereits als YOLO-Format in dataset/labels_pose/.
    Die joint_data.csv enthält nur Winkel, keine Pixel-Koordinaten.
    """

    labels_dir = BLENDER_DATASET_DIR / "labels_pose"
    image_dir = BLENDER_DATASET_DIR / "images"
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
        img_path = image_dir / (lbl_path.stem + ".png")
        if img_path.exists():
            valid_pairs.append((img_path, lbl_path))

    print(f"Bilder mit Labels: {len(valid_pairs)}")
    if not valid_pairs:
        print(f"FEHLER: Keine passenden Bilder in {image_dir} gefunden!")
        return

    # Ausgabe-Ordner erstellen
    for split in ["train", "val"]:
        img_split = POSE_DATASET_DIR / "images" / split
        lbl_split = POSE_DATASET_DIR / "labels" / split
        img_split.mkdir(parents=True, exist_ok=True)
        lbl_split.mkdir(parents=True, exist_ok=True)

        # Alte Daten entfernen, damit Splits konsistent bleiben.
        for p in img_split.glob("*"):
            if p.is_file():
                p.unlink()
        for p in lbl_split.glob("*"):
            if p.is_file():
                p.unlink()

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

    yaml_content = _create_pose_yaml(_path_for_yaml(POSE_DATASET_DIR, POSE_YAML), keypoint_count)
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

    # Alte Dateien entfernen, damit keine stale Auto-Annotationen bleiben.
    for p in img_dir.glob("*"):
        if p.is_file():
            p.unlink()
    for p in lbl_dir.glob("*"):
        if p.is_file():
            p.unlink()

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
                    kp_idx = SEGMENT_CLASS_TO_KP_INDEX[cid]
                    joint_points[kp_idx] = np.array([np.mean(pts[:, 0]), np.mean(pts[:, 1])])

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
        img_split = REAL_POSE_DATASET_DIR / "images" / split
        lbl_split = REAL_POSE_DATASET_DIR / "labels" / split
        img_split.mkdir(parents=True, exist_ok=True)
        lbl_split.mkdir(parents=True, exist_ok=True)

        # Alte Dateien entfernen, damit keine stale Train/Val-Daten bleiben.
        for p in img_split.glob("*"):
            if p.is_file():
                p.unlink()
        for p in lbl_split.glob("*"):
            if p.is_file():
                p.unlink()

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
                kp_idx = SEGMENT_CLASS_TO_KP_INDEX[cid]
                keypoints[kp_idx] = (float(np.mean(pts[:, 0])), float(np.mean(pts[:, 1])), 2)

        if SYNTHESIZE_EXTRA_REAL_POSE_KEYPOINTS:
            keypoints = _infer_extra_pose_keypoints(keypoints)

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
                    kx, ky, vis = keypoints[j]
                    kx = max(0.0, min(float(img_w - 1), float(kx)))
                    ky = max(0.0, min(float(img_h - 1), float(ky)))
                    kps.append((kx / img_w, ky / img_h, int(vis)))
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

    yaml_content = _create_pose_yaml(_path_for_yaml(REAL_POSE_DATASET_DIR, REAL_POSE_YAML), target_keypoint_count)
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
    print(f"  Seg-Labels:  {BLENDER_DATASET_DIR / 'labels'}")
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