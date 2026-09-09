# keypoint_axis_calibration.py — Detaillierte Dokumentation

## 📋 Übersicht

Das Skript **`keypoint_axis_calibration.py`** ist ein **Kalibrierungs-Tool für die Keypoint-Offsets** eines Roboterarms in Blender.

Es hilft Ihnen dabei:
1. **Marker manuell platzieren** an den gewünschten Keypoint-Positionen
2. **Lokale Offsets berechnen** (relativ zu Gelenk-Ankern)
3. **Preview-Bilder rendern** zur visuellen Kontrolle
4. **Kalibrierungswerte exportieren** zur Verwendung im Datengenerierungs-Skript

---

## 🎯 Workflow

```
Roboterarm-Modell in Blender (mit Gelenken/Links)
    ↓
[1] Marker platzieren (KP_CAL_MARKER_*)
    Manuell im Viewport auf die Drehachsen setzen
    ↓
[2] keypoint_axis_calibration.py ausführen
    ├─ Marker → Anchor-Offsets berechnen
    ├─ Preview-Bilder rendern
    └─ Log-Datei mit Offsets schreiben
    ↓
[3] Preview-Bilder überprüfen
    Stimmen die Marker korrekt?
    ↓
[4] Falls nötig: Marker anpassen und [2] wiederholen
    ↓
[5] Offsets ins Datengenerierungs-Skript kopieren
    (KEYPOINT_SPECS in Blender_Segment_Keypoint_Generation.py)
    ↓
Blender_Segment_Keypoint_Generation.py ausführen
```

---

## 🔧 Konfiguration

### Verzeichnisse

```python
CAMERA_NAME = "Camera"  # Name der Kamera in Blender

OUTPUT_DIR = os.path.join(_SCRIPT_DIR, "dataset", "keypoint_calibration")
# Speichert hier:
# - keypoint_axis_calibration.log
# - keypoint_axis_preview_scene.png
# - keypoint_axis_preview_markers_only.png
```

### Auto-Features

```python
AUTO_CREATE_CALIBRATION_MARKERS = True
# Wenn True: fehlende Marker werden automatisch bei Anchor-Position erstellt
# Wenn False: Fehler wenn Marker fehlen

CLEANUP_LEGACY_MARKERS = True
# Löscht alte Marker-Objekte (kp_marker_*, KP_REF_*) aus vorherigen Läufen
```

### Keypoint-Definitionen

```python
KEYPOINT_DEFS = [
    {
        "name": "base",
        "anchor_candidates": ["base_link", "base"],
        "calibration_marker": "KP_CAL_MARKER_base",
        "color": (1.0, 0.0, 0.0, 1.0),  # Rot
    },
    {
        "name": "base_z1",
        "anchor_candidates": ["base_link", "base"],
        "calibration_marker": "KP_CAL_MARKER_base_z1",
        "color": (1.0, 0.4, 0.4, 1.0),  # Helles Rot
    },
    # ... weitere Keypoints
]
```

### Marker-Einstellungen

```python
MARKER_RADIUS = 0.006           # Marker-Größe in Blender-Einheiten
MARKER_PREFIX = "KP_CAL_MARKER_"
MARKER_COLLECTION_NAME = "KP_CALIBRATION_MARKERS"
```

Die Marker sind **leuchtende Kugeln** zur visuellen Kontrolle.

### Render-Settings

```python
PREVIEW_RESOLUTION = (1024, 1024)
PREVIEW_SCENE_FILE = "keypoint_axis_preview_scene.png"
PREVIEW_MARKERS_ONLY_FILE = "keypoint_axis_preview_markers_only.png"
```

---

# 🔍 Funktionen im Detail

## Utility-Funktionen

### `first_existing_object(candidates) -> bpy.types.Object or None`

```python
def first_existing_object(candidates):
    for name in candidates:
        obj = bpy.data.objects.get(name)
        if obj is not None:
            return obj
    return None
```

