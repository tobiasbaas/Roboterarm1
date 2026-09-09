"""
Blender-Datengenerierung für Roboterarm-Segmentierung und Pose.

Dieses Skript erzeugt pro Sample:
1) RGB-Bild,
2) Segmentierungsmaske,
3) YOLO-Pose-Label,
4) CSV-Eintrag mit den verwendeten Gelenkwinkeln.

Ziel ist ein konsistentes synthetisches Trainingsset für die nachgelagerten
Trainingsskripte im Projektordner Training_Scripts.
"""

import bpy
import random
import math
import os
import csv
import numpy as np
from bpy_extras.object_utils import world_to_camera_view
from mathutils import Vector

# Dieses Skript erzeugt pro Sample:
# 1) ein RGB-Bild,
# 2) eine Segmentierungsmaske,
# 3) ein YOLO-Pose-Label mit Keypoints,
# 4) eine CSV mit den verwendeten Gelenkwinkeln.
#
# Wichtiger Punkt fuer die Keypoints:
# Die Labelpositionen werden aus Anchor + lokalem Offset berechnet.
# Dadurch bleiben Keypoints stabil an der Mechanik gekoppelt, auch wenn
# die Markerobjekte in der Szene unsichtbar sind.

# =========================
# KONFIGURATION
# =========================
def _resolve_base_dir():
    """Ermittelt robust das Basisverzeichnis fuer Output-Dateien.

    Prioritaet:
    1) Ordner der aktuell geladenen .blend-Datei,
    2) Ordner des Python-Skripts,
    3) aktuelles Arbeitsverzeichnis.
    """
    # Bevorzugt den Blender-Pfad "//" (Ordner der .blend-Datei), weil er robust
    # fuer relative Projektpfade ist.
    if bpy.data.filepath:
        blend_base = bpy.path.abspath("//")
        if blend_base:
            return os.path.abspath(blend_base)

    # Fallback: Dateipfad des Python-Skripts.
    script_file = globals().get("__file__")
    if script_file:
        return os.path.dirname(os.path.abspath(script_file))

    # Letzter Fallback: aktuelles Arbeitsverzeichnis.
    return os.path.abspath(os.getcwd())


base_dir = _resolve_base_dir()
output_dir = os.path.join(base_dir, "dataset")

rgb_dir = os.path.join(output_dir, "images")
mask_dir = os.path.join(output_dir, "masks")
pose_labels_dir = os.path.join(output_dir, "labels_pose")

csv_path = os.path.join(output_dir, "joint_data.csv")
num_images = 2000
rotation_jitter = 0.5  # Legacy, wird durch DOMAIN_RAND ersetzt

