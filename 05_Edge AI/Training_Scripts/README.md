# Roboterarm Joint-Erkennung — KI-Dokumentation

## Übersicht

Ziel des Projekts ist die **Segmentierung der 5 Sektionen eines Roboterarms** mittels YOLO, um daraus die Gelenkwinkel (Joint Angles) abzuleiten. Das Training erfolgt in zwei Phasen:

1. **Pre-Training** auf synthetischen Blender-Simulationsdaten (Formwissen)
2. **Fine-Tuning** auf echten Kamerabildern (Realwelt-Transfer)

### Blender: Diese 2 Python-Skripte brauchst du

1. `Segmentierung per Blender/Blender_Segment_Keypoint_Generation.py`
   Erzeugt aus Blender die Datengrundlage in `dataset/images`, `dataset/masks` und `dataset/labels_pose`.

2. `Segmentierung per Blender/keypoint_axis_calibration.py`
   Kalibriert die 3D-Keypoints mit `KP_CAL_MARKER_*` und gibt konsistente Offsets + Preview aus.

### Klassen

| ID | Name            | Farbe (Blender-Maske) |
|----|-----------------|----------------------|
| 0  | Base            | Rot                  |
| 1  | Joint 1         | Grün                 |
| 2  | Joint 2         | Blau                 |
| 3  | Joint 3         | Gelb                 |
| 4  | Joint 4 / Finger| Magenta              |

---

## Projektstruktur

```
Beachlor_Thesis_AI/
├── Training_Scripts/
│   ├── EdgeAI.py                  # KI-Training (Virtuell / Echt / Validierung / Export)
│   ├── Umrisse_in_Polygone.py     # Datenvorbereitung (Virtuell / Echt / Winkel)
│   ├── Segmentierung per Blender/
│   │   ├── Blender_Segment_Keypoint_Generation.py # Blender-Generierung (RGB/Mask/Pose-Labels)
│   │   ├── keypoint_axis_calibration.py           # Kalibrierung der KP_CAL_MARKER_*
│   │   ├── dataset/
│   │   │   ├── images/            # Blender-RGB-Bilder (image_*.png)
│   │   │   ├── masks/             # Blender-Segmentmasken (mask_*.png)
│   │   │   ├── labels/            # YOLO-Seg Labels
│   │   │   ├── labels_pose/       # YOLO-Pose Labels aus Blender
│   │   │   └── keypoint_calibration/ # Kalibrierungs-Logs + Preview-Bilder
│   │   ├── yolo_dataset/          # YOLO-Seg Ordnerstruktur (images/labels train/val)
│   │   ├── pose_dataset/          # YOLO-Pose Ordnerstruktur (images/labels train/val)
│   │   ├── dataset.yaml           # YOLO-Seg Dataset-Konfiguration
│   │   └── pose_dataset.yaml      # YOLO-Pose Dataset-Konfiguration
│   ├── label_studio_converter.py  # YOLO -> Label Studio Task-JSON (Predictions)
│   ├── Validierung.ipynb          # Visualisierung & Auswertung der Ergebnisse
│   ├── winkel_output/             # Visualisierte Bilder mit Skeleton + Winkeln
│   ├── label_studio_import/       # Fertige Importdateien + Label-Configs fuer Label Studio
│   ├── ResearchEdgeAI.ipynb       # MobileNetV3 Regression (Forschung/Alternative)
│   ├── real_dataset/              # Auto-annotierte echte Bilder (Seg)
│   ├── real_pose_dataset/         # Pose-Labels für echte Bilder
│   └── ...
├── Trained_Models/
│   └── AI-Models/
│       ├── roboterarm_seg/        # Segmentierungsmodell (Blender)
│       ├── roboterarm_pose/       # Pose-Modell (Blender)
│       ├── roboterarm_real/       # Fine-Tuned Seg-Modell (echte Bilder)
│       └── roboterarm_pose_real/  # Fine-Tuned Pose-Modell (echte Bilder)
├── Edge Export/                   # Exportierte Modelle (ONNX)
├── yolo11n-seg.pt                 # Pretrained YOLO11 Nano Segmentation
└── yolo11n-pose.pt                # Pretrained YOLO11 Nano Pose
```