#### Zweck
Flexible Objekt-Suche mit Fallback-Namen.

#### Beispiel
```python
anchor = first_existing_object(["base_link", "base"])
# Sucht zuerst "base_link", dann "base"
```

---

### `ensure_output_dir(path)`

```python
def ensure_output_dir(path):
    os.makedirs(path, exist_ok=True)
```

Erstellt Output-Verzeichnis falls nicht vorhanden.

---

### `ensure_collection(name) -> bpy.types.Collection`

```python
def ensure_collection(name):
    coll = bpy.data.collections.get(name)
    if coll is None:
        coll = bpy.data.collections.new(name)
        bpy.context.scene.collection.children.link(coll)
    return coll
```

#### Zweck
Erstellt oder findet eine Blender-Collection zum Organisieren der Marker.

**Warum Collection?** Damit können alle Marker zusammen hidden/visible werden.

---

## Material und Rendering

### `ensure_emission_material(name, rgba) -> bpy.types.Material`

```python
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
```

#### Zweck
Erstellt ein **selbstleuchtendes Material** für die Marker.

#### Logik
- **Emission-Shader:** Das Material leuchtet selbst
- **Strength 3.0:** Helle Leuchtkraft (sichtbar auch ohne externe Beleuchtung)
- **RGBA:** Farbe des Materials

#### Beispiel
```python
red_mat = ensure_emission_material("Marker_base_MAT", (1.0, 0.0, 0.0, 1.0))
sphere.data.materials.append(red_mat)
# → Rote selbstleuchtende Kugel
```

---

## Marker-Management

### `delete_object_if_exists(name)`

```python
def delete_object_if_exists(name):
    obj = bpy.data.objects.get(name)
    if obj is None:
        return
    bpy.data.objects.remove(obj, do_unlink=True)
```

Löscht Objekt sauber aus Blender.

---

### `cleanup_legacy_markers(valid_marker_names)`

```python
def cleanup_legacy_markers(valid_marker_names):
    if not CLEANUP_LEGACY_MARKERS:
        return

    for obj in list(bpy.data.objects):
        name = obj.name
        if name in valid_marker_names:
            continue
        if name.startswith("kp_marker_") or name.startswith("KP_REF_"):
            bpy.data.objects.remove(obj, do_unlink=True)
```

Löscht alte Marker aus früheren Läufen, um nicht durcheinander zu kommen.

---

### `ensure_calibration_marker(marker_name, collection, anchor_obj, color)`

```python
def ensure_calibration_marker(marker_name, collection, anchor_obj, color):
    marker = bpy.data.objects.get(marker_name)
    if marker is None:
        if not AUTO_CREATE_CALIBRATION_MARKERS:
            return None
        # Erstelle neue UV-Sphäre
        bpy.ops.mesh.primitive_uv_sphere_add(radius=MARKER_RADIUS, location=(0.0, 0.0, 0.0))
        marker = bpy.context.active_object
        marker.name = marker_name
        marker.location = anchor_obj.matrix_world.translation

    # Zur Collection hinzufügen
    if collection not in marker.users_collection:
        collection.objects.link(marker)

    marker.hide_render = False
    marker.hide_viewport = False

    # Material zuweisen
    mat = ensure_emission_material(f"{marker_name}_MAT", color)
    marker.data.materials.clear()
    marker.data.materials.append(mat)
    return marker
```

#### Zweck
Erstellt oder aktualisiert einen Kalibrierungs-Marker.

#### Logik

1. **Marker suchen oder erstellen**
   - Falls vorhanden: nutzen
   - Falls nicht und `AUTO_CREATE=True`: neue Sphäre erstellen
   - Automatisch bei Anchor-Position platziert

2. **Zur Collection hinzufügen**
   - Organisiert alle Marker zusammen

3. **Sichtbarmachen**
   - `hide_render = False` (rendern)
   - `hide_viewport = False` (im Viewport sehen)

