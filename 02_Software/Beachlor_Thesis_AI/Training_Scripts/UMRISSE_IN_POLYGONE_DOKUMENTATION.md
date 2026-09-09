# Umrisse_in_Polygone.py — Detaillierte Dokumentation

## 📋 Übersicht

Das Skript **`Umrisse_in_Polygone.py`** ist das zentrale Datenvorbeitungs-Tool für das Roboterarm-Trainingsprojekt. Es bereitet Trainingsdaten in zwei Betriebsmodi auf:

- **[1] Virtuell (Blender):** Konvertiert farbige Blender-Segmentmasken in YOLO-Labels und organisiert Pose-Daten
- **[2] Echt (Real):** Annotiert echte Kamerabilder automatisch mit einem vortrainierten Modell und leitet Pose-Labels ab
- **[3] Winkelberechnung:** Berechnet Gelenkwinkel aus Pose- oder Segmentierungsmodellen und visualisiert sie

Das Skript dient als **Brücke** zwischen der Datenerfassung (Blender-Simulation oder echte Kamerabilder) und dem Training (EdgeAI.py).

---

## 🔧 Systemarchitektur

```
Blender-Daten oder Kamera-Aufnahmen
            ↓
   Umrisse_in_Polygone.py
     (Dieses Skript)
            ↓
  YOLO-formatierte Labels
   (Segmentierung + Pose)
            ↓
        EdgeAI.py
      (Training)
            ↓
   Trainierte Modelle
```

---

## 🎯 Ziel der Konvertierung

### Eingabeformate
- **Blender-Masken:** Farbkodierte PNG-Bilder (HSV-Farbraum)
- **Echte Bilder:** JPEG/PNG-Fotos von der Kamera
- **Pose-CSV:** Gelenkwinkel-Daten

### Ausgabeformate (YOLO-Standard)
- **Segmentierungs-Labels:** Polygon-Koordinaten (normalisiert 0-1)
- **Pose-Labels:** Bounding Box + Keypoint-Koordinaten + Visibility-Flag

---

# 📁 Konfiguration und Konstanten

## Pfade und Verzeichnisse

```python
SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent
```
- **SCRIPT_DIR:** Der Ordner, in dem dieses Skript liegt (`Training_Scripts/`)
- **PROJECT_ROOT:** Das Parent-Verzeichnis (`Beachlor_Thesis_AI/`)

Diese Struktur macht das Skript portabel — es funktioniert unabhängig davon, von wo aus es gestartet wird.

## Datensatz-Verzeichnisse

### Virtuell (Blender)
```
Segmentierung per Blender/
├── dataset/
│   ├── images/          # RGB-Bilder (image_*.png)
│   ├── masks/           # Segmentmasken (mask_*.png)
│   ├── labels/          # Output: YOLO-Seg-Labels
│   └── labels_pose/     # Vorgenerierte YOLO-Pose-Labels
├── pose_dataset/        # Output: Train/Val Split für Pose-Training
└── pose_dataset.yaml    # Output: YOLO-Konfiguration
```

### Echt (Real)
```
real_dataset/
├── images/all/          # Auto-annotierte Bilder (kopiert)
└── labels/all/          # Auto-generierte Seg-Labels

real_pose_dataset/
├── images/
│   ├── train/
│   └── val/
├── labels/
│   ├── train/
│   └── val/
└── real_pose_dataset.yaml
```

## Farbkodierung für Blender-Masken (HSV-Farbraum)

```python
COLOR_RANGES = {
    0:     [H: 0-10, S: 50-255, V: 50-255],       # ROT (Base) — Teil 1
    '0_b': [H: 170-179, S: 50-255, V: 50-255],    # ROT (Base) — Teil 2 (wrap-around)
    1:     [H: 40-85, S: 50-255, V: 50-255],      # GRÜN (Joint 1)
    2:     [H: 90-135, S: 50-255, V: 50-255],     # BLAU (Joint 2)
    3:     [H: 20-35, S: 50-255, V: 50-255],      # GELB (Joint 3)
    4:     [H: 140-165, S: 50-255, V: 50-255],    # MAGENTA (Joint 4 / Finger)
}
```

**Warum HSV statt RGB?** HSV ist robuster gegen Beleuchtungsschwankungen und erlaubt einfache Farbbereichs-Definitionen.

## Gelenkwinkel-Struktur

```python
KP_IDX = {
    "base": 0,          # Basis (Fuß des Arms)
    "base_z1": 1,       # Z-Achsen-Referenzpunkt 1
    "base_z2": 2,       # Z-Achsen-Referenzpunkt 2
    "j1": 3,            # Joint 1 (Rotation um Z-Achse)
    "j1_z1": 4,         # Z-Referenzpunkt für Joint 1
    "j2": 5,            # Joint 2
    "j3": 6,            # Joint 3
    "j4": 7,            # Joint 4 (Gripper)
    "tcp": 8,           # Tool Center Point (Endeffector)
}
```

**Gesamtzahl:** 9 Keypoints pro Label

---

# 🔍 Hilfsfunktionen (Utilities)

## `_path_for_yaml(target: Path, yaml_file: Path) -> str`

```python
def _path_for_yaml(target: Path, yaml_file: Path) -> str:
    """Nutze relative Pfade in YAML, damit Kopieren zwischen Rechnern robust bleibt."""
```

### Zweck
Konvertiert absolute Dateipfade in **relative Pfade** für YAML-Konfigurationsdateien. Dies macht die Konfiguration portabel — das Projekt kann auf verschiedenen Computern verwendet werden, ohne die Pfade zu aktualisieren.

### Logik
```python
1. Versuche, den Ziel-Pfad relativ zum YAML-Verzeichnis zu berechnen
2. Falls das fehlschlägt (unterschiedliche Laufwerke), nutze absoluten Pfad als Fallback
```

### Beispiel
```python
target = Path("C:/Project/dataset/images")
yaml_file = Path("C:/Project/dataset.yaml")

# Ergebnis: "images" (relativ zum YAML-Verzeichnis)
```

---

## `_find_image_capture_root() -> Path`

```python
def _find_image_capture_root() -> Path:
    """Sucht den Image_Capture_Python-Root automatisch oder nutzt ENV-Override."""
```

### Zweck
Findet den Speicherort des `Image_Capture_Python`-Projekts automatisch, selbst wenn es an verschiedenen Orten sein kann.

### Suchlogik (in dieser Reihenfolge)
1. **Umgebungsvariable** `IMAGE_CAPTURE_PYTHON_ROOT` (falls gesetzt)
2. **PROJECT_ROOT** und alle übergeordneten Verzeichnisse nach Ordner `Image_Capture_Python` durchsuchen
3. **Fallback:** Annahme `../Image_Capture_Python` relativ zum Projektroot

### Use-Case
Ermöglicht flexible Projektstrukturen. Beispiel:
```
C:/Dev/
├── Image_Capture_Python/        ← Wird automatisch gefunden
│   └── dataset/images/
└── Beachlor_Thesis_AI/
    └── Training_Scripts/
        └── Umrisse_in_Polygone.py
```

---

## `_has_images(path: Path) -> bool`

```python
def _has_images(path: Path) -> bool:
    """Prüft, ob in einem Ordner PNG/JPG/JPEG-Bilder vorhanden sind."""
```

### Zweck
Schnelle Überprüfung, ob ein Verzeichnis Bilddateien enthält.

### Implementierung
Sucht nach `.png`, `.jpg`, oder `.jpeg`-Dateien mit `glob()`.

### Rückgabewert
- `True` wenn mindestens eine Bilddatei vorhanden ist
- `False` sonst

---

## `_resolve_real_images_dir(images_base_dir: Path) -> Path`

```python
def _resolve_real_images_dir(images_base_dir: Path) -> Path:
    """Ermittelt den effektiv zu nutzenden Realbild-Ordner (neueste Session bevorzugt)."""
```

### Zweck
Findet den aktuellsten Aufnahme-Session-Ordner automatisch, um die neuesten Daten zu verwenden.

