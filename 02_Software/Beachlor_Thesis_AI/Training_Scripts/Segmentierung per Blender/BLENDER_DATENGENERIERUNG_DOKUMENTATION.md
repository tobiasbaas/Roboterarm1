# Blender_Segment_Keypoint_Generation.py — Detaillierte Dokumentation

## 📋 Übersicht

Das Skript **`Blender_Segment_Keypoint_Generation.py`** ist das **Herzstück der synthetischen Datengenerierung**. Es erzeugt in Blender automatisch ein vollständiges Trainingsdataset mit:

1. **RGB-Bilder** (fotorealistisch mit EEVEE-Render)
2. **Segmentierungsmasken** (farbcodiert für Klassen-Labels)
3. **YOLO-Pose-Labels** (9 Keypoints mit Visibility-Flags)
4. **CSV-Datei** (Gelenkwinkel als Ground-Truth)

Das Skript läuft **völlig automatisiert** — Sie geben die Anzahl Bilder an, es randomiisert die Roboter-Pose, rendert alles und speichert strukturiert ab.

---

## 🎯 Workflow

```
Blender-Szene mit Roboterarm
    ↓
Blender_Segment_Keypoint_Generation.py (dieses Skript)
    ↓
Pro Sample:
    1) Zufaellige Roboter-Pose
    2) Domain Randomization (Kamera, Licht, Hintergrund, Belichtung)
    3) RGB-Render (EEVEE) + Post-Render-Noise (numpy)
    4) Maske-Render (Workbench, farbcodiert, ohne Augmentierung)
    5) YOLO-Pose-Label (9 Keypoints)
    6) CSV-Eintrag (Gelenkwinkel)
    ↓
dataset/
├── images/          (RGB-Bilder mit Domain Randomization + Noise)
├── masks/           (Masken: saubere Farben, ohne Augmentierung)
├── labels_pose/     (Pose-Labels: image_0000.txt, image_0001.txt, ...)
└── joint_data.csv   (Gelenkwinkel: Spalten: image, angle_j1-4)
    ↓
Umrisse_in_Polygone.py [1]
    (konvertiert in YOLO-Seg-Labels + organisiert Pose)
```

---

## 🔧 Konfiguration und Konstanten

### Basis-Verzeichnisse

```python
base_dir = _resolve_base_dir()
output_dir = os.path.join(base_dir, "dataset")

rgb_dir = os.path.join(output_dir, "images")        # RGB-Bilder
mask_dir = os.path.join(output_dir, "masks")        # Segmentmasken
pose_labels_dir = os.path.join(output_dir, "labels_pose")  # Pose-Labels

csv_path = os.path.join(output_dir, "joint_data.csv")
```

**Automatische Pfad-Erkennung:**
1. Blender-Dateipfad (`//` in Blender)
2. Python-Skript-Verzeichnis
3. Aktuelles Arbeitsverzeichnis

### Datengenerierungs-Parameter

```python
num_images = 2000                    # Anzahl zu generierender Bilder
rotation_jitter = 0.5               # Legacy, wird durch DOMAIN_RAND ersetzt
```

### Domain Randomization (DOMAIN_RAND)

Um den **Domain Gap** zwischen synthetischen Blender-Bildern und echten Kamerabildern zu reduzieren, werden pro Sample zufaellige Variationen auf Kamera, Beleuchtung, Hintergrund, Belichtung und Bildrauschen angewendet.

```python
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
```

**Parameter im Detail:**

| Parameter | Bereich | Wirkung |
|-----------|---------|---------|
| `camera_loc_jitter` | 0.03m | Kamera wird pro Achse bis zu 3cm verschoben |
| `camera_rot_jitter` | 2.0° | Kamera wird pro Achse bis zu 2° gedreht |
| `light_energy_range` | 0.4x - 2.5x | Lichtintensitaet wird zufaellig skaliert |
| `light_color_jitter` | ±0.15 | Lichtfarbe wird pro RGB-Kanal verschoben |
| `world_color_random` | True/False | Hintergrundfarbe zufaellig variieren |
| `world_color_range` | 0.01 - 0.25 | Helligkeitsbereich der Hintergrundfarbe |
| `exposure_range` | -2.0 bis +1.5 | Belichtung (dunkel bis ueberbelichtet) |
| `noise_strength_range` | 0.0 - 0.12 | Gausssches Rauschen (Standardabweichung) |

**Rausch-Intensitaeten:**
- **0.00 - 0.02**: kein bis kaum sichtbares Rauschen
- **0.03 - 0.05**: leichtes, realistisches Kamerarauschen
- **0.06 - 0.08**: deutlich verrauschtes Bild
- **0.09 - 0.12**: stark verrauscht, Roboter teilweise im Noise versteckt

**Belichtungs-Wirkung:**
- **-2.0**: Sehr dunkel, Roboter kaum erkennbar (4x dunkler)
- **0.0**: Neutral (Originalbelichtung)
- **+1.5**: Stark ueberbelichtet (ca. 3x heller)