# ── Domain Randomization ─────────────────────────────────────────────────────
# Variiert Beleuchtung, Kamera und Hintergrund pro Sample,
# um den Domain Gap zwischen synthetischen und echten Bildern zu reduzieren.
DOMAIN_RAND = {
    "camera_loc_jitter": 0.03,          # Max Verschiebung pro Achse (Meter)
    "camera_rot_jitter": 2.0,           # Max Rotation pro Achse (Grad)
    "light_energy_range": (0.4, 2.5),   # Multiplikator auf Original-Intensitaet
    "light_color_jitter": 0.15,         # Max Abweichung der Lichtfarbe (RGB)
    "world_color_random": True,         # Zufaellige Hintergrundfarbe
    "world_color_range": (0.01, 0.25),  # Helligkeitsbereich fuer Hintergrund
    "exposure_range": (-2.0, 1.5),      # Belichtung: negativ=dunkel, positiv=hell
    "noise_strength_range": (0.0, 0.12),# Gausssches Bildrauschen (Std auf 0-1 Skala)
}

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
#
# Wenn local_offset None ist, wird er einmalig aus anchor + reference_candidates
# (typischerweise KP_CAL_MARKER_*) berechnet.
#
# Optional: Trage die kalibrierten Werte aus keypoint_axis_calibration.py direkt
# in local_offset ein, dann sind Marker in der Datengenerierung nicht erforderlich.
KEYPOINT_SPECS = [
    {
        "name": "base",
        "anchor_candidates": ["base_link", "base"],
        "reference_candidates": ["KP_CAL_MARKER_base", "base_link", "base"],
        "local_offset": None,
    },
    {
        "name": "base_z1",
        "anchor_candidates": ["base_link", "base"],
        "reference_candidates": ["KP_CAL_MARKER_base_z1"],
        "local_offset": None,
    },
    {
        "name": "base_z2",
        "anchor_candidates": ["base_link", "base"],
        "reference_candidates": ["KP_CAL_MARKER_base_z2"],
        "local_offset": None,
    },
    {
        "name": "j1",
        "anchor_candidates": ["joint1", "link1"],
        "reference_candidates": ["KP_CAL_MARKER_j1", "joint1", "link1"],
        "local_offset": None,
    },
    {
        "name": "j1_z1",
        "anchor_candidates": ["joint1", "link1"],
        "reference_candidates": ["KP_CAL_MARKER_j1_z1"],
        "local_offset": None,
    },
    {
        "name": "j2",
        "anchor_candidates": ["joint2", "link2"],
        "reference_candidates": ["KP_CAL_MARKER_j2", "joint2", "link2"],
        "local_offset": None,
    },
    {
        "name": "j3",
        "anchor_candidates": ["joint3", "link3"],
        "reference_candidates": ["KP_CAL_MARKER_j3", "joint3", "link3"],
        "local_offset": None,
    },
    {
        "name": "j4",
        "anchor_candidates": ["joint4", "link4"],
        "reference_candidates": ["KP_CAL_MARKER_j4", "joint4", "link4"],
        "local_offset": None,
    },
    {
        "name": "tcp",
        "anchor_candidates": ["tcp_gripper", "tcp", "gripper_tip"],
        "reference_candidates": ["KP_CAL_MARKER_tcp", "tcp_gripper", "tcp", "gripper_tip"],
        "local_offset": None,
    },
]

BBOX_PADDING = 0.10
RENDER_HELPER_PREFIXES = ("KP_CAL_MARKER_", "kp_marker_", "KP_REF_")

# Sanity-Checks, um instabile/degenerierte Pose-Samples auszusortieren.
MIN_VISIBLE_KEYPOINTS = 6
MIN_BBOX_W = 0.05
MIN_BBOX_H = 0.05

# =========================
# VERZEICHNISSE
# =========================
for d in [output_dir, rgb_dir, mask_dir, pose_labels_dir]:
    os.makedirs(d, exist_ok=True)

# =========================
# HILFSFUNKTIONEN
# =========================
def find_first_existing(candidates):
    """Liefert das erste in Blender vorhandene Objekt aus einer Kandidatenliste."""
    for n in candidates:
        obj = bpy.data.objects.get(n)
        if obj is not None:
            return obj
    return None

def compute_local_offset(anchor_obj, reference_obj):
    """Berechnet den lokalen Offset von reference relativ zum Anchor.

    Ergebnis ist ein Punkt im lokalen Koordinatensystem des Anchors.
    """
    local = anchor_obj.matrix_world.inverted() @ reference_obj.matrix_world.translation
    return (float(local.x), float(local.y), float(local.z))

def world_from_anchor_offset(anchor_obj, local_offset):
    """Transformiert einen lokalen Anchor-Offset in eine Weltposition."""
    local_vec = Vector(local_offset)
    return anchor_obj.matrix_world @ local_vec

def project_world_point(scene, camera, world_pos):
    """
    Liefert normalisierte Koordinaten und Sichtbarkeit.
    YOLO pose visibility: 2 = sichtbar, 1 = verdeckt, 0 = nicht vorhanden
    Hier nutzen wir 2 (sichtbar) oder 0 (nicht im Bild / hinter Kamera).
    """
    if world_pos is None:
        return 0.0, 0.0, 0

    p = world_to_camera_view(scene, camera, world_pos)
    x = float(p.x)
    y = 1.0 - float(p.y)
    z = float(p.z)

    in_front = z > 0.0
    in_frame = (0.0 <= x <= 1.0) and (0.0 <= y <= 1.0)
    v = 2 if (in_front and in_frame) else 0

    return x, y, v