4. **Material zuweisen**
   - Leuchtend mit der angegebenen Farbe

---

## Offset-Berechnung

### `compute_local_offset(anchor_obj, reference_obj)`

```python
def compute_local_offset(anchor_obj, reference_obj):
    local = anchor_obj.matrix_world.inverted() @ reference_obj.matrix_world.translation
    return (float(local.x), float(local.y), float(local.z))
```

#### Zweck
Berechnet den Offset eines Markers relativ zu seinem Anchor-Gelenk.

#### Mathematik

```
Weltposition des Markers
    ↓
Anchor-Koordinatensystem (inverted @ translation)
    ↓
Lokaler Offset (relativ zu Anchor)
```

#### Beispiel
```
Anchor (joint1): Weltposition (0.1, 0.0, 0.05), Rotation 0°
Marker: Weltposition (0.15, 0.0, 0.05)

Offset = (0.15 - 0.1, 0.0 - 0.0, 0.05 - 0.05) = (0.05, 0.0, 0.0)
```

**Wichtig:** Dieser Offset bleibt gültig, auch wenn sich der Anchor dreht!

---

## Preview-Rendering

### `render_scene_preview(scene, filepath)`

```python
def render_scene_preview(scene, filepath):
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = PREVIEW_RESOLUTION[0]
    scene.render.resolution_y = PREVIEW_RESOLUTION[1]
    scene.render.filepath = filepath
    bpy.ops.render.render(write_still=True)
```

#### Zweck
Rendert die komplette Szene mit Roboter + Markern.

**Output:** `keypoint_axis_preview_scene.png`

Zeigt:
- Roboterarm mit realistischer Beleuchtung
- Marker als farbige leuchtende Kugeln
- Helfen zu sehen, ob die Marker korrekt platziert sind

---

### `render_markers_only_preview(scene, marker_names, camera_name)`

```python
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
```

#### Zweck
Rendert **nur die Marker** (ohne Roboter) zur Kontrolle.

**Output:** `keypoint_axis_preview_markers_only.png`

Zeigt:
- Nur die farbigen Marker
- Hilft zu sehen, ob die räumliche Anordnung korrekt ist
- Leichter zu prüfen als die volle Szene

#### Logik
1. Alle Objekte speichern
2. Alles außer Marker/Kamera/Lichter hidden_render = True
3. Rendert
4. Wieder back to original state (finally-Block)

---

## Logging

### `log(message)`

```python
def log(message):
    msg = str(message)
    print(msg)
    try:
        os.makedirs(OUTPUT_DIR, exist_ok=True)
        with open(_log_path(), "a", encoding="utf-8") as f:
            f.write(msg + "\n")
    except Exception:
        pass  # Logging muss nicht brechen
```

#### Zweck
Schreibt Logs in Console **und** in Log-Datei.

**Output:** `dataset/keypoint_calibration/keypoint_axis_calibration.log`

---

# 🎯 Hauptfunktion

### `calibrate_and_preview()`

```python
def calibrate_and_preview():
```

Dies ist die zentrale Funktion, die alles koordiniert.

#### Prozess

**1. Initialisierung**
```python
ensure_output_dir(OUTPUT_DIR)
with open(_log_path(), "w", encoding="utf-8") as f:
    f.write("=== Keypoint Calibration Log ===\n")

scene = bpy.context.scene
cam = bpy.data.objects.get(CAMERA_NAME)
if cam is None:
    raise RuntimeError(f"Kamera '{CAMERA_NAME}' wurde nicht gefunden.")
```

- Output-Verzeichnis erstellen
- Log-Datei leeren
- Kamera finden (essentiell!)

**2. Marker-Collection erstellen**
```python
marker_collection = ensure_collection(MARKER_COLLECTION_NAME)
valid_marker_names = [kp["calibration_marker"] for kp in KEYPOINT_DEFS]
cleanup_legacy_markers(valid_marker_names)
```