### Gelenkkonfiguration

```python
joints_config = {
    "joint1": {"axis": 2, "min": -95.0,  "max": 95.0},   # Z-Rotation
    "joint2": {"axis": 1, "min": 41.0,   "max": -55.0},  # Y-Rotation
    "joint3": {"axis": 1, "min": -80.0,  "max": 80.0},   # Y-Rotation
    "joint4": {"axis": 1, "min": 70.0,   "max": -110.0}, # Y-Rotation
}
```

**Bedeutung:**
- `axis`: Blender-Rotationsachse (0=X, 1=Y, 2=Z)
- `min`, `max`: Gelenkgrenzen in Grad
- Bei jedem Sample wird ein zufälliger Winkel zwischen min/max gewählt

### Farb-Karte für Segmentierung (RGBA)

```python
COLOR_MAP = {
    "base": (1.0, 0.0, 0.0, 1.0),    # ROT
    "j1":   (0.0, 1.0, 0.0, 1.0),    # GRÜN
    "j2":   (0.0, 0.0, 1.0, 1.0),    # BLAU
    "j3":   (1.0, 1.0, 0.0, 1.0),    # GELB
    "j4":   (1.0, 0.0, 1.0, 1.0),    # MAGENTA
    "tcp":  (1.0, 1.0, 1.0, 1.0),    # WEIß
}
```

Diese Farben werden in die Objektfarben geschrieben und dann beim Masken-Render verwendet.

### Keypoint-Spezifikationen

```python
KEYPOINT_SPECS = [
    {
        "name": "base",                              # Name
        "anchor_candidates": ["base_link", "base"],  # Objekte zum Suchen
        "reference_candidates": ["KP_CAL_MARKER_base", "base_link", "base"],
        "local_offset": None,  # Wird aus Kalibrierung berechnet
    },
    # ... 8 weitere Keypoints (base_z1, base_z2, j1, j1_z1, j2, j3, j4, tcp)
]
```

**Konzept:** 
- **Anchor:** Das Gelenk/Objekt, zu dem der Offset relativ ist
- **Reference:** Marker oder Objekt zur Kalibrierung
- **local_offset:** Wird berechnet als `anchor.inverted() @ reference.location`

Falls `local_offset` direkt angegeben wird, wird die Referenz nicht benötigt.

### Sanity-Check Parameter

```python
MIN_VISIBLE_KEYPOINTS = 6      # Mindestens 6 Keypoints müssen sichtbar sein
MIN_BBOX_W = 0.05              # Min. BBox-Breite (5% des Bildes)
MIN_BBOX_H = 0.05              # Min. BBox-Höhe (5% des Bildes)

BBOX_PADDING = 0.10            # 10% Padding um sichtbare Keypoints
```

Diese Parameter filtern degenerierte Samples (zu kleine Roboter, zu wenige sichtbare Joints).

### Render-Settings

```python
RENDER_HELPER_PREFIXES = ("KP_CAL_MARKER_", "kp_marker_", "KP_REF_")

scene.render.resolution_x = 1024
scene.render.resolution_y = 1024
```

Hilfsobjekte (Kalibrierungs-Marker) werden **nicht gerendert**, damit sie keine Artefakte verursachen.

### Initialisierung (Domain Randomization)

Vor der Hauptschleife werden die Originalzustaende aller variierbaren Elemente gesichert:

```python
scene_lights = collect_scene_lights()
lights_original_state = snapshot_lights(scene_lights)
world_original_state = snapshot_world()
exposure_original = scene.view_settings.exposure
```

Im `finally`-Block werden alle Zustaende zurueckgesetzt:

```python
finally:
    cam.location = orig_loc
    cam.rotation_euler = orig_rot
    restore_lights(scene_lights, lights_original_state)
    restore_world(world_original_state)
    scene.view_settings.exposure = exposure_original
    restore_render_visibility(render_helpers, helpers_original_render_state)
```

So bleibt die Blender-Szene nach Skriptende unveraendert.

---

# 🔍 Funktionen im Detail

## Pfad-Auflösung

### `_resolve_base_dir() -> str`

```python
def _resolve_base_dir():
    """Ermittelt robust das Basisverzeichnis fuer Output-Dateien."""
```

#### Zweck
Findet den korrekten Ausgabe-Pfad automatisch, unabhängig davon, wie das Skript gestartet wird.

#### Logik (Priorität)

1. **Blender-Dateipfad** (bevorzugt)
   ```python
   if bpy.data.filepath:
       blend_base = bpy.path.abspath("//")  # "//" = Ordner der .blend-Datei
   ```
   
   **Beispiel:**
   ```
   Blender-Datei: C:/Projects/roboterarm.blend
   base_dir = C:/Projects/
   ```

2. **Python-Skript-Verzeichnis** (Fallback)
   ```python
   script_file = globals().get("__file__")
   return os.path.dirname(os.path.abspath(script_file))
   ```

