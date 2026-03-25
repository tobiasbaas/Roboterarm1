"""
Kalibriert die 3D-Keypoint-Offsets für den synthetischen Roboterarm-Datensatz.

Ablauf:
1) Marker-Objekte (KP_CAL_MARKER_*) im Blender-Viewport auf Referenzpunkte setzen.
2) Dieses Skript ausführen, um lokale Offsets relativ zu Anchor-Objekten zu berechnen.
3) Preview-Bilder und Log-Datei prüfen.
4) Ausgegebenen KEYPOINT_SPECS-Block in das Generierungsskript übernehmen.
"""

import os
import bpy
import traceback
from mathutils import Vector

# ============================================================
# Keypoint-Achsen Kalibrierung fuer Roboterarm-Dataset
#
# Ablauf:
# 1) Pro Keypoint ein Referenz-Empty im Viewport auf die echte Drehachse setzen.
# 2) Dieses Skript in Blender ausfuehren.
# 3) Das Skript berechnet lokale Offsets relativ zum Anchor-Objekt,
#    erstellt Marker und rendert ein Preview-Bild zur Kontrolle.
# 4) Den ausgegebenen KEYPOINT_SPECS-Block in dein Datensatz-Skript uebernehmen.
# ============================================================

CAMERA_NAME = "Camera"
if "__file__" in globals():
    _SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
else:
    _SCRIPT_DIR = os.path.dirname(bpy.data.filepath)

OUTPUT_DIR = os.path.join(_SCRIPT_DIR, "dataset", "keypoint_calibration")
PREVIEW_SCENE_FILE = "keypoint_axis_preview_scene.png"
PREVIEW_MARKERS_ONLY_FILE = "keypoint_axis_preview_markers_only.png"
LOG_FILE = "keypoint_axis_calibration.log"
PREVIEW_RESOLUTION = (1024, 1024)

# Wenn True: fehlende KP_CAL_MARKER_* werden an Anchor-Position erzeugt.
AUTO_CREATE_CALIBRATION_MARKERS = True

# Wenn True: alte Marker-Objekte aufraeumen (kp_marker_*, KP_REF_*).
CLEANUP_LEGACY_MARKERS = True

# Du kannst Namen hier auf dein Blender-Setup anpassen.
KEYPOINT_DEFS = [
    {
        "name": "base",
        "anchor_candidates": ["base_link", "base"],
        "calibration_marker": "KP_CAL_MARKER_base",
        "color": (1.0, 0.0, 0.0, 1.0),
    },
    {
        "name": "base_z1",
        "anchor_candidates": ["base_link", "base"],
        "calibration_marker": "KP_CAL_MARKER_base_z1",
        "color": (1.0, 0.4, 0.4, 1.0),
    },
    {
        "name": "base_z2",
        "anchor_candidates": ["base_link", "base"],
        "calibration_marker": "KP_CAL_MARKER_base_z2",
        "color": (1.0, 0.7, 0.7, 1.0),
    },
    {
        "name": "j1",
        "anchor_candidates": ["joint1", "link1"],
        "calibration_marker": "KP_CAL_MARKER_j1",
        "color": (0.0, 1.0, 0.0, 1.0),
    },
    {
        "name": "j1_z1",
        "anchor_candidates": ["joint1", "link1"],
        "calibration_marker": "KP_CAL_MARKER_j1_z1",
        "color": (0.5, 1.0, 0.5, 1.0),
    },
    {
        "name": "j2",
        "anchor_candidates": ["joint2", "link2"],
        "calibration_marker": "KP_CAL_MARKER_j2",
        "color": (0.0, 0.0, 1.0, 1.0),
    },
    {
        "name": "j3",
        "anchor_candidates": ["joint3", "link3"],
        "calibration_marker": "KP_CAL_MARKER_j3",
        "color": (1.0, 1.0, 0.0, 1.0),
    },
    {
        "name": "j4",
        "anchor_candidates": ["joint4", "link4"],
        "calibration_marker": "KP_CAL_MARKER_j4",
        "color": (1.0, 0.0, 1.0, 1.0),
    },
    {
        "name": "tcp",
        "anchor_candidates": ["tcp_gripper", "tcp", "gripper_tip"],
        "calibration_marker": "KP_CAL_MARKER_tcp",
        "color": (1.0, 1.0, 1.0, 1.0),
    },
]

MARKER_RADIUS = 0.006
MARKER_PREFIX = "KP_CAL_MARKER_"
MARKER_COLLECTION_NAME = "KP_CALIBRATION_MARKERS"