- Alle Marker in einer Collection organisieren
- Alte Marker aufräumen

**3. Pro Keypoint: Anchor + Marker auflösen**
```python
for kp in KEYPOINT_DEFS:
    kp_name = kp["name"]
    anchor = first_existing_object(kp["anchor_candidates"])
    if anchor is None:
        warnings.append(f"[WARN] {kp_name}: Kein Anchor gefunden...")
        continue

    marker_name = kp["calibration_marker"]
    marker = ensure_calibration_marker(marker_name, marker_collection, anchor, kp["color"])
    if marker is None:
        warnings.append(f"[WARN] {kp_name}: Marker fehlt...")
        continue

    local_offset = compute_local_offset(anchor, marker)
    results.append({
        "name": kp_name,
        "anchor": anchor.name,
        "ref": marker.name,
        "local_offset": local_offset,
    })
```

Pro Keypoint:
1. Anchor (Gelenk) finden
2. Marker finden oder erstellen
3. Offset berechnen
4. In `results` speichern

**4. Prüfen ob erfolgreich**
```python
if not results:
    log("[ERROR] Keine Keypoints kalibriert...")
    _show_popup(...)
    return
```

Falls kein einziger Keypoint kalibriert wurde, Error.

**5. Preview-Bilder rendern**
```python
log("[STEP] Rendere Szene-Preview...")
render_scene_preview(scene, os.path.join(OUTPUT_DIR, PREVIEW_SCENE_FILE))
log("[STEP] Rendere Marker-Only-Preview...")
render_markers_only_preview(scene, marker_names, CAMERA_NAME)
```

Zwei Preview-Bilder zur visuellen Kontrolle.

**6. Ergebnisse loggen und exportieren**
```python
log("\nBerechnete lokale Offsets:")
for r in results:
    ox, oy, oz = r["local_offset"]
    log(f"  {r['name']:<5} anchor={r['anchor']:<12} ... offset=({ox:.6f}, {oy:.6f}, {oz:.6f})")

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
```

Gibt die berechneten Offsets aus im Format, das direkt in `Blender_Segment_Keypoint_Generation.py` kopiert werden kann!

**7. Info-Popup**
```python
_show_popup(
    "Keypoint Calibration",
    [
        f"Fertig: {len(results)} Keypoints kalibriert.",
        "Preview-Bilder wurden geschrieben.",
        f"Log: {_log_path()}",
    ],
    icon='INFO',
)
```

Bestätigungsdialog mit Zusammenfassung.

---

# 🚀 Praktischer Workflow

## Schritt-für-Schritt Anleitung

### [1] Blender-Datei vorbereiten

```
Roboterarm-Modell in Blender
├── base_link (oder: base)
├── joint1 (Rotation um Z)
│   └── link1
├── joint2 (Rotation um Y)
│   └── link2
├── joint3 (Rotation um Y)
│   └── link3
├── joint4 (Rotation um Y)
│   └── link4 (Gripper/TCP)
└── Camera
```

**Wichtig:** Alle Gelenke und die Kamera müssen existieren!

---

### [2] Marker platzieren (manuell im Viewport)

```
Im Blender-Viewport:

1. Add → Empty → Plain Axes (oder Sphere als Marker)
   Benennen: KP_CAL_MARKER_base

2. Im Outliner zu Viewport-Position dieser Marker setzen:
   - Base-Marker auf base_link Drehachse
   - Joint1-Marker auf joint1 Rotationspunkt
   - Joint2-Marker auf joint2 Rotationspunkt
   - usw.

3. Speichern (Ctrl+S)
```

**Visuelle Kontrolle:** Wenn ein Gelenk sich dreht, sollte nur der korrespondierende Marker sich bewegen, nicht die anderen.

---

### [3] Skript ausführen

```
Blender:
1. Scripting Tab öffnen
2. Dieses Skript laden (Open → keypoint_axis_calibration.py)
3. Alt+P oder Button "Run Script"
```