3. **Aktuelles Arbeitsverzeichnis** (letzter Fallback)
   ```python
   return os.path.abspath(os.getcwd())
   ```

**Warum diese Reihenfolge?** 
- Blender-Pfad ist am robustesten (relativ zur Projekt-Datei)
- Skript-Pfad funktioniert auch wenn Skript verschoben wird
- CWD ist am wenigsten zuverlässig

---

## Objekt-Verwaltung

### `find_first_existing(candidates) -> bpy.types.Object or None`

```python
def find_first_existing(candidates):
    """Liefert das erste in Blender vorhandene Objekt aus einer Kandidatenliste."""
    for n in candidates:
        obj = bpy.data.objects.get(n)
        if obj is not None:
            return obj
    return None
```

#### Zweck
Flexible Objekt-Suche — wenn die Blender-Datei unterschiedliche Naming-Conventions hat.

#### Beispiel
```python
anchor = find_first_existing(["joint1", "link1"])
# Sucht zuerst "joint1", dann "link1"
# Gibt das erste gefundene zurück
```

---

### `collect_render_helper_objects() -> list[bpy.types.Object]`

```python
def collect_render_helper_objects():
    """Sammelt Hilfsobjekte (Marker/Referenzen), die nicht ins Render sollen."""
    helpers = []
    for obj in bpy.data.objects:
        if obj.name.startswith(RENDER_HELPER_PREFIXES):
            helpers.append(obj)
    return helpers
```

#### Zweck
Findet alle Kalibrierungs-Marker (`KP_CAL_MARKER_*`, `kp_marker_*`, `KP_REF_*`) für Visibility-Control.

#### Warum nötig?
Diese Objekte sollen **nicht gerendert werden**, da sie nur zur Kalibrierung dienen und Artefakte verursachen würden.

---

### `set_helpers_render_visibility(helpers, visible)`

```python
def set_helpers_render_visibility(helpers, visible):
    for obj in helpers:
        obj.hide_render = not visible
```

**Vor Datengenerierung:** `hide_render = True` (nicht sichtbar)  
**Nach Datengenerierung:** `hide_render = False` (Szene-Zustand wiederherstellen)

---

## Geometrie und Keypoints

### `compute_local_offset(anchor_obj, reference_obj) -> tuple[float, float, float]`

```python
def compute_local_offset(anchor_obj, reference_obj):
    """Berechnet den lokalen Offset von reference relativ zum Anchor."""
    local = anchor_obj.matrix_world.inverted() @ reference_obj.matrix_world.translation
    return (float(local.x), float(local.y), float(local.z))
```

#### Mathematik

```
World-Position des Referenz-Objekts
    ↓
Transformiert in Anchor-Lokalkoordinaten (inverted @ translation)
    ↓
Ergebnis: (ox, oy, oz) im Anchor-Koordinatensystem
```

#### Beispiel
```
Anchor (joint1): Position (0.1, 0.0, 0.05) in Welt
Reference-Marker: Position (0.15, 0.0, 0.05) in Welt

anchor.inverted() @ reference.translation
= Offset in Anchor-Koordinaten (0.05, 0.0, 0.0)
```

**Wichtig:** Dieser Offset bleibt konstant, auch wenn sich der Anchor dreht!

---

### `world_from_anchor_offset(anchor_obj, local_offset) -> Vector`

```python
def world_from_anchor_offset(anchor_obj, local_offset):
    """Transformiert einen lokalen Anchor-Offset in eine Weltposition."""
    local_vec = Vector(local_offset)
    return anchor_obj.matrix_world @ local_vec
```

#### Umkehrung der vorherigen Funktion

```
Lokaler Offset (relativ zu Anchor)
    ↓
Transformiert mit Anchor-Matrix (@ ist Matrizenmultiplikation)
    ↓
Ergebnis: Weltposition
```

#### Beispiel
```
Anchor hat rotiert (matrix_world geändert)
local_offset bleibt gleich (0.05, 0.0, 0.0)
world_from_anchor_offset() berechnet neue Weltposition
→ Der Keypoint "folgt" dem rotierenden Gelenk!
```

---

### `project_world_point(scene, camera, world_pos) -> tuple[float, float, int]`

```python
def project_world_point(scene, camera, world_pos):
    """Projiziert eine 3D-Weltposition auf die 2D-Bildebene."""
    p = world_to_camera_view(scene, camera, world_pos)
    x = float(p.x)
    y = 1.0 - float(p.y)  # Y umkehren (Blender: oben=1, unten=0)
    z = float(p.z)

    in_front = z > 0.0                              # Vor der Kamera?
    in_frame = (0.0 <= x <= 1.0) and (0.0 <= y <= 1.0)  # Im Bild?
    v = 2 if (in_front and in_frame) else 0        # Visibility

    return x, y, v
```

#### Zweck
Konvertiert 3D-Punkt zu normalisierten 2D-Bild-Koordinaten [0, 1].