---

## Skripte im Detail

### 1. `EdgeAI.py` — KI-Training (4 Schritte)

Training in zwei Modi: **Virtuell** (Blender-Daten) und **Echt** (echte Kamerabilder).  
Jeder Modus trainiert Segmentierung + Pose nacheinander in einem Durchlauf.

```
python EdgeAI.py       # Menü
python EdgeAI.py 1     # Direkt Schritt 1
python EdgeAI.py 1 --opt       # Schritt 1 + Optimierungen
python EdgeAI.py 2 --standard  # Schritt 2 ohne Optimierungen
python EdgeAI.py 4 --quantize  # Schritt 4 + direkte INT8-Quantisierung (ohne Retraining)
```

| Schritt | Funktion | Beschreibung |
|---|---|---|
| [1] | `train_virtuell()` | **Seg + Pose** auf Blender-Daten |
| [2] | `train_echt()` | **Seg Fine-Tuning + Pose** auf echten Bildern |
| [3] | `validate_model()` | Validierung — erkennt automatisch Pose/Seg (Real/Blender) |
| [4] | `export_model()` | ONNX-Export des besten Modells |

Bei Schritt **[1]** und **[2]** kann nun zusätzlich ein Trainingsmodus gewählt werden:
- **Standard**: Verhalten wie bisher
- **Optimiert**: Nach dem Training wird optional eine zusätzliche Pipeline ausgeführt:
   - Structured Pruning + kurzes Recovery-Fine-Tuning
   - ONNX-Export + statische INT8-Quantisierung (ONNX Runtime, Kalibrierbilder)

CLI-Flags für Moduswahl:
- `--opt` erzwingt Optimiert
- `--standard` erzwingt Standard

CLI-Flags für Schritt **[4] Export**:
- `--quantize` exportiert und quantisiert direkt auf INT8 (ohne erneutes Training)
- `--no-quantize` nur normaler ONNX-Export (FP32)

#### Implementierte Optimierungslogik (in `EdgeAI.py`)

Wenn **Optimiert** aktiv ist, läuft nach dem normalen Training je Modell (Seg/Pose) diese Pipeline:

1. **Structured Pruning**
   - Methode: `torch.nn.utils.prune.ln_structured`
   - Auf Conv2d-Layern entlang Output-Kanälen (`dim=0`)
   - Standardwert: `PRUNE_RATIO = 0.20`

2. **Recovery-Fine-Tuning**
   - Kurzes Nachtraining des geprunten Modells
   - Standardwert: `PRUNE_RECOVERY_EPOCHS = 10`
   - Ergebnis-Runnamen:
     - Blender: `roboterarm_seg_opt`, `roboterarm_pose_opt`
     - Echt: `roboterarm_real_opt`, `roboterarm_pose_real_opt`

3. **INT8-Quantisierung (statisch, ONNX Runtime)**
   - Erst FP32-ONNX Export (`opset=20`, `simplify=True`)
   - Danach `quantize_static(...)` im QDQ-Format
   - Typen:
   - Activations: `QInt8`
     - Weights: `QInt8`
   - `per_channel=True`
   - Kalibrierung über bis zu `CALIBRATION_IMAGES = 64` Bilder aus dem jeweiligen Train-Ordner
   - Ausgabe im Ordner `Edge Export/` als `<run_name>_int8.onnx`

4. **Fallback-Verhalten**
   - Falls benötigte Pakete fehlen (z. B. `onnxruntime`) oder keine Kalibrierbilder vorhanden sind,
     wird die Optimierung sauber übersprungen und das Standardtraining bleibt nutzbar.

**Allgemeine Parameter:**
- Device: CPU (Intel Ultra7 155H — XPU nicht von ultralytics unterstützt)
- Batch: 16, Image Size: 640, Seed: 42
- Augmentation: Mosaic, HSV-Shift, Rotation, Scale (angepasst pro Modus)