---

### [4] Konsole und Popup überprüfen

```
Console Output:

[START] keypoint_axis_calibration.py
[INFO] Kamera gefunden: Camera
[INFO] Marker-Collection: KP_CALIBRATION_MARKERS
[STEP] Verarbeite Keypoint: base
[INFO] Anchor gefunden: base_link
[INFO] Kalibrierungsmarker: KP_CAL_MARKER_base
...
[STEP] Rendere Szene-Preview...
[STEP] Rendere Marker-Only-Preview...

Berechnete lokale Offsets:
  base  anchor=base_link ref=KP_CAL_MARKER_base offset=(0.000000, 0.000000, 0.050000)
  j1    anchor=joint1    ref=KP_CAL_MARKER_j1   offset=(0.000000, 0.000000, 0.000000)
  ...

KEYPOINT_SPECS für dein Datensatz-Skript:
KEYPOINT_SPECS = [
    {"name": "base", "anchor": "base_link", "local_offset": (0.000000, 0.000000, 0.050000)},
    {"name": "j1", "anchor": "joint1", "local_offset": (0.000000, 0.000000, 0.000000)},
    ...
]
```

---

### [5] Preview-Bilder überprüfen

```
Öffne im Datei-Explorer:
dataset/keypoint_calibration/

keypoint_axis_preview_scene.png
  → Zeigt Roboter + farbige Marker-Kugeln
  → Stimmen die Positionen?

keypoint_axis_preview_markers_only.png
  → Zeigt nur die Marker
  → Sind sie räumlich sinnvoll angeordnet?
```

---

### [6] Falls nötig: Anpassen und wiederholen

Falls die Marker nicht korrekt sind:

```
1. Zurück zu Blender
2. Marker im Viewport verschieben (G-Taste)
3. Speichern
4. Skript nochmal ausführen (Alt+P)
5. Neue Preview-Bilder überprüfen
→ Wiederholen bis OK
```

---

### [7] Offsets ins Datengenerierungs-Skript kopieren

Aus der Log-Datei:

```
dataset/keypoint_calibration/keypoint_axis_calibration.log

KEYPOINT_SPECS = [
    {"name": "base", "anchor": "base_link", "local_offset": (0.000000, 0.000000, 0.050000)},
    {"name": "base_z1", "anchor": "base_link", "local_offset": (0.050000, 0.000000, 0.000000)},
    ...
]
```

Diese ganze Liste kopieren und in `Blender_Segment_Keypoint_Generation.py` im Abschnitt `KEYPOINT_SPECS =` einfügen.

---

### [8] Datengenerierung starten

```
Blender: Alt+P für Blender_Segment_Keypoint_Generation.py
→ Generiert Datensatz mit kalibrierten Keypoints!
```

---

# 📊 Output-Struktur

```
dataset/keypoint_calibration/
├── keypoint_axis_calibration.log              (Text-Log mit Offsets)
├── keypoint_axis_preview_scene.png            (Szene + Marker)
└── keypoint_axis_preview_markers_only.png     (Nur Marker)
```

### Log-Datei-Beispiel

```
=== Keypoint Calibration Log ===
[START] keypoint_axis_calibration.py
[INFO] Kamera gefunden: Camera
[INFO] Marker-Collection: KP_CALIBRATION_MARKERS
[STEP] Verarbeite Keypoint: base
[INFO] Anchor gefunden: base_link
[INFO] Kalibrierungsmarker: KP_CAL_MARKER_base
[STEP] Verarbeite Keypoint: base_z1
...

Berechnete lokale Offsets:
  base  anchor=base_link offset=(0.000000, 0.000000, 0.050000)
  base_z1 anchor=base_link offset=(0.050000, 0.000000, 0.000000)
  base_z2 anchor=base_link offset=(-0.050000, 0.000000, 0.000000)
  j1    anchor=joint1    offset=(0.000000, 0.000000, 0.000000)
  ...

KEYPOINT_SPECS für dein Datensatz-Skript:
KEYPOINT_SPECS = [
    {"name": "base", "anchor": "base_link", "local_offset": (0.000000, 0.000000, 0.050000)},
    {"name": "base_z1", "anchor": "base_link", "local_offset": (0.050000, 0.000000, 0.000000)},
    ...
]
```