#### Logik

1. **world_to_camera_view()** (Blender-Funktion)
   - Projektive Transformation
   - Gibt (x, y, z) in Kamera-Raum zurück

2. **Y-Umkehrung**
   ```python
   y = 1.0 - float(p.y)
   # Blender: oben=1, unten=0
   # YOLO: oben=0, unten=1
   ```

3. **Visibility-Check**
   ```python
   v = 2 if (z > 0 and 0 ≤ x ≤ 1 and 0 ≤ y ≤ 1) else 0
   # 2 = sichtbar (im Bild, vor Kamera)
   # 0 = nicht sichtbar (hinter Kamera oder außerhalb)
   ```

#### Rückgabe
```python
(x, y, visibility)
# x, y ∈ [0.0, 1.0]
# visibility ∈ {0, 2}  (0=unsichtbar, 2=sichtbar)
```

---

### `compute_bbox_from_visible_points(points_xy, pad=BBOX_PADDING) -> tuple or None`

```python
def compute_bbox_from_visible_points(points_xy, pad=BBOX_PADDING):
    """Berechnet BBox um sichtbare Keypoints mit Padding."""
```

#### Prozess

1. **Min/Max finden**
   ```python
   xs = [p[0] for p in points_xy]
   ys = [p[1] for p in points_xy]
   x_min, x_max = min(xs), max(xs)
   y_min, y_max = min(ys), max(ys)
   ```

2. **Padding hinzufügen**
   ```python
   w = x_max - x_min
   h = y_max - y_min
   
   x_min -= w * pad    # 10% Padding nach links
   x_max += w * pad    # 10% Padding nach rechts
   y_min -= h * pad    # 10% Padding nach oben
   y_max += h * pad    # 10% Padding nach unten
   ```

3. **Ins Bild clampen**
   ```python
   x_min = clamp01(x_min)  # Mindestens 0.0
   x_max = clamp01(x_max)  # Höchstens 1.0
   # usw.
   ```

4. **Zu Mittelpunkt + Größe konvertieren**
   ```python
   cx = (x_min + x_max) / 2.0
   cy = (y_min + y_max) / 2.0
   w2 = x_max - x_min
   h2 = y_max - y_min
   
   return cx, cy, w2, h2  # YOLO-Format
   ```

#### Rückgabe
```python
(center_x, center_y, width, height)  # alle normalisiert [0, 1]
# oder None falls BBox zu klein oder degeneriert
```

---

## Render-Setup

### `set_segment_colors()`

```python
def set_segment_colors():
    """Färbt Meshes anhand ihrer Namen/Hierarchie für Segment-Masken."""
    for obj in bpy.data.objects:
        if obj.type != "MESH":
            continue
        current = obj
        col = (0.5, 0.5, 0.5, 1.0)  # Grau-Default
        while current is not None:
            n = current.name.lower()
            if "base" in n:
                col = COLOR_MAP["base"]  # Rot
                break
            elif "link1" in n or "joint1" in n:
                col = COLOR_MAP["j1"]    # Grün
                # usw.
            current = current.parent
        obj.color = col
```

#### Zweck
Setzt Objektfarben basierend auf hierarchischem Naming.

#### Logik
- Durchlaufe Objekthierarchie von Mesh zum Root
- Suche Keywords in Namen ("base", "joint1", etc.)
- Erste Übereinstimmung bestimmt die Farbe

#### Warum hierarchisch?
```
gripper_finger_tip (Mesh)
  ↑ parent
gripper (Mesh)
  ↑ parent
link4 (Mesh)  ← "link4" im Namen → Farbe = COLOR_MAP["j4"]
  ↑ parent
joint4 (Armature)
```

Damit auch tief verschachtelte Meshes die richtige Farbe bekommen.

---

## Domain Randomization Funktionen

### `collect_scene_lights() -> list`

```python
def collect_scene_lights():
    """Sammelt alle Lichter in der Szene."""
    return [obj for obj in bpy.data.objects if obj.type == 'LIGHT']
```

Findet alle Licht-Objekte in der Blender-Szene, deren Intensitaet und Farbe randomisiert werden.

---

### `snapshot_lights(lights) -> dict` / `restore_lights(lights, originals)`

Speichert den Originalzustand aller Lichter (Energie + Farbe) vor der Datengenerierung und stellt ihn danach wieder her.

```python
# Speichern: {'LichtName': {'energy': 100.0, 'color': (1.0, 1.0, 1.0)}}
lights_original_state = snapshot_lights(scene_lights)

# Wiederherstellen nach Generierung
restore_lights(scene_lights, lights_original_state)
```

---

### `snapshot_world() -> dict` / `restore_world(snap)`

Speichert/wiederherstellt den Hintergrund-Zustand der Blender-Welt. Unterstuetzt sowohl Node-basierte Welten (`Background`-Node) als auch einfache `world.color`.

---