def clamp01(v):
    """Begrenzt einen Wert auf den Bereich [0, 1]."""
    return max(0.0, min(1.0, v))

def is_finite_number(v):
    """Prueft robust auf endliche numerische Werte."""
    return isinstance(v, (int, float)) and math.isfinite(v)

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
    """Faerbt Meshes anhand ihrer Namen/Hierarchie fuer Segment-Masken.

    Die Objektfarbe wird spaeter im Workbench-Render als Klassenfarbe genutzt.
    """
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

def collect_render_helper_objects():
    """Sammelt Hilfsobjekte (Marker/Referenzen), die nicht ins Render sollen."""
    helpers = []
    for obj in bpy.data.objects:
        if obj.name.startswith(RENDER_HELPER_PREFIXES):
            helpers.append(obj)
    return helpers

def set_helpers_render_visibility(helpers, visible):
    # Helperobjekte sind nur fuer Kalibrierung/Label-Berechnung da.
    # In RGB- und Maskenbildern wuerden sie sonst Artefakte erzeugen.
    for obj in helpers:
        obj.hide_render = not visible

def snapshot_render_visibility(objs):
    """Merkt den aktuellen hide_render-Zustand zur spaeteren Wiederherstellung."""
    return {obj.name: obj.hide_render for obj in objs}

def restore_render_visibility(objs, states):
    """Stellt den vorher gesicherten hide_render-Zustand wieder her."""
    for obj in objs:
        obj.hide_render = states.get(obj.name, obj.hide_render)

def collect_scene_lights():
    """Sammelt alle Lichter in der Szene."""
    return [obj for obj in bpy.data.objects if obj.type == 'LIGHT']


def snapshot_lights(lights):
    """Speichert den Originalzustand aller Lichter."""
    return {l.name: {'energy': l.data.energy, 'color': tuple(l.data.color)} for l in lights}


def snapshot_world():
    """Speichert den Originalzustand des World-Hintergrunds."""
    world = bpy.context.scene.world
    if not world:
        return None
    if world.use_nodes:
        bg = world.node_tree.nodes.get('Background')
        if bg:
            return {
                'use_nodes': True,
                'color': tuple(bg.inputs['Color'].default_value),
                'strength': bg.inputs['Strength'].default_value,
            }
    return {'use_nodes': False, 'color': tuple(world.color)}


def randomize_camera(cam_obj, orig_l, orig_r, cfg):
    """Kamera-Position und Rotation zufaellig variieren."""
    jl = cfg["camera_loc_jitter"]
    jr = cfg["camera_rot_jitter"]
    cam_obj.location.x = orig_l.x + random.uniform(-jl, jl)
    cam_obj.location.y = orig_l.y + random.uniform(-jl, jl)
    cam_obj.location.z = orig_l.z + random.uniform(-jl, jl)
    cam_obj.rotation_euler.x = orig_r.x + math.radians(random.uniform(-jr, jr))
    cam_obj.rotation_euler.y = orig_r.y + math.radians(random.uniform(-jr, jr))
    cam_obj.rotation_euler.z = orig_r.z + math.radians(random.uniform(-jr, jr))


def randomize_lighting(lights, originals, cfg):
    """Lichtintensitaet und -farbe zufaellig variieren."""
    lo, hi = cfg["light_energy_range"]
    cj = cfg["light_color_jitter"]
    for l in lights:
        orig = originals[l.name]
        l.data.energy = orig['energy'] * random.uniform(lo, hi)
        r, g, b = orig['color']
        l.data.color = (
            max(0.0, min(1.0, r + random.uniform(-cj, cj))),
            max(0.0, min(1.0, g + random.uniform(-cj, cj))),
            max(0.0, min(1.0, b + random.uniform(-cj, cj))),
        )


def randomize_world_background(cfg):
    """Hintergrundfarbe der Welt zufaellig variieren (nur fuer EEVEE-Render)."""
    world = bpy.context.scene.world
    if not world or not cfg.get("world_color_random"):
        return
    lo, hi = cfg["world_color_range"]
    if world.use_nodes:
        bg = world.node_tree.nodes.get('Background')
        if bg:
            bg.inputs['Color'].default_value = (
                random.uniform(lo, hi),
                random.uniform(lo, hi),
                random.uniform(lo, hi),
                1.0,
            )
    else:
        world.color = (random.uniform(lo, hi), random.uniform(lo, hi), random.uniform(lo, hi))


