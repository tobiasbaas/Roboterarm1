import bpy
import random
import math
import os
import csv
from bpy_extras.object_utils import world_to_camera_view

# =========================
# KONFIGURATION
# =========================
output_dir = r"C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\Segmentierung per Blender\dataset"

rgb_dir = os.path.join(output_dir, "images")
mask_dir = os.path.join(output_dir, "masks")
pose_labels_dir = os.path.join(output_dir, "labels_pose")

csv_path = os.path.join(output_dir, "joint_data.csv")
num_images = 10
rotation_jitter = 0.5

# Blender-Gelenke
joints_config = {
    "joint1": {"axis": 2, "min": -95.0, "max": 95.0},
    "joint2": {"axis": 1, "min": 41.0,  "max": -55.0},
    "joint3": {"axis": 1, "min": -80.0, "max": 80.0},
    "joint4": {"axis": 1, "min": 70.0,  "max": -110.0},
}

# Segmentfarben
COLOR_MAP = {
    "base": (1.0, 0.0, 0.0, 1.0),
    "j1":   (0.0, 1.0, 0.0, 1.0),
    "j2":   (0.0, 0.0, 1.0, 1.0),
    "j3":   (1.0, 1.0, 0.0, 1.0),
    "j4":   (1.0, 0.0, 1.0, 1.0),
    "tcp":  (1.0, 1.0, 1.0, 1.0),
}

# Reihenfolge = YOLO-Keypoint-Reihenfolge
# Fuer z-Achsen-Rotation erweitert:
# 0: base, 1: base_z1, 2: base_z2, 3: j1, 4: j1_z1, 5: j2, 6: j3, 7: j4, 8: tcp
KEYPOINT_OBJECT_CANDIDATES = [
    ("base",    ["KP_CAL_MARKER_base", "base_link", "base"]),
    ("base_z1", ["KP_CAL_MARKER_base_z1"]),
    ("base_z2", ["KP_CAL_MARKER_base_z2"]),
    ("j1",      ["KP_CAL_MARKER_j1", "joint1", "link1"]),
    ("j1_z1",   ["KP_CAL_MARKER_j1_z1"]),
    ("j2",      ["KP_CAL_MARKER_j2", "joint2", "link2"]),
    ("j3",      ["KP_CAL_MARKER_j3", "joint3", "link3"]),
    ("j4",      ["KP_CAL_MARKER_j4", "joint4", "link4"]),
    ("tcp",     ["KP_CAL_MARKER_tcp", "tcp_gripper", "tcp", "gripper_tip"]),
]

BBOX_PADDING = 0.10

# =========================
# VERZEICHNISSE
# =========================
for d in [output_dir, rgb_dir, mask_dir, pose_labels_dir]:
    os.makedirs(d, exist_ok=True)

# =========================
# HILFSFUNKTIONEN
# =========================
def find_first_existing(candidates):
    for n in candidates:
        obj = bpy.data.objects.get(n)
        if obj is not None:
            return obj
    return None

def project_point(scene, camera, obj):
    """
    Liefert normalisierte Koordinaten und Sichtbarkeit.
    YOLO pose visibility: 2 = sichtbar, 1 = verdeckt, 0 = nicht vorhanden
    Hier nutzen wir 2 (sichtbar) oder 0 (nicht im Bild / hinter Kamera).
    """
    if obj is None:
        return 0.0, 0.0, 0

    p = world_to_camera_view(scene, camera, obj.matrix_world.to_translation())
    x = float(p.x)
    y = 1.0 - float(p.y)
    z = float(p.z)

    in_front = z > 0.0
    in_frame = (0.0 <= x <= 1.0) and (0.0 <= y <= 1.0)
    v = 2 if (in_front and in_frame) else 0

    return x, y, v

def clamp01(v):
    return max(0.0, min(1.0, v))

def compute_bbox_from_visible_points(points_xy, pad=BBOX_PADDING):
    """
    points_xy: Liste von (x, y), nur sichtbare Punkte.
    """
    if len(points_xy) < 2:
        return None

    xs = [p[0] for p in points_xy]
    ys = [p[1] for p in points_xy]

    x_min = min(xs)
    x_max = max(xs)
    y_min = min(ys)
    y_max = max(ys)

    w = (x_max - x_min)
    h = (y_max - y_min)
    if w <= 1e-6 or h <= 1e-6:
        return None

    # Padding
    x_min -= w * pad
    x_max += w * pad
    y_min -= h * pad
    y_max += h * pad

    # Clamp ins Bild
    x_min = clamp01(x_min)
    x_max = clamp01(x_max)
    y_min = clamp01(y_min)
    y_max = clamp01(y_max)

    w2 = x_max - x_min
    h2 = y_max - y_min
    if w2 <= 1e-6 or h2 <= 1e-6:
        return None

    cx = (x_min + x_max) / 2.0
    cy = (y_min + y_max) / 2.0
    return cx, cy, w2, h2