### `randomize_camera(cam_obj, orig_l, orig_r, cfg)`

```python
def randomize_camera(cam_obj, orig_l, orig_r, cfg):
    """Kamera-Position und Rotation zufaellig variieren."""
```

Variiert **Position** (±`camera_loc_jitter` Meter pro Achse) und **Rotation** (±`camera_rot_jitter` Grad pro Achse) der Kamera relativ zur Originalposition.

**Unterschied zum alten `rotation_jitter`:** Frueher wurde nur die Rotation um ±0.5° variiert, jetzt auch die Position (±3cm) und staerkere Rotation (±2°).

---

### `randomize_lighting(lights, originals, cfg)`

```python
def randomize_lighting(lights, originals, cfg):
    """Lichtintensitaet und -farbe zufaellig variieren."""
```

Pro Licht in der Szene:
1. **Energie**: Originalwert × zufaelliger Faktor aus `light_energy_range`
2. **Farbe**: Jeder RGB-Kanal wird um ±`light_color_jitter` verschoben

**Beispiel:** Bei Originalenergie 100.0 und `light_energy_range = (0.4, 2.5)` ergibt sich ein Bereich von 40.0 bis 250.0.

---

### `randomize_world_background(cfg)`

```python
def randomize_world_background(cfg):
    """Hintergrundfarbe der Welt zufaellig variieren (nur fuer EEVEE-Render)."""
```

Setzt die World-Hintergrundfarbe auf einen zufaelligen dunklen Wert. Jeder RGB-Kanal wird unabhaengig aus `world_color_range` gewaehlt.

---

### `randomize_exposure(scn, cfg)`

```python
def randomize_exposure(scn, cfg):
    """Belichtung (Helligkeit) zufaellig variieren."""
```

Setzt `scene.view_settings.exposure` auf einen zufaelligen Wert aus `exposure_range`. Negative Werte erzeugen dunkle Bilder (Roboter schwer erkennbar), positive Werte ueberbelichtete.

**Wichtig:** Die Exposure wird vor dem Masken-Render auf 0 zurueckgesetzt, damit die Segmentierungsfarben sauber bleiben.

---

### `apply_post_noise(filepath, cfg)`

```python
def apply_post_noise(filepath, cfg):
    """Legt Gausssches Rauschen auf ein gerendertes PNG-Bild."""
```

Wird **nach** dem RGB-Render aufgerufen:
1. Laedt das gerenderte PNG via `bpy.data.images.load()`
2. Waehlt eine zufaellige Noise-Staerke aus `noise_strength_range`
3. Addiert Gausssches Rauschen (numpy) auf die RGB-Kanaele
4. Speichert das verrauschte Bild zurueck

**Nicht angewendet auf:** Segmentierungsmasken (diese brauchen exakte Farben).

**Abhaengigkeit:** `import numpy as np` (in Blender 2.8+ enthalten).

---

## Keypoint-Auflösung

### `resolve_keypoints(specs) -> tuple[list, list]`

```python
def resolve_keypoints(specs):
    """Löst Keypoint-Spezifikationen in konkrete, nutzbare Einträge auf."""
```

#### Prozess (pro Keypoint)

1. **Anchor-Objekt finden**
   ```python
   anchor = find_first_existing(spec["anchor_candidates"])
   if anchor is None:
       errors.append(f"{name}: Anchor fehlt...")
       continue
   ```

2. **Local-Offset bestimmen**
   ```python
   local_offset = spec.get("local_offset")
   if local_offset is None:
       # Offset aus Kalibrierungs-Marker berechnen
       ref = find_first_existing(spec.get("reference_candidates", []))
       local_offset = compute_local_offset(anchor, ref)
   ```

3. **Speichern**
   ```python
   resolved.append({
       "name": name,
       "anchor": anchor,
       "local_offset": tuple(local_offset),
   })
   ```

#### Rückgabe
```python
resolved = [
    {"name": "base", "anchor": <Object>, "local_offset": (0.0, 0.0, 0.05)},
    {"name": "base_z1", "anchor": <Object>, "local_offset": (0.05, 0.0, 0.0)},
    # ... 7 weitere
]
errors = []  # Falls Fehler, werden sie hier gesammelt
```

---

# 📊 Hauptschleife: Datengenerierung

## `main()` — Die Generierungsschleife

```python
while generated < num_images and attempt < max_attempts:
    attempt += 1
```

Die Hauptschleife iteriert bis `num_images` erfolgreiche Samples generiert sind.

### Schritt 1: Roboter bewegen (zufällige Pose)

```python
blender_angles = {}
for j_name, cfg in joints_config.items():
    joint = bpy.data.objects.get(j_name)
    val = random.uniform(cfg["min"], cfg["max"])
    joint.rotation_euler[cfg["axis"]] = math.radians(val)
    blender_angles[j_name] = val
```

**Pro Gelenk:**
1. Zufälliger Winkel zwischen min/max
2. In Radiant konvertieren
3. Auf korrekter Achse setzen