def restore_lights(lights, originals):
    """Stellt den Originalzustand aller Lichter wieder her."""
    for l in lights:
        orig = originals.get(l.name)
        if orig:
            l.data.energy = orig['energy']
            l.data.color = orig['color']


def restore_world(snap):
    """Stellt den Originalzustand des World-Hintergrunds wieder her."""
    if snap is None:
        return
    world = bpy.context.scene.world
    if not world:
        return
    if snap.get('use_nodes') and world.use_nodes:
        bg = world.node_tree.nodes.get('Background')
        if bg:
            bg.inputs['Color'].default_value = snap['color']
            bg.inputs['Strength'].default_value = snap['strength']
    elif not snap.get('use_nodes'):
        world.color = snap['color'][:3]


def randomize_exposure(scn, cfg):
    """Belichtung (Helligkeit) zufaellig variieren.

    Negative Werte = dunkel (Roboter schwer erkennbar),
    positive Werte = ueberbelichtet.
    """
    lo, hi = cfg["exposure_range"]
    scn.view_settings.exposure = random.uniform(lo, hi)


def apply_post_noise(filepath, cfg):
    """Legt Gausssches Rauschen auf ein gerendertes PNG-Bild.

    Das Rauschen wird nur auf den RGB-Render angewendet,
    nicht auf die Segmentierungsmaske.
    """
    lo, hi = cfg["noise_strength_range"]
    noise_std = random.uniform(lo, hi)
    if noise_std <= 0.001:
        return
    img = bpy.data.images.load(filepath, check_existing=False)
    w, h = img.size
    pixels = np.array(img.pixels[:], dtype=np.float32).reshape(h, w, 4)
    noise = np.random.normal(0.0, noise_std, (h, w, 3)).astype(np.float32)
    pixels[:, :, :3] = np.clip(pixels[:, :, :3] + noise, 0.0, 1.0)
    img.pixels[:] = pixels.flatten().tolist()
    img.filepath_raw = filepath
    img.file_format = 'PNG'
    img.save()
    bpy.data.images.remove(img)


def resolve_keypoints(specs):
    """Loest Keypoint-Spezifikationen in konkrete, nutzbare Eintraege auf.

    Pro Keypoint werden aufgeloest:
    - ein Anchor-Objekt,
    - ein lokaler Offset.

    Falls local_offset in der Spec fehlt, wird er aus Anchor + Referenzobjekt
    berechnet (typisch ueber KP_CAL_MARKER_*).
    """
    resolved = []
    errors = []

    for spec in specs:
        name = spec["name"]
        anchor = find_first_existing(spec["anchor_candidates"])
        if anchor is None:
            errors.append(
                f"{name}: Anchor fehlt. Kandidaten={spec['anchor_candidates']}"
            )
            continue

        local_offset = spec.get("local_offset")
        if local_offset is None:
            ref = find_first_existing(spec.get("reference_candidates", []))
            if ref is None:
                errors.append(
                    f"{name}: Kein Referenzobjekt gefunden. Kandidaten={spec.get('reference_candidates', [])}"
                )
                continue
            local_offset = compute_local_offset(anchor, ref)

        resolved.append(
            {
                "name": name,
                "anchor": anchor,
                "local_offset": tuple(float(v) for v in local_offset),
            }
        )

    return resolved, errors

# =========================
# INITIALISIERUNG
# =========================
scene = bpy.context.scene
cam = bpy.data.objects.get("Camera")
if cam is None:
    raise RuntimeError("Camera Objekt nicht gefunden.")

# Keypoints einmalig vorbereiten (Anchor + Offset aufloesen).
# So kann die Hauptschleife pro Frame nur noch transformieren/projizieren.
kp_items, kp_errors = resolve_keypoints(KEYPOINT_SPECS)
if kp_errors:
    print("WARNUNG: Keypoint-Aufloesung fehlgeschlagen:")
    for err in kp_errors:
        print(f"  - {err}")
    raise RuntimeError("Keypoint-Specs unvollstaendig. Bitte Anchor/Reference pruefen.")