### Suchlogik
1. **Umgebungsvariable** `IMAGE_CAPTURE_IMAGES_DIR` (falls gesetzt)
2. **Base-Verzeichnis selbst** (falls Bilder vorhanden)
3. **Unterordner mit Bildern** — nutze den **neuesten** (nach Änderungsdatum)
4. **Fallback:** Das Base-Verzeichnis selbst

### Beispiel
```
images/
├── 2026-04-10_session1/   ← älter
│   └── *.png
├── 2026-04-16_session2/   ← NEUER → wird gewählt
│   └── *.png
└── misc/
    └── *.png
```

---

## `_resolve_ground_truth_csv(dataset_dir: Path, images_dir: Path) -> Path`

```python
def _resolve_ground_truth_csv(dataset_dir: Path, images_dir: Path) -> Path:
    """Bestimmt die passendste Ground-Truth-CSV für den gewählten Bildordner."""
```

### Zweck
Findet automatisch die zugehörige Ground-Truth-CSV (mit Gelenkwinkel-Messwerten) für einen gegebenen Bildordner.

### Suchlogik
1. **Umgebungsvariable** `IMAGE_CAPTURE_LABELS_CSV`
2. **Session-spezifisches CSV:** Falls der Bildordner `2026-04-16_session2/` heißt, suche `labels_2026-04-16_session2.csv`
3. **Neueste CSV:** Suche alle `labels*.csv` und verwende die **jüngste**
4. **Fallback:** `labels.csv`

### Bedeutung
Die Ground-Truth-CSV enthält die tatsächlich gemessenen Gelenkwinkel. Dies wird später für die Validierung der Modellergebnisse benötigt.

---

## `_infer_pose_keypoint_count(label_files, fallback=9) -> int`

```python
def _infer_pose_keypoint_count(label_files, fallback=DEFAULT_POSE_KEYPOINT_COUNT):
    """Liest die Keypoint-Anzahl aus YOLO-Pose-Labels aus."""
```

### Zweck
Bestimmt automatisch, wie viele Keypoints pro Label vorhanden sind, indem die vorhandenen Label-Dateien analysiert werden.

### Logik
```
YOLO-Pose-Format pro Zeile: 
  class_id bbox_x bbox_y bbox_w bbox_h kx1 ky1 conf1 kx2 ky2 conf2 ...

Werte pro Keypoint: 3 (x, y, confidence)
Basis-Werte (BBox): 5 (class_id, x, y, w, h)
Keypoint-Anzahl = (Gesamtwerte - 5) / 3
```

### Beispiel
```python
Zeile: "0 0.5 0.5 0.3 0.3 0.2 0.3 1 0.5 0.6 1 0.8 0.9 1 0.1 0.2 0 0.4 0.5 1"
#      class bbox x,y,w,h                                    
#      ^^^^^^ ───────────────────────────────────────────────────
#             5 Werte                    18 Werte (6 Keypoints × 3)
# → 6 Keypoints erkannt
```

### Konsistenzprüfung
Falls unterschiedliche Keypoint-Anzahlen in verschiedenen Dateien gefunden werden, wird eine **ValueError** geworfen.

---

## `_create_pose_yaml(dataset_dir, keypoint_count) -> str`

```python
def _create_pose_yaml(dataset_dir, keypoint_count):
    """Erzeugt den YAML-Inhalt für ein YOLO-Pose-Dataset."""
```

### Zweck
Erzeugt die YOLO-Konfigurationsdatei für Pose-Training.

### Output-Format
```yaml
path: /absolute/path/to/dataset
train: images/train
val: images/val

kpt_shape: [9, 3]          # 9 Keypoints, je (x, y, conf)
flip_idx: [0, 1, 2, ...]   # Augmentations-Indizes für Spiegelung

names:
  0: robot_arm             # Nur eine Klasse
```

Die `kpt_shape: [9, 3]` teilt YOLO mit: "Erwarte 9 Keypoints mit je 3 Werten (x, y, Konfidenz)."

---

## Geometrie-Hilfsfunktionen für Winkelberechnung

### `_joint_angle(reference_vector, target_vector) -> float`

```python
def _joint_angle(reference_vector, target_vector):
    """Berechnet den signierten Winkel zwischen zwei 2D-Vektoren in Grad."""
```

**Mathematik:** Nutzt **arctan2** um den vorzeichenbehafteten Winkel zu berechnen.

```
Vorzeichen zeigt Rotationsrichtung an:
  + = Gegen den Uhrzeigersinn
  - = Im Uhrzeigersinn

Beispiel:
  reference = [0, -1]  (oben, Vertikale)
  target = [1, 0]      (rechts)
  Winkel = 90° (gegen Uhrzeigersinn um 90°)
```

---

### `_vector_between(joint_points, start_idx, end_idx) -> np.ndarray or None`

```python
def _vector_between(joint_points, start_idx, end_idx):
    """Liefert den Verbindungsvektor zwischen zwei vorhandenen Keypoints."""
```

**Logik:** Subtrahiert Startposition von Endposition.

```
joint_points = {
    0: [100, 200],  # Base
    3: [150, 150],  # Joint 1
}

vector_between(joint_points, 0, 3)
= [150, 150] - [100, 200]
= [50, -50]  ← Vektor von Base zu Joint 1
```

**Rückgabewert:** `None` falls ein Keypoint fehlt.

---

### `_normalize(vec) -> np.ndarray or None`

```python
def _normalize(vec):
    """Normalisiert einen Vektor; bei nahezu Nullvektor wird None zurückgegeben."""
```

**Mathematik:** Teilt Vektor durch seine Länge (Norm).

```
vec = [3, 4]
norm = sqrt(3² + 4²) = 5
normalized = [3/5, 4/5] = [0.6, 0.8]
```

**Sicherheit:** Falls die Länge < 1e-6 (praktisch null), wird `None` zurückgegeben, um Division-durch-Null zu vermeiden.

---

### `_perp(vec) -> np.ndarray`

```python
def _perp(vec):
    """Erzeugt einen 2D-Senkrechtvektor (90° Rotation) zum Eingangsvektor."""
```

**Transformation:** Dreht Vektor um 90° gegen den Uhrzeigersinn.

```
vec = [x, y]
perp = [-y, x]

Beispiel:
vec = [1, 0]  (rechts)
perp = [0, 1]  (oben) ← 90° Rotation
```

---

### `_infer_extra_pose_keypoints(keypoints) -> dict`

```python
def _infer_extra_pose_keypoints(keypoints):
    """Ergänzt fehlende 9er-Keypoints heuristisch (NICHT VERWENDET für echte Daten)."""
```

### Zweck
Für echte Bilder: Nur vom Modell erkannte Keypoints werden verwendet. Fehlende Keypoints (base_z1, base_z2, j1_z1, tcp) werden **manuell in Label Studio annotiert**.

### Heuristiken

#### Base Z-Achsen-Punkte (base_z1, base_z2)
```python
# Senkrecht zur Base→J1-Linie, im Abstand von 25% der Base→J1-Länge
direction_bj = normalize(j1 - base)
perpendicular = perp(direction_bj)
base_z1 = base + perpendicular * offset
base_z2 = base - perpendicular * offset
```

**Zweck:** Definiert ein "virtuelles Koordinatensystem" für Base-Rotationen.

#### Joint 1 Z-Achsen-Punkt (j1_z1)
```python
# Senkrecht zur J1→J2-Linie, im Abstand von 20% der J1→J2-Länge
direction_j1j2 = normalize(j2 - j1)
perpendicular = perp(direction_j1j2)
j1_z1 = j1 + perpendicular * offset
```

#### TCP (Tool Center Point)
```python
# Verlängerung der J3→J4-Linie um 80% der J3→J4-Länge
direction_j3j4 = normalize(j4 - j3)
tcp = j4 + direction_j3j4 * offset
```

**Alle diese Punkte erhalten `vis=1`** (sichtbar), obwohl sie synthetisch berechnet sind. Dies erlaubt das Training auf vollständigen Labels, auch wenn echte Daten nur Teilinformationen enthalten.