**Beispiel:**
```python
# joint1: axis=2 (Z-Rotation), range=[-95, 95]
val = random.uniform(-95.0, 95.0)      # z.B. 45.2°
joint.rotation_euler[2] = math.radians(45.2)  # zu Radiant
```

---

### Schritt 2: Domain Randomization

```python
# Kamera: Position + Rotation variieren
randomize_camera(cam, orig_loc, orig_rot, DOMAIN_RAND)

# Lichter: Intensitaet + Farbe variieren
randomize_lighting(scene_lights, lights_original_state, DOMAIN_RAND)

# Hintergrund: Zufaellige Farbe
randomize_world_background(DOMAIN_RAND)

# Belichtung: Dunkel bis ueberbelichtet
randomize_exposure(scene, DOMAIN_RAND)
```

**Zweck:** Reduziert den Domain Gap zwischen synthetischen und echten Bildern, indem jedes Sample unter anderen Bedingungen gerendert wird.

**Was wird variiert:**
- **Kamera:** Position ±3cm, Rotation ±2° (frueher nur ±0.5° Rotation)
- **Lichter:** Intensitaet 40%-250%, Farbverschiebung ±0.15
- **Hintergrund:** Zufaellige dunkle Farben
- **Belichtung:** -2.0 (sehr dunkel) bis +1.5 (ueberbelichtet)

---

### Schritt 3: Dependency-Graph Update

```python
bpy.context.view_layer.update()
```

**Kritisch!** Aktualisiert alle `matrix_world`-Transformationen, sonst würden die Keypoints alte Positionen haben.

---

### Schritt 4: Keypoints projizieren

```python
projected = []
visible_points = []
for item in kp_items:
    world_pos = world_from_anchor_offset(item["anchor"], item["local_offset"])
    x, y, v = project_world_point(scene, cam, world_pos)
    projected.append((x, y, v))
    if v == 2:
        visible_points.append((x, y))
```

**Pro Keypoint:**
1. Weltposition aus Anchor + Offset berechnen
2. Auf Bildebene projizieren
3. Sichtbar? → in `visible_points` sammeln

---

### Schritt 5: Sanity-Checks

```python
# Check 1: Numerische Stabilität
if any((not is_finite_number(x)) or (not is_finite_number(y)) for x, y, _ in projected):
    skipped_non_finite += 1
    continue

# Check 2: Genügend sichtbare Keypoints?
if len(visible_points) < MIN_VISIBLE_KEYPOINTS:
    skipped_not_enough_visible += 1
    continue

# Check 3: Brauchbare BBox?
bbox = compute_bbox_from_visible_points(visible_points, pad=BBOX_PADDING)
if bbox is None or w < MIN_BBOX_W or h < MIN_BBOX_H:
    skipped_bad_bbox += 1
    continue
```

**Diese Checks filtern degenerierte Samples:**
- NaN/Inf-Werte
- Roboter zu klein / zu viele verdeckte Keypoints
- BBox zu klein

---

### Schritt 6: YOLO-Pose-Label schreiben

```python
kp_parts = []
for x, y, v in projected:
    if v == 2:
        x = clamp01(x)
        y = clamp01(y)
        kp_parts.append(f"{x:.6f} {y:.6f} 2")
    else:
        kp_parts.append("0.000000 0.000000 0")

label_file = os.path.join(pose_labels_dir, f"{img_id}.txt")
with open(label_file, "w") as f_pose:
    f_pose.write(f"0 {cx:.6f} {cy:.6f} {w:.6f} {h:.6f} " + " ".join(kp_parts) + "\n")
```

**Format:**
```
0 0.50 0.50 0.30 0.40 0.50 0.45 2 0.55 0.50 2 ... (9 Keypoints × 3 Werte)
↑ Klasse (immer 0)
  ↑ BBox (cx, cy, w, h)
                  ↓ Keypoints (x, y, visibility)
```

**Visibility:**
- `2` = sichtbar (im Bild, vor Kamera)
- `0` = nicht sichtbar

---

### Schritt 7: RGB rendern (EEVEE) + Post-Processing

```python
# 7a) RGB rendern
scene.render.engine = "BLENDER_EEVEE"
rgb_path = os.path.join(rgb_dir, f"{img_id}.png")
scene.render.filepath = rgb_path
bpy.ops.render.render(write_still=True)

# 7b) Gausssches Rauschen auf RGB-Bild legen
apply_post_noise(rgb_path, DOMAIN_RAND)

# 7c) Exposure fuer Masken-Render zuruecksetzen
scene.view_settings.exposure = 0.0
```

**EEVEE:** Schneller Echtzeit-Renderer (fotorealistisch aussehend)

**Post-Render-Noise (7b):** Nach dem Rendern wird zufaelliges Gausssches Rauschen auf das PNG gelegt (via numpy). Die Staerke variiert pro Sample von 0.0 (kein Rauschen) bis 0.12 (stark verrauscht). Das macht das Modell robust gegen Bildrauschen echter Kameras.