---

### 2. `Umrisse_in_Polygone.py` — Datenvorbereitung (3 Schritte)

Datenvorbereitung in zwei Modi: **Virtuell** und **Echt**.  
Jeder Modus bereitet Seg- und Pose-Labels in einem Durchlauf auf.

```
python Umrisse_in_Polygone.py       # Menü
python Umrisse_in_Polygone.py 1     # Direkt Schritt 1
```

#### Schritt [1]: Virtuell — Daten vorbereiten

Führt nacheinander aus:

1. **`blender_masken_zu_labels()`** — Farbige Blender-Masken (`dataset/masks/mask_*.png`) → YOLO-Seg-Labels (`dataset/labels/`)  
   HSV-Farbraum-Segmentierung → Konturfindung → normalisierte Polygon-Koordinaten.  
   Rot benötigt zwei HSV-Bereiche (H: 0-10 und 170-179).

2. **`csv_zu_pose_labels()`** — Kopiert vorgenerierte Pose-Labels aus `dataset/labels_pose/` + Bilder aus `dataset/images/` nach `pose_dataset/` (Train/Val Split)  
   Aktuell **9 Keypoints** (inkl. zusätzlicher Base-/Joint1-Referenzpunkte für Z-Achsen-Rotation).

#### Zusatz: Keypoint-Kalibrierung in Blender

Für die Pose-Keypoints wird ein einzelnes Marker-Set verwendet (`KP_CAL_MARKER_*`).
Diese Marker werden im Blender-Viewport manuell auf die gewünschten Referenzpunkte gesetzt.
`keypoint_axis_calibration.py` berechnet daraus Offsets und rendert Kontroll-Previews.

#### Schritt [2]: Echt — Daten vorbereiten

Führt nacheinander aus:

1. **`auto_annotate()`** — Vortrainiertes Modell labelt echte Kamerabilder automatisch  
   Input: Bilder aus `Image_Capture_Python/dataset/` | Confidence ≥ 30%  
   **Wichtig:** Auto-Labels danach manuell prüfen (CVAT / Label Studio).

2. **`seg_zu_pose_labels_real()`** — Seg-Schwerpunkte → YOLO-Pose-Labels für echte Bilder  
   Nutzt das trainierte Seg-Modell als "Lehrer" zur Keypoint-Erzeugung.  
   Bei 9-Keypoint-Schema bleiben Zusatz-Referenzpunkte (z. B. `Base_Z1`, `Base_Z2`, `Joint_1_Z1`) in Pseudo-Labels typischerweise auf `0 0 0`, wenn sie aus Segmenten nicht eindeutig ableitbar sind.

#### Schritt [3]: Winkelberechnung

| Funktion | `winkel_berechnen()` |
|---|---|
| **Input** | Echte Kamerabilder + trainiertes YOLO-Modell |
| **Output** | Visualisierte Bilder mit Skeleton + Winkelwerten in `winkel_output/` |
| **Methode** | Pose → Keypoints direkt / Seg → Schwerpunkte → Richtungsvektoren → Gelenkwinkel |

**Algorithmus:**
1. Inferenz auf echten Bildern → Keypoints (Pose) oder Segmentierungsmasken (Seg)
2. **Pose:** Keypoints direkt auslesen (genauer) / **Seg:** Schwerpunkt jeder Maske berechnen
3. **Richtungsvektoren** entlang der Hauptkette `Base → Joint_1 → Joint_2 → Joint_3 → Joint_4 → TCP`
4. **Joint 1:** Winkel des Vektors `Base → Joint_1` zur Vertikalen (Referenz: oben = 0°)
5. **Joints 2–3:** Winkel zwischen aufeinanderfolgenden Richtungsvektoren (mit Vorzeichen)
6. **Joint 4:** Winkel zwischen `Joint_3 → Joint_4` und `Joint_4 → TCP` (Fallback bei Altmodellen: `Joint_2 → Joint_3` zu `Joint_3 → Joint_4`)
6. Vergleich mit Ground-Truth-Winkeln aus CSV, MAE-Berechnung