---

### `_predict_joint_angles(joint_points) -> dict`

```python
def _predict_joint_angles(joint_points):
    """Berechnet bis zu vier Gelenkwinkel aus der Keypoint-Kette."""
```

### Algorithmus

Die Gelenke werden berechnet als **Winkel zwischen aufeinanderfolgenden Vektoren der Kette**.

```
Kette: Base → J1 → J2 → J3 → J4 → TCP

Joint 1: Winkel(oben, Base→J1)
Joint 2: Winkel(Base→J1, J1→J2)
Joint 3: Winkel(J1→J2, J2→J3)
Joint 4: Winkel(J3→J4, J4→TCP)  [oder Fallback: Winkel(J2→J3, J3→J4)]
```

### Beispiel-Berechnung
```python
joint_points = {
    0: [100, 100],   # Base
    3: [100, 50],    # J1 (20 Pixel höher)
    5: [130, 20],    # J2 (diagonal oben-rechts)
}

# Joint 1
ref_up = [0, -1]
base_vector = [100, 50] - [100, 100] = [0, -50]
angle1 = _joint_angle([0, -1], [0, -50])
       = arctan2(0*(-50) - (-1)*0, 0*0 + (-1)*(-50))
       = arctan2(0, 50) = 0°  ← Gerade nach oben

# Joint 2
j1_vector = [130, 20] - [100, 50] = [30, -30]
angle2 = _joint_angle([0, -50], [30, -30])
       ≈ -45°  ← 45° Drehung im Uhrzeigersinn
```

---

# 🎯 Hauptfunktionen

## Schritt 1: Virtuell — Blender-Daten

### `blender_masken_zu_labels()`

```python
def blender_masken_zu_labels():
    """Konvertiert farbige Blender-Masken in YOLO-Segmentierungs-Labels."""
```

#### Eingabe
```
Segmentierung per Blender/dataset/masks/
├── mask_0001.png   (farbcodiert: Rot=Base, Grün=J1, etc.)
├── mask_0002.png
└── ...
```

#### Prozess

1. **Bild laden & in HSV konvertieren**
   ```python
   img_bgr = cv2.imread(mask_path)      # BGR-Format
   img_hsv = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2HSV)
   ```
   
   **Warum HSV?** Farbbereichs-Schwellwertbildung ist im HSV-Raum robuster.

2. **Für jede Klasse (0-4):**

   **Rote Klasse (0) — Besonderheit:**
   ```python
   mask1 = cv2.inRange(img_hsv, [0, 50, 50], [10, 255, 255])      # H: 0-10
   mask2 = cv2.inRange(img_hsv, [170, 50, 50], [179, 255, 255])   # H: 170-179
   binary_mask = cv2.bitwise_or(mask1, mask2)  # Kombiniere beide Teile
   ```
   
   **Grund:** Der HSV-Hue-Kanal ist zyklisch (0° = 360°), also ist Rot an beiden Enden.

   **Andere Klassen:**
   ```python
   binary_mask = cv2.inRange(img_hsv, lower, upper)
   ```

3. **Konturen finden & filtern**
   ```python
   contours, _ = cv2.findContours(binary_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
   
   for contour in contours:
       if cv2.contourArea(contour) < 50:  # Zu kleine Konturen ignorieren
           continue
   ```

4. **Konturen normalisieren**
   ```python
   for point in contour:
       x = point[0][0] / width    # Pixel → [0, 1]
       y = point[0][1] / height
       polygon.append(f"{x:.6f} {y:.6f}")
   ```

5. **YOLO-Label schreiben**
   ```
   Zeile: class_id norm_x1 norm_y1 norm_x2 norm_y2 ... norm_xN norm_yN
   Beispiel: 0 0.5 0.5 0.55 0.6 0.52 0.7 ...
            ↑ Base mit 3 Eckpunkten
   ```

#### Ausgabe
```
Segmentierung per Blender/dataset/labels/
├── image_0001.txt
├── image_0002.txt
└── ...
```

---

### `csv_zu_pose_labels()`

```python
def csv_zu_pose_labels():
    """Erstellt YOLO-Pose Dataset aus vorgenerierten Labels + Train/Val Split."""
```

#### Eingabe
```
dataset/
├── labels_pose/     (vorgenerierte YOLO-Pose-Labels aus Blender)
│   └── *.txt
└── images/          (RGB-Bilder)
    └── *.png
```

#### Prozess

1. **Keypoint-Anzahl inferieren**
   ```python
   keypoint_count = _infer_pose_keypoint_count(label_files)
   # Beispiel: 9 Keypoints erkannt
   ```

2. **Valid Pairs sammeln** (Bild + Label vorhanden)
   ```python
   valid_pairs = []
   for lbl_path in label_files:
       img_path = image_dir / (lbl_path.stem + ".png")
       if img_path.exists():
           valid_pairs.append((img_path, lbl_path))
   ```

3. **Train/Val Split (standardmäßig 70/30)**
   ```python
   indices = list(range(len(valid_pairs)))
   random.seed(42)
   random.shuffle(indices)
   val_count = int(len(indices) * 0.3)  # 30% für Validierung
   
   splits = {
       "val": indices[:val_count],
       "train": indices[val_count:]
   }
   ```

4. **Dateien kopieren**
   ```python
   for split in ["train", "val"]:
       for idx in splits[split]:
           img_path, lbl_path = valid_pairs[idx]
           shutil.copy2(img_path, POSE_DATASET_DIR / "images" / split / img_path.name)
           shutil.copy2(lbl_path, POSE_DATASET_DIR / "labels" / split / lbl_path.name)
   ```

5. **Dataset-YAML erzeugen**
   ```python
   yaml_content = _create_pose_yaml(POSE_DATASET_DIR, keypoint_count)
   with open(POSE_YAML, "w") as f:
       f.write(yaml_content)
   ```

#### Ausgabe
```
pose_dataset/
├── images/
│   ├── train/  (70% der Bilder)
│   └── val/    (30% der Bilder)
├── labels/
│   ├── train/  (70% der Labels)
│   └── val/    (30% der Labels)
└── pose_dataset.yaml
```

#### Warum dieser Schritt?
- Organisiert Daten in YOLO-Standard-Struktur
- Aufteilt in Train/Val für Validierung während des Trainings
- Sichert Konsistenz (zufällig, aber mit fester Seed)

---

## Schritt 2: Echt — Automatische Annotation

### 📊 Übersicht: Seg vs. Pose für echte Bilder

Bei echten Bildern werden **ZWEI verschiedene Label-Typen** erstellt:

#### 1️⃣ **Segmentierungs-Labels (Seg)**
```
Erstellt von: auto_annotate()
Was: Polygon-Konturen für jede Körperpart (Base, J1, J2, J3, J4)
Format: class_id x1 y1 x2 y2 ... xN yN (normalisiert)
Speichert in: real_dataset/labels/all/

Beispiel:
0 0.50 0.45 0.55 0.50 0.48 0.43    ← Base Polygon (5 Eckpunkte)
1 0.60 0.30 0.65 0.25 0.62 0.28    ← Joint 1 Polygon (3 Eckpunkte)
```

**Verwendung:** Zum Trainieren des Segmentierungsmodells (roboterarm_real)

---

#### 2️⃣ **Pose-Labels (Keypoints)**
```
Erstellt von: seg_zu_pose_labels_real()
Was: 9 Keypoint-Positionen (xyz-Koordinaten) + BBox
Format: class_id bbox_cx bbox_cy bbox_w bbox_h kx1 ky1 vis1 kx2 ky2 vis2 ...
Speichert in: real_pose_dataset/labels/train|val/

Beispiel:
0 0.50 0.50 0.30 0.40 0.50 0.45 1 0.55 0.50 1 0.58 0.52 1 ...
  ↑ Klasse (immer 0)
    ↓ BBox-Center + Größe
                    ↓ 9 Keypoints (je: x, y, visibility)
```