def set_segment_colors():
    for obj in bpy.data.objects:
        if obj.type != "MESH":
            continue
        current = obj
        col = (0.5, 0.5, 0.5, 1.0)
        while current is not None:
            n = current.name.lower()
            if "base" in n:
                col = COLOR_MAP["base"]
                break
            elif "link1" in n or "joint1" in n:
                col = COLOR_MAP["j1"]
                break
            elif "link2" in n or "joint2" in n:
                col = COLOR_MAP["j2"]
                break
            elif "link3" in n or "joint3" in n:
                col = COLOR_MAP["j3"]
                break
            elif "link4" in n or "joint4" in n or "finger" in n:
                col = COLOR_MAP["j4"]
                break
            current = current.parent
        obj.color = col

# =========================
# INITIALISIERUNG
# =========================
scene = bpy.context.scene
cam = bpy.data.objects.get("Camera")
if cam is None:
    raise RuntimeError("Camera Objekt nicht gefunden.")

kp_items = []
missing = []
for tag, candidates in KEYPOINT_OBJECT_CANDIDATES:
    obj = find_first_existing(candidates)
    kp_items.append((tag, obj))
    if obj is None:
        missing.append((tag, candidates))

if missing:
    print("WARNUNG: Fehlende Keypoint-Objekte:")
    for tag, cands in missing:
        print(f"  {tag}: erwartet einen von {cands}")
    raise RuntimeError("Bitte Objekt-Namen korrigieren, damit keine falschen 0,0-Keypoints geschrieben werden.")

# Gelenkobjekte für Rotation prüfen
for j_name in joints_config.keys():
    if bpy.data.objects.get(j_name) is None:
        raise RuntimeError(f"Gelenkobjekt '{j_name}' nicht gefunden.")

# Kamera sichern
cam.constraints.clear()
orig_loc = cam.location.copy()
orig_rot = cam.rotation_euler.copy()

# Renderauflösung
scene.render.resolution_x = 1024
scene.render.resolution_y = 1024

# Farben für Segmentierung
set_segment_colors()

# =========================
# HAUPTSCHLEIFE
# =========================
with open(csv_path, mode="w", newline="") as f_csv:
    writer = csv.writer(f_csv)
    writer.writerow(["image", "angle_j1", "angle_j2", "angle_j3", "angle_j4"])

    generated = 0
    attempt = 0
    max_attempts = num_images * 5

    while generated < num_images and attempt < max_attempts:
        attempt += 1

        # 1) Roboter bewegen
        blender_angles = {}
        for j_name, cfg in joints_config.items():
            joint = bpy.data.objects.get(j_name)
            val = random.uniform(cfg["min"], cfg["max"])
            joint.rotation_euler[cfg["axis"]] = math.radians(val)
            blender_angles[j_name] = val

        # 2) Kamera-Jitter
        cam.location = orig_loc
        cam.rotation_euler.x = orig_rot.x + math.radians(random.uniform(-rotation_jitter, rotation_jitter))
        cam.rotation_euler.y = orig_rot.y + math.radians(random.uniform(-rotation_jitter, rotation_jitter))
        cam.rotation_euler.z = orig_rot.z + math.radians(random.uniform(-rotation_jitter, rotation_jitter))

        bpy.context.view_layer.update()

        # 3) Keypoints projizieren
        projected = []
        visible_points = []
        for tag, obj in kp_items:
            x, y, v = project_point(scene, cam, obj)
            projected.append((x, y, v))
            if v == 2:
                visible_points.append((x, y))

        # Nur Samples mit genug sichtbaren Punkten nutzen
        bbox = compute_bbox_from_visible_points(visible_points, pad=BBOX_PADDING)
        if bbox is None:
            continue

        cx, cy, w, h = bbox

        img_id = f"image_{generated:04d}"

        # 4) YOLO Pose Label schreiben
        kp_parts = []
        for x, y, v in projected:
            if v == 2:
                kp_parts.append(f"{x:.6f} {y:.6f} 2")
            else:
                kp_parts.append("0.000000 0.000000 0")

        label_file = os.path.join(pose_labels_dir, f"{img_id}.txt")
        with open(label_file, "w") as f_pose:
            f_pose.write(f"0 {cx:.6f} {cy:.6f} {w:.6f} {h:.6f} " + " ".join(kp_parts) + "\n")

        # 5) RGB rendern
        scene.render.engine = "BLENDER_EEVEE"
        scene.render.filepath = os.path.join(rgb_dir, f"{img_id}.png")
        bpy.ops.render.render(write_still=True)

        # 6) Segment-Maske rendern
        scene.render.engine = "BLENDER_WORKBENCH"
        scene.display.shading.light = "FLAT"
        scene.display.shading.color_type = "OBJECT"
        scene.display.render_aa = "OFF"
        scene.render.filepath = os.path.join(mask_dir, f"mask_{generated:04d}.png")
        bpy.ops.render.render(write_still=True)

        # 7) CSV
        writer.writerow([
            f"{img_id}.png",
            round(blender_angles["joint1"], 2),
            round(blender_angles["joint2"] + 40.0, 2),
            round(blender_angles["joint3"] - 20.0, 2),
            round(blender_angles["joint4"] - 20.0, 2),
        ])

        generated += 1
        if generated % 25 == 0:
            print(f"{generated}/{num_images} Samples erzeugt...")

# Kamera zurücksetzen
cam.location = orig_loc
cam.rotation_euler = orig_rot

print("Dataset-Generierung abgeschlossen.")
print(f"RGB:        {rgb_dir}")
print(f"Masken:     {mask_dir}")
print(f"PoseLabels: {pose_labels_dir}")