**Exposure-Reset (7c):** Die zufaellige Belichtung wird auf 0 zurueckgesetzt, bevor die Segmentierungsmaske gerendert wird. Masken brauchen neutrale, exakte Farben.

---

### Schritt 8: Segmentierungs-Maske rendern (Workbench)

```python
scene.render.engine = "BLENDER_WORKBENCH"
scene.display.shading.light = "FLAT"
scene.display.shading.color_type = "OBJECT"
scene.display.render_aa = "OFF"
scene.render.filepath = os.path.join(mask_dir, f"mask_{generated:04d}.png")
bpy.ops.render.render(write_still=True)
```

**Workbench:** 
- Nutzt Objektfarben direkt (keine Beleuchtung)
- FLAT-Shading für saubere Klassenfarben
- AA-Off für exakte Pixel-Farben

**Ausgabe:** Reine Farbbild (Base=Rot, J1=Grün, usw.)

---

### Schritt 9: CSV-Eintrag schreiben

```python
writer.writerow([
    f"{img_id}.png",
    round(blender_angles["joint1"], 2),
    round(blender_angles["joint2"] + 40.0, 2),
    round(blender_angles["joint3"] - 20.0, 2),
    round(blender_angles["joint4"] - 20.0, 2),
])
```

**Format:**
```csv
image_0000.png, -45.50, 75.20, 30.10, 85.60
```

**Wichtig:** Offsets werden hinzugefügt (`+ 40.0`, `- 20.0`), um **Kalibrierungsfehler zu kompensieren**.

Diese Werte **müssen projektspezifisch angepasst werden** (abhängig von Blender-Setup)!

---

# 🚀 Praktisches Beispiel

## Vollständiger Workflow

```bash
# 1. Blender öffnen
blender roboterarm.blend

# 2. Skript im Scripting-Tab öffnen oder mit Alt+P ausführen
# → Blender_Segment_Keypoint_Generation.py

# 3. Konsole beobachten:
# Verwendete Keypoint-Offsets (lokal relativ zu Anchor):
#   base: anchor=base_link, offset=(0.000000, 0.000000, 0.050000)
#   base_z1: anchor=base_link, offset=(0.050000, 0.000000, 0.000000)
#   ...

# 4. Datengenerierung läuft:
# 25/2000 Samples erzeugt...
# 50/2000 Samples erzeugt...
# 100/2000 Samples erzeugt...
# ...
# 2000/2000 Samples erzeugt...

# 5. Statistik:
# Pose-Sanity-Filter:
#   verworfen (sichtbare KP < 6): 234
#   verworfen (BBox ungültig/zu klein): 187
#   verworfen (nicht-endliche Werte): 12
#   Versuche gesamt: 2443

# 6. Output:
# RGB:        /path/to/dataset/images
# Masken:     /path/to/dataset/masks
# PoseLabels: /path/to/dataset/labels_pose
```

---

# 📁 Output-Struktur

```
dataset/
├── images/          (2000 RGB-Bilder)
│   ├── image_0000.png
│   ├── image_0001.png
│   ├── image_0002.png
│   └── ... image_1999.png
│
├── masks/           (2000 Segmentierungsmasken)
│   ├── mask_0000.png  (reine Farben: Base=Rot, J1=Grün, etc.)
│   ├── mask_0001.png
│   └── ... mask_1999.png
│
├── labels_pose/     (2000 YOLO-Pose-Labels)
│   ├── image_0000.txt  (9 Keypoints + BBox)
│   ├── image_0001.txt
│   └── ... image_1999.txt
│
└── joint_data.csv   (Ground-Truth-Winkel)
    image_0000.png,-45.50,75.20,30.10,85.60
    image_0001.png,-12.30,41.50,15.80,-55.30
    ...
```

---

# 🎯 Kalibrierung: Offset-Berechnung

Das Skript versucht automatisch, die lokalen Offsets zu berechnen:

```python
for spec in KEYPOINT_SPECS:
    anchor = find_first_existing(spec["anchor_candidates"])
    
    if spec.get("local_offset") is None:
        ref = find_first_existing(spec.get("reference_candidates", []))
        local_offset = compute_local_offset(anchor, ref)
```

**Zwei Optionen:**

### Option 1: Automatische Berechnung (mit Markern)
```python
{
    "name": "base",
    "anchor_candidates": ["base_link"],
    "reference_candidates": ["KP_CAL_MARKER_base"],  # Marker in Blender setzen!
    "local_offset": None,  # Wird berechnet
}
```

Sie setzen einen Marker `KP_CAL_MARKER_base` in Blender auf die gewünschte Position, das Skript berechnet den Offset.

### Option 2: Direkte Angabe (nach Kalibrierung)
```python
{
    "name": "base",
    "anchor_candidates": ["base_link"],
    "local_offset": (0.0, 0.0, 0.05),  # Direkt angegeben
}
```