**Verwendung:** Zum Trainieren des Pose-Modells (roboterarm_pose_real)

---

### 🔗 Beziehung zwischen Seg und Pose

```
Echte Bilder
    ↓
auto_annotate()
│ Nutzt: Seg-Modell (vom Blender-Training)
│ Erstellt: Segmentierungsmasken für Base, J1-J4
│ Output: real_dataset/labels/all/ (Polygon-Koordinaten)
│
└→ Optional: Manuell überprüfen in Label Studio
    (nicht zwingend nötig)
    
    ↓
seg_zu_pose_labels_real()
│ Nutzt: Trainiertes Pose-Modell (Real oder Blender)
│ Input: Echte Bilder direkt (nicht Seg-Labels!)
│ Extrahiert: Keypoints DIREKT vom Pose-Modell (nur erkannte)
│ Output: real_pose_dataset/labels/train|val/ (Keypoint-Koordinaten)
│
└→ Manuelle Annotation nötig (Fehlende Keypoints in Label Studio hinzufügen)
    
    ↓
EdgeAI.py [2]
├─ Trainiert: roboterarm_real (Seg-Modell auf realen Daten)
└─ Trainiert: roboterarm_pose_real (Pose-Modell auf realen Daten)
```

**Wichtig:** Pose-Labels werden direkt vom trainierten Pose-Modell generiert! Dies vermeidet Abhängigkeiten von manuellen Seg-Label-Korrektionen und spart Nacharbeit.

---

### `auto_annotate()`

```python
def auto_annotate():
    """Nutzt das vortrainierte Modell um echte Bilder automatisch zu labeln."""
```

#### 📝 Was diese Funktion macht

Lädt das **vortrainierte Seg-Modell** (von Blender) und nutzt es, um echte Kamerabilder **automatisch zu segmentieren**.

Die Ausgabe sind **Segmentierungs-Labels** (Polygon-Koordinaten), nicht Keypoints!

#### Eingabe
```
Image_Capture_Python/dataset/images/
└── <session_folder>/   (automatisch erkannt)
    ├── *.png
    └── *.jpg
```

#### Prozess

1. **Seg-Modell laden** (vom Blender-Training)
   ```python
   from ultralytics import YOLO
   model = YOLO(str(PRETRAINED_MODEL))  # Best.pt vom Blender-Training
   # Das Modell kann Segmentieren: Base, Joint_1, Joint_2, Joint_3, Joint_4
   ```

2. **Für jedes Bild inferieren**
   ```python
   for img_path in image_files:
       results = model(str(img_path), conf=CONF_THRESHOLD, verbose=False)
       r = results[0]  # Erste (einzige) Inferenz
   ```

3. **Segmentierungsmasken extrahieren** (nicht Keypoints!)
   ```python
   label_lines = []
   if r.masks is not None and len(r.masks) > 0:
       h, w = r.orig_shape
       # Für jede erkannte Maske:
       for mask, cls in zip(r.masks.xy, r.boxes.cls):
           class_id = int(cls)  # 0=Base, 1=J1, 2=J2, 3=J3, 4=J4
           
           # Polygon-Punkte normalisieren auf [0, 1]
           polygon = mask / np.array([w, h])
           
           # Koordinaten in Textform: x1 y1 x2 y2 ... xN yN
           coords = " ".join(f"{x:.6f} {y:.6f}" for x, y in polygon)
           
           # YOLO-Seg-Format: class_id + polygon-koordinaten
           label_lines.append(f"{class_id} {coords}")
   ```

   **Ausgabe-Format pro Zeile:**
   ```
   0 0.50 0.45 0.55 0.50 0.48 0.43
   ↑ Klasse (Base)
     ↓ 3 Polygon-Punkte (normalisiert)
   ```

4. **Label schreiben**
   ```python
   with open(lbl_dir / label_name, "w") as f:
       f.write("\n".join(label_lines))
   
   # Falls keine Detektionen: leere Datei wird trotzdem erstellt
   ```

5. **Statistik sammeln**
   ```python
   annotated_count += 1 if label_lines else 0
   empty_count += 1 if not label_lines else 0
   ```

#### Ausgabe
```
real_dataset/
├── images/all/     (Kopien der Originalbilder)
│   ├── pose_0001.png
│   ├── pose_0002.jpg
│   └── ...
└── labels/all/     (Segmentierungs-Labels!)
    ├── pose_0001.txt   (Polygon-Konturen)
    ├── pose_0002.txt
    └── ...
```

#### Label-Beispiel (real_dataset/labels/all/pose_0001.txt)
```
0 0.50 0.45 0.55 0.50 0.48 0.43 0.45 0.48
1 0.60 0.30 0.65 0.25 0.62 0.28
2 0.72 0.15 0.75 0.20 0.68 0.18
3 0.80 0.05 0.85 0.00 0.82 0.08
4 0.88 0.00 0.92 0.05 0.90 0.02
↑ Base-Polygon
  ↑ Joint 1 Polygon
    ↑ Joint 2 Polygon
      ↑ Joint 3 Polygon
        ↑ Joint 4 Polygon
```

#### ⚠️ Wichtig
**Diese Auto-Labels sind VORSCHLÄGE und oft fehlerhaft!**
- Manche Joints werden übersehen (zu hoher Hintergrund)
- Manche Konturen sind falsch
- **Manuell überprüfen und korrigieren** in Label Studio oder CVAT ist essentiell!

Nach Korrektur sollte `seg_zu_pose_labels_real()` erneut ausgeführt werden, um die Pose-Labels zu aktualisieren.

---

### `seg_zu_pose_labels_real()`

```python
def seg_zu_pose_labels_real():
    """Generiert YOLO-Pose-Labels aus Pose-Modell-Inferenz auf echten Bildern."""
```

#### 📝 Was diese Funktion macht

Nutzt das **trainierte Pose-Modell** (von Blender-Daten) um echte Bilder zu inferieren und **extrahiert direkt die Keypoint-Positionen**:

1. **Keypoints werden direkt vom Pose-Modell auslesen** (präzise, basierend auf Blender-Training)
2. **Fehlende Keypoints werden NICHT berechnet** (base_z1, base_z2, j1_z1, tcp)
   - Nur erkannte Keypoints werden verwendet (saubere Daten)
   - Fehlende Keypoints müssen manuell in Label Studio annotiert werden
3. **YOLO-Pose-Format** wird erzeugt (BBox + erkannte Keypoints)

**Vorteil:** Saubere Daten mit hoher Modell-Präzision — du kontrollierst die fehlenden Keypoints durch manuelle Annotation!

#### Eingabe
```
REAL_IMAGES_DIR/  (echte Kamerabilder, automatisch erkannt)
├── *.png
└── *.jpg
```

Die echten Bilder werden direkt mit dem trainierten Pose-Modell inferiert (keine Abhängigkeit von Seg-Labels).

#### Prozess

1. **Pose-Modell laden** (mit Priorität: Real > Blender)
   ```python
   pose_real_path = MODEL_OUTPUT_DIR / "roboterarm_pose_real" / "weights" / "best.pt"
   pose_blender_path = MODEL_OUTPUT_DIR / "roboterarm_pose" / "weights" / "best.pt"
   
   if pose_real_path.exists():
       model_path = pose_real_path  # Fine-Tuned auf realen Daten
   elif pose_blender_path.exists():
       model_path = pose_blender_path  # Ursprüngliches Modell von Blender
   else:
       raise RuntimeError("Kein Pose-Modell gefunden!")
   
   model = YOLO(str(model_path))
   ```

2. **Für jedes echte Bild inferieren**
   ```python
   for img_path in images:
       result = model(str(img_path), verbose=False)[0]
       
       if result.keypoints is None or len(result.keypoints) == 0:
           skipped_no_kps += 1
           continue
   ```