def _show_popup(title, lines, icon='INFO'):
    def draw(self, _context):
        for line in lines:
            self.layout.label(text=line)

    bpy.context.window_manager.popup_menu(draw, title=title, icon=icon)


def _log_path():
    return os.path.join(OUTPUT_DIR, LOG_FILE)


def log(message):
    msg = str(message)
    print(msg)
    try:
        os.makedirs(OUTPUT_DIR, exist_ok=True)
        with open(_log_path(), "a", encoding="utf-8") as f:
            f.write(msg + "\n")
    except Exception:
        # Logging must never break the actual calibration flow.
        pass


def first_existing_object(candidates):
    for name in candidates:
        obj = bpy.data.objects.get(name)
        if obj is not None:
            return obj
    return None


def ensure_output_dir(path):
    os.makedirs(path, exist_ok=True)


def ensure_collection(name):
    coll = bpy.data.collections.get(name)
    if coll is None:
        coll = bpy.data.collections.new(name)
        bpy.context.scene.collection.children.link(coll)
    return coll


def ensure_emission_material(name, rgba):
    mat = bpy.data.materials.get(name)
    if mat is None:
        mat = bpy.data.materials.new(name=name)
        mat.use_nodes = True

    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()

    out = nodes.new(type="ShaderNodeOutputMaterial")
    em = nodes.new(type="ShaderNodeEmission")
    em.inputs["Color"].default_value = rgba
    em.inputs["Strength"].default_value = 3.0
    links.new(em.outputs["Emission"], out.inputs["Surface"])
    return mat


def delete_object_if_exists(name):
    obj = bpy.data.objects.get(name)
    if obj is None:
        return
    bpy.data.objects.remove(obj, do_unlink=True)


def cleanup_legacy_markers(valid_marker_names):
    if not CLEANUP_LEGACY_MARKERS:
        return

    for obj in list(bpy.data.objects):
        name = obj.name
        if name in valid_marker_names:
            continue
        if name.startswith("kp_marker_") or name.startswith("KP_REF_"):
            bpy.data.objects.remove(obj, do_unlink=True)

    old_coll = bpy.data.collections.get("KP_DEBUG_MARKERS")
    if old_coll is not None:
        bpy.data.collections.remove(old_coll)


def compute_local_offset(anchor_obj, reference_obj):
    local = anchor_obj.matrix_world.inverted() @ reference_obj.matrix_world.translation
    return (float(local.x), float(local.y), float(local.z))


def ensure_calibration_marker(marker_name, collection, anchor_obj, color):
    marker = bpy.data.objects.get(marker_name)
    if marker is None:
        if not AUTO_CREATE_CALIBRATION_MARKERS:
            return None
        bpy.ops.mesh.primitive_uv_sphere_add(radius=MARKER_RADIUS, location=(0.0, 0.0, 0.0))
        marker = bpy.context.active_object
        marker.name = marker_name
        marker.location = anchor_obj.matrix_world.translation

    # Keep existing parent/transform untouched; only ensure the marker is present in
    # the calibration collection for easier visibility control.
    if collection not in marker.users_collection:
        collection.objects.link(marker)

    marker.hide_render = False
    marker.hide_viewport = False

    mat = ensure_emission_material(f"{marker_name}_MAT", color)
    marker.data.materials.clear()
    marker.data.materials.append(mat)
    return marker


def render_scene_preview(scene, filepath):
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = PREVIEW_RESOLUTION[0]
    scene.render.resolution_y = PREVIEW_RESOLUTION[1]
    scene.render.filepath = filepath
    bpy.ops.render.render(write_still=True)


def render_markers_only_preview(scene, marker_names, camera_name):
    original_visibility = {}

    for obj in bpy.data.objects:
        original_visibility[obj.name] = obj.hide_render
        keep_visible = (
            obj.name in marker_names
            or obj.name == camera_name
            or obj.type == "LIGHT"
        )
        obj.hide_render = not keep_visible

    try:
        render_scene_preview(scene, os.path.join(OUTPUT_DIR, PREVIEW_MARKERS_ONLY_FILE))
    finally:
        for obj in bpy.data.objects:
            obj.hide_render = original_visibility.get(obj.name, False)