**Modellauswahl (automatisch):** Pose (Real) > Pose (Blender) > Fine-Tuned-Seg > Blender-Seg

---

### 3. `Validierung.ipynb` — Ergebnis-Visualisierung

Jupyter Notebook zur Auswertung des Trainings.

| Sektion | Inhalt |
|---|---|
| 1 | Training Loss Kurven (Box, Seg, Cls, DFL) |
| 2 | Mask-Metriken (mAP50, mAP50-95, Precision, Recall) |
| 3 | Box vs. Mask Metriken Vergleich |
| 4 | Confusion Matrix (absolut + normalisiert) |
| 5 | Precision-Recall Kurven |
| 6 | Trainingsbeispiele & Validierungsvorhersagen |
| 7 | Ground Truth vs. Predictions (Detailvergleich) |
| 8 | Inferenz auf synthetischem Bild |
| 8b | Inferenz auf echten Kamerabildern (max. 5) |
| 9 | Trainingszusammenfassung mit besten Metriken |

---

### 4. `label_studio_converter.py` — Import fuer Label Studio vorbereiten

Konvertiert YOLO-Labels in Label-Studio-Tasks, damit auto-annotierte Daten
direkt im UI kontrolliert und korrigiert werden koennen.

#### Ziel

- Segmentierungsdaten (`real_dataset`) als Polygon-Vorannotation importieren
- Pose-Daten (`real_pose_dataset`) als Keypoint-Vorannotation importieren
- Nach Korrektur in Label Studio wieder fuer Training nutzen

#### Erzeugte Dateien

Im Ordner `Training_Scripts/label_studio_import/`:

- `tasks_seg_all.json` (Anzahl abhängig vom aktuellen Datensatz)
- `tasks_pose_train.json` (Anzahl abhängig vom aktuellen Datensatz)
- `tasks_pose_val.json` (Anzahl abhängig vom aktuellen Datensatz)
- `label_config_seg.xml` (Projekt "Robot Segmentierung")
- `label_config_pose.xml` (Projekt "Robot Pose")

#### Empfohlener Schnellstart (alle Dateien auf einmal)

```powershell
cd C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts

python .\label_studio_converter.py prepare-all \
   --root . \
   --image-host http://localhost:3000 \
   --import-as annotations
```

Fuer Segmentierung per Pinsel statt Polygon:

```powershell
python .\label_studio_converter.py prepare-all \
   --root . \
   --image-host http://localhost:3000 \
   --import-as annotations \
   --seg-format brush
```

Damit werden in `label_studio_import/` erzeugt:
- `tasks_seg_all.json`
- `tasks_pose_train.json`
- `tasks_pose_val.json`
- `label_config_seg.xml`
- `label_config_pose.xml`

Im Brush-Modus stattdessen:
- `tasks_seg_all_brush.json`
- `label_config_seg_brush.xml`
- plus die gleichen Pose-Dateien wie oben

#### Ausgefuehrte Konvertierung

```powershell
cd C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts

python .\label_studio_converter.py seg \
   --images-dir .\real_dataset\images\all \
   --labels-dir .\real_dataset\labels\all \
   --image-url-prefix http://localhost:3000/real_dataset/images/all \
   --output .\label_studio_import\tasks_seg_all.json

python .\label_studio_converter.py seg \
   --images-dir .\real_dataset\images\all \
   --labels-dir .\real_dataset\labels\all \
   --image-url-prefix http://localhost:3000/real_dataset/images/all \
   --output .\label_studio_import\tasks_seg_all_brush.json \
   --import-as annotations \
   --seg-format brush \
   --from-name brush

python .\label_studio_converter.py pose \
   --images-dir .\real_pose_dataset\images\train \
   --labels-dir .\real_pose_dataset\labels\train \
   --image-url-prefix http://localhost:3000/real_pose_dataset/images/train \
   --output .\label_studio_import\tasks_pose_train.json \
   --from-name kp

python .\label_studio_converter.py pose \
   --images-dir .\real_pose_dataset\images\val \
   --labels-dir .\real_pose_dataset\labels\val \
   --image-url-prefix http://localhost:3000/real_pose_dataset/images/val \
   --output .\label_studio_import\tasks_pose_val.json \
   --from-name kp
```