3. **Keypoints DIREKT vom Pose-Modell auslesen** ✨ (Kern der Funktion)
   ```python
   # Keypoints extrahieren: [N_keypoints, 3] (x, y, confidence)
   kps = result.keypoints[0].data[0]
   keypoints = {}
   visible_points = []
   
   for j in range(len(kps)):
       x, y, conf = float(kps[j][0]), float(kps[j][1]), float(kps[j][2])
       if conf > 0.3:  # Konfidenz-Schwelle
           keypoints[j] = (x, y, 2)  # visibility = 2 (sichtbar)
           visible_points.append((x, y))
       else:
           keypoints[j] = (0.0, 0.0, 0)  # nicht sichtbar/unsicher
   ```

   **Das ist anders als bei echten Daten ohne Blender!**
   - Das Modell wurde auf **Blender-Keypoints** trainiert
   - Es nutzt die **gleiche Keypoint-Definition** (base, j1, j2, j3, j4, tcp, etc.)
   - Dies überträgt die Blender-Präzision auf echte Fotos!

4. **Sanity-Check: Genug sichtbare Keypoints?**
   ```python
   if len(visible_points) < 3:
       skipped_low_visibility += 1
       continue
   ```

5. **Fehlende Keypoints NICHT heuristisch ergänzen**
   ```python
   # SYNTHESIZE_EXTRA_REAL_POSE_KEYPOINTS = False
   # Nur vom Modell erkannte Keypoints werden verwendet
   ```

   **Warum nicht?** 
   - Heuristische Werte sind nicht akkurat für echte Daten
   - Manuelle Annotation ist präziser
   - Fehlende Keypoints (base_z1, base_z2, j1_z1, tcp) werden **manuell in Label Studio annotiert**

6. **Train/Val Split** (70/30)
   ```python
   random.seed(42)
   indices = list(range(len(valid_pairs)))
   random.shuffle(indices)
   val_count = int(len(indices) * 0.3)
   splits = {"val": indices[:val_count], "train": indices[val_count:]}
   ```

7. **YOLO-Pose-Label schreiben**
   ```python
   # Für alle Keypoints (0-8):
   kp_parts = []
   for j in range(target_keypoint_count):
       if j in keypoints:
           kx, ky, v = keypoints[j]
           if v > 0:
               kx = max(0.0, min(float(img_w - 1), float(kx)))  # Clamp
               ky = max(0.0, min(float(img_h - 1), float(ky)))
               kp_parts.append(f"{kx / img_w:.6f} {ky / img_h:.6f} {int(v)}")
           else:
               kp_parts.append("0.000000 0.000000 0")
       else:
           kp_parts.append("0.000000 0.000000 0")
   
   # Bounding Box aus sichtbaren Keypoints
   visible_kps = []
   for j in range(target_keypoint_count):
       if j in keypoints:
           kx, ky, v = keypoints[j]
           if v > 0:
               visible_kps.append((kx, ky))
   
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
   
   # YOLO-Pose-Format: class + bbox + keypoints
   label = f"0 {cx:.6f} {cy:.6f} {bw:.6f} {bh:.6f} " + " ".join(kp_parts)
   ```

#### Ausgabe
```
real_pose_dataset/
├── images/
│   ├── train/  (70% der Bilder)
│   │   ├── pose_0001.png
│   │   ├── pose_0002.png
│   │   └── ...
│   └── val/    (30% der Bilder)
│       ├── pose_0100.png
│       └── ...
├── labels/
│   ├── train/  (70% der Pose-Labels)
│   │   ├── pose_0001.txt   (9 Keypoints!)
│   │   ├── pose_0002.txt
│   │   └── ...
│   └── val/    (30% der Pose-Labels)
│       ├── pose_0100.txt
│       └── ...
└── real_pose_dataset.yaml
```

#### Label-Beispiel (real_pose_dataset/labels/train/pose_0001.txt)
```
0 0.50 0.50 0.30 0.40 0.50 0.45 1 0.55 0.50 1 0.58 0.52 1 0.60 0.48 2 0.65 0.52 2 0.72 0.15 1 0.75 0.20 1 0.68 0.18 1 0.80 0.05 1 0.88 0.00 1
↑ Klasse
  ↑ BBox-Center-X, Center-Y, Width, Height
                  ↓ Keypoint 0-8 (je: x, y, visibility)
                    - Keypoint 0 (Base): (0.50, 0.45, vis=1)
                    - Keypoint 1 (base_z1): (0.55, 0.50, vis=1)
                    - ... (insgesamt 9 Keypoints)
```

#### ✨ Wichtiger Unterschied zu älteren Versionen

**Alt (überholte Methode):** Pose-Labels wurden aus Seg-Masken-Schwerpunkten berechnet.
- Problem: Verlust von Präzision
- Abhängigkeit: Fehler in Seg-Labels → Fehler in Pose-Labels
- Nacharbeit: Viel manuelle Arbeit in Label Studio nötig

**Neu (aktuelle Methode):** Pose-Labels werden **direkt vom trainierten Pose-Modell** extrahiert.
- Vorteil: Volle Präzision des Modells
- Unabhängig: Nutzt echte Bilder direkt, nicht Seg-Labels
- Saubere Daten: Nur erkannte Keypoints (keine heuristischen Annahmen)
- Manuelle Kontrolle: Fehlende Keypoints werden manuell annotiert

```
Alt:  Seg-Masken → Schwerpunkte → Keypoints (fehleranfällig)
Neu:  Echte Bilder → Pose-Modell → Keypoints direkt (präzise)
      → Manuell fehlende ergänzen (sauber)
```

Dies ist die **empfohlene und präferierte Methode** für hochwertige Pose-Labels!

---

### 📋 Vergleich: Segmentierung vs. Pose (echte Bilder)

| Aspekt | **Segmentierung (Seg)** | **Pose (Keypoints)** |
|--------|------------------------|----------------------|
| **Erstellt von** | `auto_annotate()` | `seg_zu_pose_labels_real()` |
| **Input** | Echte Bilder | Echte Bilder (direkt) |
| **Modell verwendet** | Seg-Modell (roboterarm_seg) | Pose-Modell (roboterarm_pose) |
| **Output-Format** | Polygon-Konturen | Keypoint-Koordinaten + BBox |
| **Anzahl pro Bild** | 5 Polygone (0, 1, 2, 3, 4) | 9 Keypoints (0-8) |
| **Speicherort** | `real_dataset/labels/all/` | `real_pose_dataset/labels/train\|val/` |
| **Sichtbar im Bild** | Ja (Umrisse) | Ja (Punkte + Skeleton) |
| **Manuell überprüfen?** | **JA, SEHR WICHTIG** | Minimal (Modell ist trainiert) |
| **Wird trainiert mit** | EdgeAI.py [2] Seg-Phase | EdgeAI.py [2] Pose-Phase |
| **YOLO-Format** | `class_id x1 y1 x2 y2 ... xN yN` | `class_id cx cy w h kx1 ky1 vis1 ...` |
| **Beispiel-Zeile** | `0 0.50 0.45 0.55 0.50` | `0 0.50 0.50 0.30 0.40 0.50 0.45 1 ...` |
| **Abhängigkeit** | Independent | ✅ Unabhängig (direkt vom Pose-Modell) |
| **Nacharbeit nötig?** | ⚠️ Ja (Auto-Labels überprüfen) | ✅ Minimal (Auto-Labels von Modell) |

---

### 🔄 Datenfluss (detailliert)

```
Echte Kamerabilder
    │
    ├─→ [1] auto_annotate()
    │   Lädt: roboterarm_seg (Blender-Modell)
    │   Macht: Inferenz → Segmentierungsmasken
    │   Output: real_dataset/labels/all/*.txt (Polygone)
    │   ⚠️ Sollten manuell überprüft werden (optional)
    │
    ├─→ [2] Optional: Manuelle Kontrolle in Label Studio
    │   (Fehler korrigieren, aber nicht zwingend)
    │
    └─→ [3] seg_zu_pose_labels_real()
        Lädt: roboterarm_pose (Blender- oder Real-Modell) ← POSE-Modell!
        Input: Echte Bilder direkt (nicht Seg-Labels!)
        Macht: 
          - Pose-Modell-Inferenz
          - Keypoints DIREKT auslesen (nur erkannte!)
          - BBox berechnen
          - Erkannte Keypoints speichern
        Output: real_pose_dataset/labels/train|val/*.txt (erkannte Keypoints)
        ⚠️ Manuell: Fehlende Keypoints in Label Studio annotieren!
        
        ↓
        [4] EdgeAI.py [2]
        Trainiert ZWEI Modelle:
          a) roboterarm_real (Seg-Modell auf realen Daten)
          b) roboterarm_pose_real (Pose-Modell auf realen Daten)
```