print("Verwendete Keypoint-Offsets (lokal relativ zu Anchor):")
for item in kp_items:
    ox, oy, oz = item["local_offset"]
    print(f"  {item['name']}: anchor={item['anchor'].name}, offset=({ox:.6f}, {oy:.6f}, {oz:.6f})")

# Gelenkobjekte fuer Rotation pruefen, damit beim Sampling kein KeyError entsteht.
for j_name in joints_config.keys():
    if bpy.data.objects.get(j_name) is None:
        raise RuntimeError(f"Gelenkobjekt '{j_name}' nicht gefunden.")

# Kamera-Zustand sichern und Constraints leeren, damit der Jitter reproduzierbar
# direkt auf location/rotation geschrieben werden kann.
cam.constraints.clear()
orig_loc = cam.location.copy()
orig_rot = cam.rotation_euler.copy()

# Renderauflösung
scene.render.resolution_x = 1024
scene.render.resolution_y = 1024

# Farben für Segmentierung
set_segment_colors()

# Marker nur für Keypoint-Berechnung verwenden, nicht rendern.
render_helpers = collect_render_helper_objects()
helpers_original_render_state = snapshot_render_visibility(render_helpers)
set_helpers_render_visibility(render_helpers, visible=False)

# Domain Randomization: Originalzustaende sichern
scene_lights = collect_scene_lights()
lights_original_state = snapshot_lights(scene_lights)
world_original_state = snapshot_world()
exposure_original = scene.view_settings.exposure
print(f"Domain Randomization aktiv: {len(scene_lights)} Lichter gefunden.")
print(f"  Kamera-Jitter: loc={DOMAIN_RAND['camera_loc_jitter']}m, rot={DOMAIN_RAND['camera_rot_jitter']}°")
print(f"  Licht-Energie: {DOMAIN_RAND['light_energy_range']}, Farb-Jitter: {DOMAIN_RAND['light_color_jitter']}")
print(f"  Hintergrund: {'zufaellig' if DOMAIN_RAND['world_color_random'] else 'fest'}")
print(f"  Belichtung: {DOMAIN_RAND['exposure_range']} (Original: {exposure_original})")
print(f"  Rauschen: {DOMAIN_RAND['noise_strength_range']}")