Nach einer Kalibrierung mit `keypoint_axis_calibration.py` können Sie die Offsets direkt eintragen.

---

# ⚠️ Häufige Fehler & Lösungen

| Fehler | Ursache | Lösung |
|--------|--------|--------|
| `RuntimeError: Camera Objekt nicht gefunden` | Keine Camera in Blender | Kamera hinzufügen und auf "Camera" benennen |
| `RuntimeError: Gelenkobjekt 'joint1' nicht gefunden` | Joint-Namen stimmen nicht | `joints_config` anpassen oder Blender-Objekte umbenennen |
| `RuntimeError: Keypoint-Specs unvollständig` | Anchor oder Reference fehlt | `KEYPOINT_SPECS` überprüfen oder Marker in Blender setzen |
| Nur ~10% erfolgreiche Samples | Zu strenge Sanity-Checks | `MIN_VISIBLE_KEYPOINTS` reduzieren oder Kamera-Winkel anpassen |
| Offset-Werte sind absurd (z.B. 1000.0) | Einheiten-Mismatch (cm vs. m) | Blender-Einheiten überprüfen |
| Masken haben falsche Farben | `set_segment_colors()` fehlgeschlagen | Objekt-Namen überprüfen ("base", "link1", "link2", etc.) |

---

# ✅ Checkliste vor Datengenerierung

- [ ] Roboterarm-Modell in Blender importiert
- [ ] Kamera hinzugefuegt und benannt ("Camera")
- [ ] Alle Gelenke benannt (joint1, joint2, etc.) oder `joints_config` angepasst
- [ ] Keypoint-Marker gesetzt (`KP_CAL_MARKER_*`) ODER `local_offset` direkt angegeben
- [ ] `num_images` auf gewuenschte Anzahl gesetzt
- [ ] `DOMAIN_RAND` Parameter geprueft (Kamera, Licht, Exposure, Noise)
- [ ] Sanity-Check-Parameter ueberprueft (`MIN_VISIBLE_KEYPOINTS`, `MIN_BBOX_*`)
- [ ] `blend`-Datei gespeichert (damit `base_dir` korrekt ist)
- [ ] Skript ausgefuehrt: **Alt+P** oder **Script -> Run Script**
- [ ] Output-Verzeichnis `dataset/` ueberprueft
- [ ] Beispiele-Bilder angeschaut (images/, masks/)
- [ ] Bilder mit verschiedenen Helligkeiten/Rauschstufen vorhanden?
- [ ] CSV-Datei ueberprueft (Winkel sinnvoll?)
- [ ] `keypoint_axis_calibration.py` zur Validierung ausgefuehrt (optional)

---

# 💡 Best Practices

### Rendering-Qualitaet
- **EEVEE:** Fuer schnelle, fotorealistische RGB-Bilder (bevorzugt)
- **Cycles:** Zu langsam fuer 2000+ Bilder
- **Workbench:** Fuer Masken-Rendering (exakte Farben)

### Domain Randomization Tuning

**Ziel:** Das Modell soll auf echten Bildern funktionieren, nicht nur auf Blender-Bildern.

| Wenn... | Dann... |
|---------|---------|
| Modell erkennt Roboter nur bei guter Beleuchtung | `exposure_range` auf `(-3.0, 2.0)` erweitern |
| Modell ist nicht robust gegen Kamerarauschen | `noise_strength_range` auf `(0.0, 0.15)` erhoehen |
| Modell versagt bei leicht anderer Kameraposition | `camera_loc_jitter` auf `0.05` und `camera_rot_jitter` auf `3.0` erhoehen |
| Zu viele Samples werden verworfen | `camera_rot_jitter` reduzieren oder `MIN_VISIBLE_KEYPOINTS` senken |
| Modell hat Probleme mit verschiedenen Lichtverhaeltnissen | `light_energy_range` auf `(0.2, 3.0)` erweitern |

**Faustregel:** Lieber zu viel Varianz als zu wenig. Das Modell soll auch unter schwierigen Bedingungen funktionieren.

### Sanity-Checks
Wenn <50% erfolgreiche Samples generiert werden:
1. Kamera naeher am Roboter positionieren
2. `MIN_VISIBLE_KEYPOINTS` reduzieren
3. `camera_rot_jitter` reduzieren

### Offset-Kalibrierung
- Praezision ist kritisch! 1mm Fehler fuehrt zu schlechtem Training
- Nutzen Sie `keypoint_axis_calibration.py` fuer visuelle Kontrolle
- Preview-Bilder zeigen ob Marker korrekt sind

---

**Dokumentation erstellt:** 2026-04-17  
**Letzte Aktualisierung:** 2026-04-21 (Domain Randomization: Kamera, Licht, Hintergrund, Belichtung, Rauschen)  
**Skript-Version:** basierend auf Blender_Segment_Keypoint_Generation.py