---

### 🎯 Wichtige Erkenntnisse

#### ✅ Das ist die RICHTIGE Reihenfolge:

1. **auto_annotate()** zuerst
   - Erstellt Seg-Labels (Polygone)
   - Optional: manuell überprüfen (nicht zwingend)

2. **seg_zu_pose_labels_real()** danach
   - Nutzt trainiertes Pose-Modell
   - Extrahiert Keypoints DIREKT (hochwertig)
   - Unabhängig von Seg-Label-Qualität

3. **Training mit EdgeAI.py [2]**
   - Beide Modelle trainieren (Seg + Pose)

#### ✨ Der Vorteil der neuen Methode:

- **Keine Abhängigkeit von Seg-Labels** für Pose-Training
- **Höhere Qualität** durch direktes Modell-Output
- **Weniger manuelle Arbeit** — Auto-Labels von Modell sind bereits hochwertig
- **Blender-Konsistenz** — Pose-Definition wird direkt übernommen

#### ❌ Das ist FALSCH:

- `seg_zu_pose_labels_real()` NICHT ausführen
  → Sie erhalten keine Pose-Labels für echte Bilder!

- Nur Seg-Labels trainieren, ohne Pose-Training
  → Pose-Vorhersagen unmöglich!

#### 💡 Workflow-Tipps:

**Schneller Workflow (empfohlen):**
```
1. auto_annotate()      → Seg-Labels
2. seg_zu_pose_labels_real()  → Pose-Labels
3. EdgeAI.py [2]        → Training (alles automatisch)
```

**Wenn Sie Seg-Labels manuell überprüfen möchten:**
```
1. auto_annotate()      → Seg-Labels
2. Label Studio Überprüfung (optional)
3. seg_zu_pose_labels_real()  → Pose-Labels
4. EdgeAI.py [2]        → Training
```

---

### 🖼️ Visuelle Unterschiede: Was Sie sehen

#### Segmentierungs-Label visualisiert

```
Original-Bild mit Seg-Labels:

┌─────────────────────────┐
│                         │
│    [Roboterarm]         │
│    /  |    |   /        │
│   /   |    |  /         │
│  ○────●────●─●          │
│       ↑    ↑ ↑          │
│    Polygon-Konturen     │
│                         │
└─────────────────────────┘

Sichtbar:
- 5 farbige Polygon-Umrisse (Base, J1, J2, J3, J4)
- Jede Klasse = ein Polygon
- Viele Eckpunkte zur genauen Konturdarstellung
```

**YOLO-Format:**
```
0 0.50 0.45 0.55 0.50 0.48 0.43    ← Base Polygon (3 Punkte)
1 0.60 0.30 0.65 0.25              ← Joint 1 Polygon (2 Punkte)
2 0.72 0.15 0.75 0.20 0.68 0.18    ← Joint 2 Polygon (3 Punkte)
```

---

#### Pose-Label visualisiert

```
Original-Bild mit Pose-Labels:

┌─────────────────────────┐
│                         │
│    [Roboterarm]         │
│    /  |    |   /        │
│   /   |    |  /         │
│  ●────●────●─●─●        │
│  ↓    ↓    ↓ ↓ ↓        │
│ Keypoints (Punkte)      │
│ + Skeleton (Linien)     │
│                         │
└─────────────────────────┘

Sichtbar:
- 9 Keypoint-Punkte (nummeriert 0-8)
- Skeleton-Linien verbinden Keypoints
- Kompakt + geometrisch strukturiert
```

**YOLO-Format:**
```
0 0.50 0.50 0.30 0.40 0.50 0.45 1 0.55 0.50 1 0.58 0.52 1 0.60 0.48 2 ...
  ↓ Klasse (immer 0, nur 1 Klasse "robot_arm")
    ↓ BBox (Center + Size)
                ↓ Keypoint 0 (x, y, visibility)
                            ↓ Keypoint 1 (x, y, visibility)
                                        ↓ ...9 Keypoints total
```

---

#### Nebeneinander-Vergleich

```
SEGMENTIERUNG                    POSE
═════════════════════════════════════════════════════════════════

5 separate Polygone              1 Bounding Box + 9 Keypoints

O════●────●───●           O─────●─────●─────●
↑    ↑    ↑   ↑            │     │     │     │
Base J1   J2  J3           0  1  2  3  4  5  6  7  8
                           └─────┴─────┴─────┘
                          Skeleton-Linien

Genauer für Konturen       Präziser für Bewegung

~20-50 Punkte pro Label    Exakt 9 Punkte pro Label

Konturen-Details zeigen    Gelenkpositionen + Winkel
was die Teile sind         zeigen wie sie sich bewegen
```

---

#### Im Label Studio

**Segmentierungs-Projekt:**
```
Canvas zeigt:
- Polygon-Masken (Pinsel oder Polygon-Tool)
- Unterschiedliche Farben pro Klasse
- Zum Zeichnen/Editieren: Polygon-Tool verwenden
```

**Pose-Projekt:**
```
Canvas zeigt:
- Keypoint-Punkte (kleine Kreise)
- Skeleton-Linien zwischen Punkten
- Zum Zeichnen/Editieren: Keypoint-Tool verwenden
```

---

### 📊 Workflow-Entscheidung

**Was trainieren Sie mit welchen Labels?**

```
roboterarm_real (Seg-Modell)
    ↑
    Nutzt: real_dataset/labels/all/*.txt
    Format: Polygone (5 Klassen)
    Aufgabe: Segmentieren (gibt Pixel-Masken)
    
roboterarm_pose_real (Pose-Modell)
    ↑
    Nutzt: real_pose_dataset/labels/train|val/*.txt
    Format: Keypoints (9 pro Bild, 1 Klasse)
    Aufgabe: Pose (gibt Gelenkpositionen)
```

**Sie können BEIDE Modelle trainieren:**
```
EdgeAI.py [2]
  Phase 1: Trainiert roboterarm_real mit real_dataset/
           (Segmentierungs-Labels)
  Phase 2: Trainiert roboterarm_pose_real mit real_pose_dataset/
           (Pose-Labels, abgeleitet aus Seg-Labels)
```

Dies ermöglicht zwei verschiedene Anwendungen:
- **Seg-Modell:** "Welche Teile des Arms sind wo?"
- **Pose-Modell:** "Welche Gelenkwinkel hat der Arm?"

#### Eingabe
```
REAL_IMAGES_DIR/  (echte Bilder, automatisch erkannt)
├── *.png
└── *.jpg
```

#### Prozess

1. **Seg-Modell laden** (mit Priorität)
   ```python
   finetuned = MODEL_OUTPUT_DIR / "roboterarm_real" / "weights" / "best.pt"
   model_path = finetuned if finetuned.exists() else PRETRAINED_MODEL
   ```

2. **Für jedes Bild inferieren**
   ```python
   for img_path in images:
       result = model(str(img_path), verbose=False)[0]
       
       if result.masks is None or len(result.masks) == 0:
           skipped += 1
           continue
   ```

3. **Beste Detektion pro Klasse behalten**
   ```python
   segments = {}
   for mask_xy, cls, conf in zip(result.masks.xy, result.boxes.cls, result.boxes.conf):
       cid = int(cls)
       if cid not in segments or float(conf) > segments[cid][1]:
           segments[cid] = (mask_xy, float(conf))  # Überschreibe wenn höherer Confidence
   ```