---

# ⚠️ Häufige Fehler & Lösungen

| Fehler | Ursache | Lösung |
|--------|--------|--------|
| `RuntimeError: Kamera 'Camera' wurde nicht gefunden` | Keine Kamera in Blender | Kamera hinzufügen: Add → Camera |
| `[WARN] base: Kein Anchor gefunden` | "base_link" oder "base" existiert nicht | Objekte in Blender umbenennen oder `KEYPOINT_DEFS` anpassen |
| `[WARN] base: Marker 'KP_CAL_MARKER_base' fehlt` | Marker nicht gesetzt | Marker erstellen oder `AUTO_CREATE=True` setzen |
| Marker sehr weit weg von Roboter | Einheiten-Mismatch (cm vs. m) | Blender-Einheiten überprüfen |
| Offsets ändern sich bei jedem Lauf | Marker werden bewegt | Blender speichern vor Skriptstart |
| Preview-Bilder sind schwarz | Lichter in Szene fehlen | Add → Light (Sun/Area) |

---

# ✅ Checkliste vor Kalibrierung

- [ ] Blender-Datei mit Roboterarm geladen
- [ ] Kamera hinzugefügt und benannt ("Camera")
- [ ] Alle Gelenke existieren (joint1-4) oder `KEYPOINT_DEFS` angepasst
- [ ] Marker erstellt oder `AUTO_CREATE_CALIBRATION_MARKERS = True`
- [ ] Falls manuell: Marker auf Drehachsen platziert
- [ ] Blender-Datei gespeichert
- [ ] Skript ausgeführt: **Alt+P**
- [ ] Console überprüft (Fehlermeldungen?)
- [ ] Preview-Bilder überprüft (Positionen korrekt?)
- [ ] Log-Datei kopiert in `Blender_Segment_Keypoint_Generation.py`
- [ ] Datengenerierungs-Skript mit neuen Offsets gestartet

---

# 💡 Best Practices

### Marker-Platzierung
- **Genau!** 1mm Fehler → Training leidet
- **Visualisieren:** Render Preview-Bilder zur Kontrolle
- **Iterativ:** Mehrmal anpassen bis perfekt

### AUTO_CREATE vs. manuell
- **AUTO_CREATE=True:** Schneller, Marker an Anchor-Position
- **Manuell:** Präziser, Marker genau auf Drehachsen

### Blender-Einheiten
- Konsistent halten! (alle cm oder alle m)
- Offsets sollten < 1.0 sein (sonst Einheiten-Problem)

### Debugging
- Log-Datei IMMER überprüfen
- Preview-Bilder zeigen schnell, ob Marker korrekt sind
- Marker-Farben sollten in Preview sichtbar sein (sonst Renderprobleme)

---

# 🔗 Integration mit Datengenerierung

Nach erfolgreicher Kalibrierung:

```python
# In Blender_Segment_Keypoint_Generation.py:

KEYPOINT_SPECS = [
    # Diese Liste direkt aus dem Log kopieren!
    {"name": "base", "anchor": "base_link", "local_offset": (0.000000, 0.000000, 0.050000)},
    {"name": "base_z1", "anchor": "base_link", "local_offset": (0.050000, 0.000000, 0.000000)},
    # ... alle 9 Keypoints
]
```

Dann läuft die Datengenerierung mit präzisen Keypoint-Positionen!

---

**Dokumentation erstellt:** 2026-04-17  
**Skript-Version:** basierend auf keypoint_axis_calibration.py