# =========================
# HAUPTSCHLEIFE
# =========================
try:
    with open(csv_path, mode="w", newline="") as f_csv:
        writer = csv.writer(f_csv)
        writer.writerow(["image", "angle_j1", "angle_j2", "angle_j3", "angle_j4"])

        generated = 0
        attempt = 0
        max_attempts = num_images * 5
        skipped_not_enough_visible = 0
        skipped_bad_bbox = 0
        skipped_non_finite = 0

        while generated < num_images and attempt < max_attempts:
            attempt += 1

            # 1) Roboter bewegen (zufaellige Winkel innerhalb der Gelenkgrenzen).
            blender_angles = {}
            for j_name, cfg in joints_config.items():
                joint = bpy.data.objects.get(j_name)
                val = random.uniform(cfg["min"], cfg["max"])
                joint.rotation_euler[cfg["axis"]] = math.radians(val)
                blender_angles[j_name] = val

            # 2) Domain Randomization: Kamera, Beleuchtung, Hintergrund, Exposure.
            randomize_camera(cam, orig_loc, orig_rot, DOMAIN_RAND)
            randomize_lighting(scene_lights, lights_original_state, DOMAIN_RAND)
            randomize_world_background(DOMAIN_RAND)
            randomize_exposure(scene, DOMAIN_RAND)

            # Wichtig: Blender-Dependency-Graph aktualisieren, damit
            # matrix_world fuer alle Objekte den neuen Pose-Zustand enthaelt.
            bpy.context.view_layer.update()

            # 3) Keypoints projizieren:
            #    Weltpunkt = Anchor-Transform * lokaler Offset.
            projected = []
            visible_points = []
            for item in kp_items:
                world_pos = world_from_anchor_offset(item["anchor"], item["local_offset"])
                x, y, v = project_world_point(scene, cam, world_pos)
                projected.append((x, y, v))
                if v == 2:
                    visible_points.append((x, y))

            # Numerisch instabile Samples verwerfen.
            if any((not is_finite_number(x)) or (not is_finite_number(y)) for x, y, _ in projected):
                skipped_non_finite += 1
                continue

            # Zu wenige sichtbare Punkte liefern schwache/inkonsistente Targets.
            if len(visible_points) < MIN_VISIBLE_KEYPOINTS:
                skipped_not_enough_visible += 1
                continue

            # Nur Samples mit brauchbarer Geometrie verwenden.
            # Wenn zu wenige sichtbare Punkte da sind, wird das Sample verworfen.
            bbox = compute_bbox_from_visible_points(visible_points, pad=BBOX_PADDING)
            if bbox is None:
                skipped_bad_bbox += 1
                continue

            cx, cy, w, h = bbox
            if w < MIN_BBOX_W or h < MIN_BBOX_H:
                skipped_bad_bbox += 1
                continue

            img_id = f"image_{generated:04d}"

            # 4) YOLO-Pose-Label schreiben.
            # Format: class cx cy w h (x y v)*N
            kp_parts = []
            for x, y, v in projected:
                if v == 2:
                    # Kleine Numerikrandfaelle robust begrenzen.
                    x = clamp01(x)
                    y = clamp01(y)
                    kp_parts.append(f"{x:.6f} {y:.6f} 2")
                else:
                    kp_parts.append("0.000000 0.000000 0")

            label_file = os.path.join(pose_labels_dir, f"{img_id}.txt")
            with open(label_file, "w") as f_pose:
                f_pose.write(f"0 {cx:.6f} {cy:.6f} {w:.6f} {h:.6f} " + " ".join(kp_parts) + "\n")

            # 5) RGB rendern (fotorealistischer als Workbench).
            scene.render.engine = "BLENDER_EEVEE"
            rgb_path = os.path.join(rgb_dir, f"{img_id}.png")
            scene.render.filepath = rgb_path
            bpy.ops.render.render(write_still=True)

            # 5b) Post-Render: Gausssches Rauschen auf das RGB-Bild legen.
            apply_post_noise(rgb_path, DOMAIN_RAND)

            # 5c) Exposure fuer Masken-Render zuruecksetzen (Masken brauchen neutrale Farben).
            scene.view_settings.exposure = 0.0

            # 6) Segment-Maske rendern:
            #    Workbench + Object Colors + Flat Lighting fuer klare Klassenfarben.
            scene.render.engine = "BLENDER_WORKBENCH"
            scene.display.shading.light = "FLAT"
            scene.display.shading.color_type = "OBJECT"
            scene.display.render_aa = "OFF"
            scene.render.filepath = os.path.join(mask_dir, f"mask_{generated:04d}.png")
            bpy.ops.render.render(write_still=True)

            # 7) CSV schreiben (Winkel inkl. projektspezifischer Korrekturterme).
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

        print("Pose-Sanity-Filter:")
        print(f"  verworfen (sichtbare KP < {MIN_VISIBLE_KEYPOINTS}): {skipped_not_enough_visible}")
        print(f"  verworfen (BBox ungueltig/zu klein): {skipped_bad_bbox}")
        print(f"  verworfen (nicht-endliche Werte): {skipped_non_finite}")
        print(f"  Versuche gesamt: {attempt}")

    print("Dataset-Generierung abgeschlossen.")
    print(f"RGB:        {rgb_dir}")
    print(f"Masken:     {mask_dir}")
    print(f"PoseLabels: {pose_labels_dir}")
finally:
    # Kamera zuruecksetzen, damit die Szene nach Skriptende unveraendert bleibt.
    cam.location = orig_loc
    cam.rotation_euler = orig_rot

    # Domain Randomization: Lichter, Hintergrund und Belichtung zuruecksetzen.
    restore_lights(scene_lights, lights_original_state)
    restore_world(world_original_state)
    scene.view_settings.exposure = exposure_original

    # Urspruenglichen Render-Status der Helperobjekte wiederherstellen.
    restore_render_visibility(render_helpers, helpers_original_render_state)