4. **Schwerpunkte zu Keypoints**
   ```python
   keypoints = {}
   for cid in [0, 1, 2, 3, 4]:  # Alle 5 Segmentierungs-Klassen
       if cid in segments:
           pts = segments[cid][0]
           kp_idx = SEGMENT_CLASS_TO_KP_INDEX[cid]  # Mapping: Klasse → Keypoint-Index
           keypoints[kp_idx] = (float(np.mean(pts[:, 0])), float(np.mean(pts[:, 1])), 2)
           # vis=2 bedeutet "sichtbar und hochzuverlässig"
   ```

   **Mapping:**
   ```python
   SEGMENT_CLASS_TO_KP_INDEX = {
       0: 0,   # Base (Klasse 0) → Keypoint 0
       1: 3,   # J1 (Klasse 1) → Keypoint 3
       2: 5,   # J2 (Klasse 2) → Keypoint 5
       3: 6,   # J3 (Klasse 3) → Keypoint 6
       4: 7,   # J4 (Klasse 4) → Keypoint 7
   }
   ```

5. **KEINE heuristische Ergänzung** (disabled für saubere Daten)
   ```python
   # SYNTHESIZE_EXTRA_REAL_POSE_KEYPOINTS = False
   # Nur erkannte Keypoints vom Modell verwenden
   # Fehlende Keypoints müssen manuell annotiert werden
   ```

6. **Train/Val Split**
   ```python
   random.seed(42)
   indices = list(range(len(valid_pairs)))
   random.shuffle(indices)
   val_count = int(len(indices) * 0.3)
   splits = {"val": indices[:val_count], "train": indices[val_count:]}
   ```

7. **YOLO-Pose-Label schreiben**
   ```python
   # Für jedes Keypoint (0-8, insgesamt 9):
   kps = []
   for j in range(9):
       if j in keypoints:
           kx, ky, vis = keypoints[j]
           kx = min(img_w - 1, max(0, kx))  # Clamp auf Bildgrenzen
           ky = min(img_h - 1, max(0, ky))
           kps.append((kx / img_w, ky / img_h, int(vis)))
       else:
           kps.append((0.0, 0.0, 0))  # Unsichtbar
   
   # BBox aus sichtbaren Keypoints berechnen
   visible_kps = [(kx * img_w, ky * img_h) for kx, ky, v in kps if v > 0]
   if visible_kps:
       xs = [p[0] for p in visible_kps]
       ys = [p[1] for p in visible_kps]
       x_min = max(0, min(xs) - KEYPOINT_PADDING)
       x_max = min(img_w, max(xs) + KEYPOINT_PADDING)
       y_min = max(0, min(ys) - KEYPOINT_PADDING)
       y_max = min(img_h, max(ys) + KEYPOINT_PADDING)
   
   # Normalisierte BBox + Keypoints
   cx = ((x_min + x_max) / 2) / img_w
   cy = ((y_min + y_max) / 2) / img_h
   bw = (x_max - x_min) / img_w
   bh = (y_max - y_min) / img_h
   
   kp_str = " ".join(f"{kx:.6f} {ky:.6f} {int(v)}" for kx, ky, v in kps)
   label = f"0 {cx:.6f} {cy:.6f} {bw:.6f} {bh:.6f} {kp_str}"
   ```

#### Ausgabe
```
real_pose_dataset/
├── images/
│   ├── train/  (70% der Bilder)
│   └── val/    (30% der Bilder)
├── labels/
│   ├── train/  (70% der Pose-Labels)
│   └── val/    (30% der Pose-Labels)
└── real_pose_dataset.yaml
```

---

## Schritt 3: Winkelberechnung

### `winkel_berechnen()`

```python
def winkel_berechnen():
    """Berechnet Gelenkwinkel aus den Segmentierungsmasken oder Keypoints."""
```

#### Eingabe
```
REAL_IMAGES_DIR/  (echte Bilder)
├── *.png
└── *.jpg

GROUND_TRUTH_CSV  (optional, mit gemessenen Winkeln)
```

#### Prozess

1. **Modell auswählen** (automatisch, mit Priorität)
   ```python
   # Priorität: Pose (Real) > Pose (Blender) > Seg (Fine-Tuned) > Seg (Blender)
   if pose_real_path.exists():
       use_pose = True
   elif pose_blender_path.exists():
       use_pose = True
   elif finetuned.exists():
       use_pose = False
   else:
       use_pose = False
   ```

2. **Ground-Truth-CSV laden** (falls vorhanden)
   ```python
   gt_data = {}
   if GROUND_TRUTH_CSV.exists():
       df = pd.read_csv(GROUND_TRUTH_CSV)
       for _, row in df.iterrows():
           fname = Path(row["image_file"]).name
           gt_data[fname] = [row["joint1_deg"], row["joint2_deg"], ...]
   ```

3. **Für jedes Bild inferieren**
   ```python
   for img_path in images:
       result = model(str(img_path), verbose=False)[0]
       joint_points = {}
   ```

4. **Keypoints extrahieren**

   **Falls Pose-Modell:**
   ```python
   if use_pose:
       if result.keypoints is None:
           continue
       kps = result.keypoints[0].data[0]  # [N_keypoints, 3]
       for j in range(len(kps)):
           x, y, conf = kps[j]
           if conf > 0.3:  # Nur Keypoints mit hoher Konfidenz
               joint_points[j] = np.array([x, y])
   ```

   **Falls Seg-Modell:**
   ```python
   else:
       if result.masks is None:
           continue
       segments = {}
       for mask_xy, cls, conf in zip(result.masks.xy, result.boxes.cls, result.boxes.conf):
           cid = int(cls)
           if cid not in segments or float(conf) > segments[cid][1]:
               segments[cid] = (mask_xy, float(conf))
       for cid in [0, 1, 2, 3, 4]:
           if cid in segments:
               kp_idx = SEGMENT_CLASS_TO_KP_INDEX[cid]
               pts = segments[cid][0]
               joint_points[kp_idx] = np.array([np.mean(pts[:, 0]), np.mean(pts[:, 1])])
   ```

5. **Winkel berechnen**
   ```python
   predicted = _predict_joint_angles(joint_points)
   # predicted = {0: 45.2, 1: -30.5, 2: 15.0, 3: 120.0}
   ```

6. **Visualisierung zeichnen**
   ```python
   # Skelett-Linien
   for start_idx, end_idx in POSE_CONNECTIONS:
       if start_idx in joint_points and end_idx in joint_points:
           p1 = tuple(int(x) for x in joint_points[start_idx])
           p2 = tuple(int(x) for x in joint_points[end_idx])
           cv2.line(img, p1, p2, (255, 255, 255), 2)
   
   # Keypoint-Kreise + Labels
   for j in chain:
       cx, cy = joint_points[j]
       color = colors.get(j, (255, 255, 255))
       cv2.circle(img, (int(cx), int(cy)), 8, color, -1)
       label = f"J{j}"
       if j in predicted:
           label += f" {predicted[j]:.1f}°"
       cv2.putText(img, label, (int(cx) + 12, int(cy) - 12),
                   cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)
   
   # Ground-Truth-Vergleich (falls CSV vorhanden)
   if gt:
       for j in range(4):
           gt_val = gt[j]
           pred_val = predicted.get(j, None)
           text = f"J{j+1}: {gt_val:+.1f}° | Pred: {pred_val:+.1f}°"
           cv2.putText(img, text, (10, y), ...)
   ```

7. **Metriken berechnen** (MAE)
   ```python
   errors = {j: [] for j in range(4)}
   for r in all_results:
       if r["gt"] is None:
           continue
       for j in range(4):
           if j in r["predicted"]:
               errors[j].append(abs(r["predicted"][j] - r["gt"][j]))
   
   for j in range(4):
       if errors[j]:
           mae = np.mean(errors[j])
           print(f"Joint_{j+1}: {mae:.1f}° MAE")
   ```