#### Import in Label Studio

1. Lokalen Webserver im Ordner `Training_Scripts` starten (mit CORS), damit
   die Bild-URLs erreichbar sind:

```powershell
cd C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts
python .\serve_with_cors.py --host 127.0.0.1 --port 3000
```

2. Projekt **Segmentierung** (z. B. Projekt-ID 4):
   - Polygon-Workflow: Labeling Config aus `label_config_seg.xml` setzen und `tasks_seg_all.json` importieren
   - Pinsel-Workflow: Labeling Config aus `label_config_seg_brush.xml` setzen und `tasks_seg_all_brush.json` importieren

3. Projekt **Pose** (z. B. Projekt-ID 3):
   - Labeling Config aus `label_config_pose.xml` setzen
   - `tasks_pose_train.json` und `tasks_pose_val.json` importieren

#### Wichtige Details

- Segmentierungs-Tasks verwenden `from_name="label"` + `type="polygonlabels"`
- Pose-Tasks verwenden `from_name="kp"` + `type="keypointlabels"`
- YOLO-Koordinaten (0..1) werden fuer Label Studio auf Prozent (0..100) umgerechnet
- Klassen-Mapping bleibt konsistent: `Base`, `Joint_1`, `Joint_2`, `Joint_3`, `Joint_4_Finger`

#### Optional: Direkter API-Upload ins Projekt

Der Konverter kann die erzeugten Tasks direkt in ein Label-Studio-Projekt hochladen.

```powershell
$env:LABEL_STUDIO_URL="http://localhost:8080"
$env:LABEL_STUDIO_API_KEY="<DEIN_TOKEN>"

python .\label_studio_converter.py seg \
   --images-dir .\real_dataset\images\all \
   --labels-dir .\real_dataset\labels\all \
   --image-url-prefix http://localhost:3000/real_dataset/images/all \
   --output .\label_studio_import\tasks_seg_all_annotations.json \
   --import-as annotations \
   --upload --project-id <SEG_PROJEKT_ID>

python .\label_studio_converter.py pose \
   --images-dir .\real_pose_dataset\images\train \
   --labels-dir .\real_pose_dataset\labels\train \
   --image-url-prefix http://localhost:3000/real_pose_dataset/images/train \
   --output .\label_studio_import\tasks_pose_train_annotations.json \
   --from-name kp --import-as annotations \
   --upload --project-id <POSE_PROJEKT_ID>
```

Hinweise:

- `--auth-scheme auto` ist Standard und testet `Bearer` und `Token` automatisch.
- Fuer direkt editierbare Vorlabels `--import-as annotations` verwenden.
- Fuer reine Modellvorschlaege `--import-as predictions` verwenden.

#### Troubleshooting (URL/Bild wird nicht geladen)

Wenn Label Studio meldet:
`There was an issue loading URL from $image value`

Pruefen:

1. Bildserver laeuft wirklich auf Port 3000.
2. URL ist im Browser direkt erreichbar, z. B.:
   `http://localhost:3000/real_dataset/images/all/pose_0001.png`
3. CORS ist aktiv (`Access-Control-Allow-Origin: *`).
4. Nach Serverstart Label-Studio-Seite mit `Ctrl+F5` neu laden.

Hinweis: `python -m http.server 3000` liefert je nach Setup nicht immer die
noetigen CORS-Header. Deshalb `serve_with_cors.py` verwenden.

---

### 5. `npzConverter.py` — Kalibrierungsdaten fuer STM32Cube.AI Studio

Erzeugt eine `calibration.npz` direkt aus Bilddaten (ohne Labels), fuer
`--valinput` in STM32Cube.AI Studio / ST Edge AI.

#### Was das Skript erzeugt