def calibrate_and_preview():
    ensure_output_dir(OUTPUT_DIR)
    with open(_log_path(), "w", encoding="utf-8") as f:
        f.write("=== Keypoint Calibration Log ===\n")

    log("[START] keypoint_axis_calibration.py")
    scene = bpy.context.scene
    cam = bpy.data.objects.get(CAMERA_NAME)
    if cam is None:
        raise RuntimeError(f"Kamera '{CAMERA_NAME}' wurde nicht gefunden.")

    log(f"[INFO] Kamera gefunden: {cam.name}")
    marker_collection = ensure_collection(MARKER_COLLECTION_NAME)
    log(f"[INFO] Marker-Collection: {marker_collection.name}")

    valid_marker_names = [kp["calibration_marker"] for kp in KEYPOINT_DEFS]
    cleanup_legacy_markers(valid_marker_names)

    results = []
    warnings = []
    marker_names = []

    for kp in KEYPOINT_DEFS:
        kp_name = kp["name"]
        log(f"[STEP] Verarbeite Keypoint: {kp_name}")
        anchor = first_existing_object(kp["anchor_candidates"])
        if anchor is None:
            warnings.append(
                f"[WARN] {kp_name}: Kein Anchor gefunden. Kandidaten: {kp['anchor_candidates']}"
            )
            continue
        log(f"[INFO] Anchor gefunden: {anchor.name}")

        marker_name = kp["calibration_marker"]
        marker = ensure_calibration_marker(marker_name, marker_collection, anchor, kp["color"])
        if marker is None:
            warnings.append(
                f"[WARN] {kp_name}: Marker '{marker_name}' fehlt und Auto-Create ist aus."
            )
            continue
        log(f"[INFO] Kalibrierungsmarker: {marker.name}")

        local_offset = compute_local_offset(anchor, marker)
        marker_names.append(marker.name)

        results.append(
            {
                "name": kp_name,
                "anchor": anchor.name,
                "ref": marker.name,
                "local_offset": local_offset,
            }
        )

    log("\n=== Keypoint Calibration Report ===")
    for w in warnings:
        log(w)

    if not results:
        log("[ERROR] Keine Keypoints kalibriert. Bitte Referenz-Empties pruefen.")
        _show_popup(
            "Keypoint Calibration",
            [
                "Keine Keypoints kalibriert.",
                "Pruefe KP_CAL_MARKER_* oder aktiviere Auto-Create.",
                f"Log: {_log_path()}",
            ],
            icon='ERROR',
        )
        return

    log("[STEP] Rendere Szene-Preview...")
    render_scene_preview(scene, os.path.join(OUTPUT_DIR, PREVIEW_SCENE_FILE))
    log("[STEP] Rendere Marker-Only-Preview...")
    render_markers_only_preview(scene, marker_names, CAMERA_NAME)

    log("\nBerechnete lokale Offsets:")
    for r in results:
        ox, oy, oz = r["local_offset"]
        log(f"  {r['name']:<5} anchor={r['anchor']:<12} ref={r['ref']:<12} "
            f"offset=({ox:.6f}, {oy:.6f}, {oz:.6f})")

    log("\nKEYPOINT_SPECS fuer dein Datensatz-Skript:")
    log("KEYPOINT_SPECS = [")
    for r in results:
        ox, oy, oz = r["local_offset"]
        log(
            "    {"
            f"\"name\": \"{r['name']}\", "
            f"\"anchor\": \"{r['anchor']}\", "
            f"\"local_offset\": ({ox:.6f}, {oy:.6f}, {oz:.6f})"
            "},"
        )
    log("]")

    scene_preview_path = os.path.join(OUTPUT_DIR, PREVIEW_SCENE_FILE)
    marker_preview_path = os.path.join(OUTPUT_DIR, PREVIEW_MARKERS_ONLY_FILE)

    log("\nPreviews gerendert nach:")
    log(scene_preview_path)
    log(marker_preview_path)
    log("\nHinweis: Verschiebe nur KP_CAL_MARKER_* und fuehre das Skript erneut aus, "
        "bis die Marker exakt auf den Drehachsen liegen.")

    _show_popup(
        "Keypoint Calibration",
        [
            f"Fertig: {len(results)} Keypoints kalibriert.",
            "Preview-Bilder wurden geschrieben.",
            f"Log: {_log_path()}",
        ],
        icon='INFO',
    )


def main():
    try:
        calibrate_and_preview()
    except Exception as exc:
        tb = traceback.format_exc()
        log("[FATAL] Skriptfehler:")
        log(str(exc))
        log(tb)
        _show_popup(
            "Keypoint Calibration Error",
            [
                "Fehler beim Ausfuehren des Skripts.",
                str(exc),
                f"Log: {_log_path()}",
            ],
            icon='ERROR',
        )
        raise


main()