#### Ausgabe
```
winkel_output/
├── image_0001.png  (mit Skelett + Winkeln + GT-Vergleich)
├── image_0002.png
└── ...

Console-Output:
  Joint_1: 5.3° MAE
  Joint_2: 8.1° MAE
  Joint_3: 4.7° MAE
  Joint_4: 12.5° MAE
```

---

# 🎬 Hauptprogramm & Menü

## Menü-System

```python
if __name__ == "__main__":
    steps = {
        "1": ("Virtuell — Daten vorbereiten", daten_virtuell),
        "2": ("Echt — Daten vorbereiten", daten_echt),
        "3": ("Winkelberechnung", winkel_berechnen),
    }
    
    if len(sys.argv) > 1:
        choice = sys.argv[1]  # Direkte Auswahl
    else:
        # Interaktives Menü
        print("Roboterarm - Datenvorbereitung")
        for key, (desc, _) in steps.items():
            print(f"  [{key}] {desc}")
        choice = input("Schritt wählen (1-3): ")
```

### Aufruf

```bash
# Interaktives Menü
python Umrisse_in_Polygone.py

# Direkt Schritt 1
python Umrisse_in_Polygone.py 1

# Direkt Schritt 2
python Umrisse_in_Polygone.py 2

# Direkt Schritt 3
python Umrisse_in_Polygone.py 3
```

---

# 🔄 Zusammenfassung: Datenfluss

```
Blender-Simulation
    ↓
Blender_Segment_Keypoint_Generation.py erzeugt:
├── dataset/images/       (RGB)
├── dataset/masks/        (Segmentmasken, farbcodiert)
└── dataset/labels_pose/  (vorgenerierte Pose-Labels)
    ↓
[1] Umrisse_in_Polygone.py [1]
├─→ blender_masken_zu_labels()
│   → dataset/labels/ (YOLO-Seg-Labels)
└─→ csv_zu_pose_labels()
    → pose_dataset/ (Train/Val Split)
    ↓
    EdgeAI.py [1]  (Training auf Blender-Daten)
    → roboterarm_seg (Seg-Modell)
    → roboterarm_pose (Pose-Modell)
    
┌────────────────────────────────────────────┐
│        REALE KAMERABILDER ERFASST           │
│ (in Image_Capture_Python/dataset/images/)  │
└────────────────────────────────────────────┘
    ↓
[2] Umrisse_in_Polygone.py [2]
├─→ auto_annotate()
│   (mit vortrainiertem Seg-Modell)
│   → real_dataset/labels/all/ (Auto-Labels)
├─→ Optional: CVAT/Label Studio (Überprüfung)
└─→ seg_zu_pose_labels_real()
    (mit trainiertem Pose-Modell)
    → real_pose_dataset/ (Train/Val Split)
    ↓
    EdgeAI.py [2]  (Fine-Tuning auf realen Daten)
    → roboterarm_real (Seg-Modell Real)
    → roboterarm_pose_real (Pose-Modell Real)
    
┌────────────────────────────────────────────┐
│    TEST: WINKELBERECHNUNG & VALIDIERUNG     │
└────────────────────────────────────────────┘
    ↓
[3] Umrisse_in_Polygone.py [3]
    → winkel_berechnen()
    (Inferenz auf Testbildern)
    → winkel_output/ (Visualisierungen)
    → Console-Output (MAE-Metriken)
    
    EdgeAI.py [3] (Validierung)
    EdgeAI.py [4] (ONNX-Export)
```

---

# 🚀 Praktische Anwendungsbeispiele

## Scenario 1: Erste Trainierung mit Blender-Daten

```bash
# 1. Blender rendert Bilder + Masken
#    → dataset/images/, dataset/masks/, dataset/labels_pose/

# 2. Datenvorbereitung
python Umrisse_in_Polygone.py 1

# Output:
# ✓ 500 Masken → 500 Seg-Labels
# ✓ 500 Pose-Labels organisiert (Train/Val Split)
# → pose_dataset/ bereit

# 3. Training starten
python EdgeAI.py 1
```

## Scenario 2: Fine-Tuning mit echten Daten

```bash
# 1. Kamera erfasst Bilder
#    → Image_Capture_Python/dataset/images/<session>/

# 2. Auto-Annotation + Pose-Erzeugung (mit trainiertem Modell)
python Umrisse_in_Polygone.py 2

# Output:
# ✓ Auto-Labels in real_dataset/labels/all/ (Seg-Labels)
# ✓ Pose-Labels in real_pose_dataset/ (hochwertige Auto-Labels!)

# 3. Optional: Überprüfung in Label Studio
#    (bei Bedarf Seg-Labels korrigieren)

# 4. Training mit realen Daten
python EdgeAI.py 2
```

**Voraussetzung für Schritt 2:** Das Pose-Modell muss trainiert sein (von Schritt 1/EdgeAI.py [1])

## Scenario 3: Validierung & Winkelberechnung

```bash
# Nach erfolgreichem Training:

# 1. Winkelberechnung visualisieren
python Umrisse_in_Polygone.py 3

# Output:
# ✓ Skeleton-Visualisierungen in winkel_output/
# ✓ MAE (Mean Absolute Error) für jeden Joint
# → Zeigt Modell-Genauigkeit

# 2. Formale Validierung
python EdgeAI.py 3
```

---

# 📊 Ausgabeformate

## YOLO-Segmentierungs-Label

```
image_0001.txt:

0 0.50 0.45 0.55 0.50 0.48 0.43 0.45 0.48
1 0.60 0.30 0.65 0.25 0.62 0.28
2 0.72 0.15 0.75 0.20 0.68 0.18
3 0.80 0.05 0.85 0.00 0.82 0.08
4 0.88 0.00 0.92 0.05 0.90 0.02

↑ Klasse
  ↓ Polygon-Punkte (x1 y1 x2 y2 ... xN yN)
  (normalisiert auf 0-1)
```

## YOLO-Pose-Label

```
image_0001.txt:

0 0.50 0.50 0.30 0.40 0.50 0.45 1 0.55 0.50 1 0.58 0.52 1 ... (9 Keypoints × 3)

↑ Klasse (immer 0)
  ↓ BBox-Center-X, Center-Y, Width, Height
                    ↓ Keypoint-Koordinaten + Visibility (0=unsichtbar, 1+=sichtbar)
```

---

# 🐛 Häufige Fehler & Lösungen

| Problem | Ursache | Lösung |
|---------|--------|--------|
| `FEHLER: Keine Masken gefunden` | `Segmentierung per Blender/dataset/masks/` ist leer | Blender-Render ausführen |
| `Keine Bilder gefunden!` | `Image_Capture_Python/dataset/images/` ist leer oder falsch | Umgebungsvariable `IMAGE_CAPTURE_PYTHON_ROOT` setzen |
| `Ungültiges Pose-Label: 100 Werte` | Inkonsistente Keypoint-Anzahl in Labels | Labels konsistent machen (alle 9 Keypoints) |
| `Modell nicht gefunden` | Vortrainiertes Modell in `Trained_Models/` fehlt | Erst `EdgeAI.py [1]` ausführen |
| Sehr hohe MAE-Fehler | Modell ist untrain oder Daten schlecht | `EdgeAI.py [2]` zum Fine-Tuning ausführen |

---

# ✅ Checkliste für vollständigen Workflow

- [ ] Blender-Daten generiert (images + masks + labels_pose)
- [ ] `python Umrisse_in_Polygone.py 1` erfolgreich ausgeführt
- [ ] `python EdgeAI.py 1` trainiert erfolgreich
- [ ] Echte Bilder erfasst in `Image_Capture_Python/`
- [ ] `python Umrisse_in_Polygone.py 2` erfolgreich ausgeführt
- [ ] Labels manuell überprüft/korrigiert
- [ ] `python EdgeAI.py 2` trainiert erfolgreich
- [ ] `python Umrisse_in_Polygone.py 3` zeigt annehmbare MAE-Werte
- [ ] `python EdgeAI.py 4` exportiert ONNX-Modell

---

**Dokumentation erstellt:** 2026-04-16  
**Skript-Version:** basierend auf Umrisse_in_Polygone.py