- Schluessel in der NPZ: `input`
- Datentyp: `float32`
- Tensor-Layout: `(N, 3, H, W)` (NCHW)
- Vorverarbeitung: YOLO-Letterbox (Padding 114), BGR->RGB, optional `img/255.0`

#### Wichtiger Hinweis zur Form

Die Bildgroesse muss zur Modell-Eingabe passen. Fuer das aktuelle ONNX-Modell
ist das `(-1, 3, 640, 640)`, daher `--imgsz 640` verwenden.

#### Beispiel: Echter Datensatz ohne Labels

```powershell
python "C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts\npzConverter.py" \
   --images "C:\Dev\Image_Capture_Python\dataset\images\angepasstes_seitenv_20260311_170850" \
   --out "C:\Dev\Image_Capture_Python\dataset\images\angepasstes_seitenv_20260311_170850\calibration.npz" \
   --imgsz 640 \
   --count 200 \
   --norm_to_1
```

#### Weitere Optionen

- Alle verfuegbaren Bilder nutzen: `--count 999999`
- Zufaellig mischen vor Sampling: `--shuffle --seed 42`
- Letterbox deaktivieren (nicht empfohlen): `--no_letterbox`

---

## Trainings-Workflow

```
╔═══════════════════════════════════════════════════════╗
║                  VIRTUELL (Blender)                    ║
╠═══════════════════════════════════════════════════════╣
║                                                       ║
║  1. Blender Simulation          → 500 Bilder/Masken   ║
║                                                       ║
║  2. Daten vorbereiten           → Umrisse_in_Polygone  ║
║     (Masken→Seg-Labels            .py [1]              ║
║      + Pose-Labels kopieren)                           ║
║                                                       ║
║  3. Virtuell trainieren         → EdgeAI.py [1]        ║
║     (Seg + Pose nacheinander)                          ║
║                                                       ║
╚══════════════════════╦════════════════════════════════╝
                       │
                       ▼
╔═══════════════════════════════════════════════════════╗
║                   ECHT (Kamera)                        ║
╠═══════════════════════════════════════════════════════╣
║                                                       ║
║  4. Daten vorbereiten           → Umrisse_in_Polygone  ║
║     (Auto-Annotation +            .py [2]              ║
║      Seg→Pose-Labels)                                  ║
║                                                       ║
║  5. Labels korrigieren          → CVAT / Label Studio  ║
║                                                       ║
║  6. Echt trainieren             → EdgeAI.py [2]        ║
║     (Seg + Pose nacheinander)                          ║
║                                                       ║
╚══════════════════════╦════════════════════════════════╝
                       │
                       ▼
┌───────────────────────────────────────────────────────┐
│  7. Validierung & Export        → EdgeAI.py [3] + [4]  │
├───────────────────────────────────────────────────────┤
│  8. Winkelberechnung            → Umrisse_in_Polygone  │
│     (Pose/Seg → Gelenkwinkel)     .py [3]              │
└───────────────────────────────────────────────────────┘
```

---

## Datensätze

| Datensatz | Bilder | Quelle | Zweck |
|---|---|---|---|
| Blender Synthetisch | 500 | Blender Simulation | Pre-Training (Formen lernen) |
| Echte Kamerabilder | 500 | STM32 Kamera | Fine-Tuning (Realwelt-Transfer) |

Die echten Bilder enthalten zusätzlich Gelenkwinkel in der CSV-Datei (`labels_avc_*.csv`) mit 4 Joints in Grad und Radiant.

Die Blender-Daten enthalten `joint_data.csv` mit Gelenkwinkeln (angle_j1–j4). Die Pose-Labels (6 Keypoints: Base + Joint_1-4 + TCP) liegen vorgeneriert im Ordner `dataset/labels_pose/` im YOLO-Format.

---

## Abhängigkeiten

```
ultralytics        # YOLO Training & Inferenz
opencv-python      # Bildverarbeitung, Konturfindung
numpy              # Array-Operationen
matplotlib         # Visualisierung (Notebook)
pandas             # CSV-Daten laden (Notebook, Winkelberechnung)
label-studio       # Manuelle Annotation echter Bilder
```
