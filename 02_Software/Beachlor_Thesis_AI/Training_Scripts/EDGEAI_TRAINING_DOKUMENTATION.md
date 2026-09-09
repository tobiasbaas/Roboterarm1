# EdgeAI.py — Detaillierte Dokumentation

## 📋 Übersicht

Das Skript **`EdgeAI.py`** ist das **zentrale KI-Training-System**. Es koordiniert das gesamte Training von YOLO-Modellen (Segmentierung + Pose) in zwei Betriebsmodi:

- **[1] Virtuell:** Trainiert auf Blender-Daten (Pre-Training)
- **[2] Echt:** Fine-Tuning auf echten Kamerabildern
- **[3] Validierung:** Testet die trainierten Modelle
- **[4] Export:** Exportiert zu ONNX + optionale INT8-Quantisierung

Das Skript kann in **zwei verschiedenen Trainingsmodi** laufen:
- **Standard:** Normales Training ohne weitere Optimierungen
- **Optimiert:** Training + Pruning + Recovery + INT8-Quantisierung

---

## 🎯 Trainings-Workflows

### Workflow 1: Virtuell (Blender-Daten)

```
Blender-Daten
├── dataset/images (RGB)
├── dataset/labels (Segmentierungsmasken)
└── pose_dataset (Pose-Labels + YAML)
    ↓
[1] EdgeAI.py 1
├─ Phase 1: Train Seg-Modell (roboterarm_seg)
└─ Phase 2: Train Pose-Modell (roboterarm_pose)
    ↓
Modelle: Trained_Models/AI-Models/roboterarm_seg|pose/
```

### Workflow 2: Echt (Echte Daten)

```
Echte Kamerabilder
├── real_dataset/images/all
├── real_dataset/labels/all
└── real_pose_dataset (Pose-Labels)
    ↓
[2] EdgeAI.py 2
├─ Phase 1: Fine-Tune Seg (roboterarm_real)
└─ Phase 2: Train Pose (roboterarm_pose_real)
    ↓
Modelle: Trained_Models/AI-Models/roboterarm_real|pose_real/
```

### Workflow 3 & 4: Validierung + Export

```
Trainierte Modelle
    ↓
[3] EdgeAI.py 3 → Validierungsmetriken
    ↓
[4] EdgeAI.py 4 → ONNX-Export (+ optional INT8)
    ↓
Edge Export/
```

---

## 🔧 Konfiguration und Konstanten

### Device-Auswahl

```python
DEVICE = _auto_select_device()
# Automatisch: GPU (CUDA) > CPU
# Override: YOLO_DEVICE=cpu python EdgeAI.py
```

**Priorität:**
1. Umgebungsvariable `YOLO_DEVICE`
2. PyTorch CUDA (torch.cuda.is_available())
3. nvidia-smi (Fallback-Prüfung)
4. CPU (Standard)

### Training-Parameter

```python
IMG_SIZE = 640              # Bildgröße
BATCH_SIZE = 16             # Batch-Größe pro GPU
VAL_SPLIT = 0.3             # 30% Validierung, 70% Training
SEED = 42                   # Reproduzierbarkeit
```

### Optimierungs-Parameter

```python
PRUNE_RATIO = 0.20                  # Pruning: 20% der Kanäle
PRUNE_RECOVERY_EPOCHS = 20          # Recovery-Training nach Pruning
CALIBRATION_IMAGES = 64             # Bilder für INT8-Quantisierung
```

### Verzeichnisse

```python
SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent

# Blender-Daten
DATASET_DIR = SCRIPT_DIR / "Segmentierung per Blender" / "dataset"
YOLO_DATASET_DIR = SCRIPT_DIR / "Segmentierung per Blender" / "yolo_dataset"
POSE_DATASET_DIR = SCRIPT_DIR / "Segmentierung per Blender" / "pose_dataset"

# Echte Daten
REAL_DATASET_DIR = SCRIPT_DIR / "real_dataset"
REAL_POSE_DATASET_DIR = SCRIPT_DIR / "real_pose_dataset"

# Modelle und Export
MODEL_OUTPUT_DIR = PROJECT_ROOT / "Trained_Models" / "AI-Models"
EXPORT_DIR = PROJECT_ROOT / "Edge Export"
RUNS_INDEX_FILE = MODEL_OUTPUT_DIR / "runs_index.json"
```

---

# 🔍 Device-Auswahl

## `_auto_select_device() -> str`

```python
def _auto_select_device() -> str:
    """Bevorzugt NVIDIA-GPU (CUDA), ansonsten CPU."""
```

### Priorität (der Reihe nach geprüft):

**1. Umgebungsvariable**
```python
forced_device = os.getenv("YOLO_DEVICE", "").strip()
if forced_device:
    return forced_device
```

**Beispiel:**
```bash
YOLO_DEVICE=cpu python EdgeAI.py 1  # Erzwingt CPU
YOLO_DEVICE=cuda:0 python EdgeAI.py 1  # GPU 0
```

**2. PyTorch CUDA**
```python
import torch
if torch.cuda.is_available() and torch.cuda.device_count() > 0:
    gpu_name = torch.cuda.get_device_name(0)
    return "cuda:0"
```

**3. nvidia-smi Fallback**
```bash
nvidia-smi --query-gpu=name --format=csv,noheader
```

**4. CPU Default**
```python
return "cpu"
```

### Output
```python
# Console:
# NVIDIA GPU erkannt: NVIDIA A100 -> nutze cuda:0
# oder:
# Keine NVIDIA-GPU erkannt -> nutze CPU
```

---

# 📁 YAML und Pfad-Verwaltung

## `_path_for_yaml(target: Path, yaml_file: Path) -> str`

```python
def _path_for_yaml(target: Path, yaml_file: Path) -> str:
    """Nutze relative Pfade in YAML, damit Projekte portabel bleiben."""
```

**Zweck:** Konvertiert absolute Pfade in relative, damit das Projekt auf verschiedenen Computern funktioniert.

**Beispiel:**
```python
target = Path("/data/dataset/images")
yaml_file = Path("/data/dataset.yaml")

# Ergebnis: "images" (relativ zu /data/)
```

---

## `_yaml_with_absolute_dataset_path(yaml_path: Path) -> Path`

```python
def _yaml_with_absolute_dataset_path(yaml_path: Path) -> Path:
    """Erzeugt eine temporäre YAML mit absolutem `path:` für Ultralytics."""
```

### Zweck
YOLO braucht manchmal absolute Pfade. Dieses Skript:
1. Liest original YAML
2. Ersetzt `path:` mit absolutem Pfad
3. Speichert temporäre Version in `.ultralytics_tmp/`
4. Gibt Pfad zur temp-YAML zurück

**Warum?** Ultralytics kann manchmal mit relativen Pfaden problematisch sein.

---

## `_normalize_yolo_label_file(src: Path, dst: Path)`

```python
def _normalize_yolo_label_file(src: Path, dst: Path) -> None:
    """Schreibt YOLO-Labels mit Dezimalpunkt statt Dezimalkomma."""
```

**Zweck:** Auf manchen Computern wird das Dezimalkomma verwendet (z.B. Deutschland). Dies konvertiert zu Dezimalpunkt, den YOLO erwartet.

**Beispiel:**
```
Input:  0,50 0,25 0,30
Output: 0.50 0.25 0.30
```

---

# 📊 Run-Management und Versionierung

Das Skript verwaltet mehrere Modell-Durchläufe (`runs`) mit Versionierung.

## `_load_runs_index() -> dict`

```python
def _load_runs_index() -> dict:
    """Lädt den Lauf-Index (neueste Runs + Historie) aus JSON."""
```

**Format:** `Trained_Models/AI-Models/runs_index.json`

```json
{
  "latest": {
    "roboterarm_seg": "roboterarm_seg_v001_20260416",
    "roboterarm_pose": "roboterarm_pose_v001_20260416",
    "roboterarm_real": "roboterarm_real_v001_20260417"
  },
  "history": {
    "roboterarm_seg": [
      "roboterarm_seg_v001_20260416",
      "roboterarm_seg_v002_20260417"
    ]
  }
}
```

---

## `_create_versioned_run_name(base_name: str) -> str`

```python
def _create_versioned_run_name(base_name: str) -> str:
    """Erzeugt Run-Namen mit Version + Datum, z.B. roboterarm_pose_v003_20260322."""
```

**Format:** `{base_name}_v{version:03d}_{date}`

**Beispiele:**
```
roboterarm_seg_v001_20260416
roboterarm_pose_v002_20260416
roboterarm_real_v001_20260417
```

**Versionierung:** Automatisch inkrementiert basierend auf vorherigen Runs.

---

## `_best_weights_for_base(base_name: str) -> Optional[Path]`

```python
def _best_weights_for_base(base_name: str, fallback: Optional[Path] = None) -> Optional[Path]:
    """Liefert best.pt für das neueste versionierte Basismodell (mit Fallback)."""
```

**Logik:**
1. Suche neuesten Run im Index
2. Falls nicht: suche nach Dateimodifikationszeit
3. Fallback: angegeben Pfad (z.B. Blender-Modell)

**Beispiel:**
```python
# Neuestes roboterarm_real Modell
path = _best_weights_for_base("roboterarm_real")
# → roboterarm_real_v001_20260417/weights/best.pt
```

---

# 🚀 Optimierungs-Pipeline

Das Skript bietet eine optionale Optimierungs-Pipeline (Pruning + Quantisierung).

## `_apply_structured_pruning_and_recover(...) -> Optional[Path]`

```python
def _apply_structured_pruning_and_recover(
    source_weights: Path,
    data_yaml: Path,
    output_run_name: str,
    prune_ratio: float = PRUNE_RATIO,
    recovery_epochs: int = PRUNE_RECOVERY_EPOCHS,
) -> Optional[Path]:
    """Pruned Conv-Kanäle (structured) und trainiert kurz zur Erholung nach."""
```

### Was ist Pruning?

Pruning = **Entfernen von unwichtigen Neuronen** aus dem Modell.

```
Original-Modell: 1000 Conv-Kanäle
    ↓
Pruning (20%): Entferne 200 unwichtige Kanäle
    ↓
Gepruntes Modell: 800 Conv-Kanäle
    ↓
Recovery-Training: Kurzes Training um Qualität wiederherzustellen
    ↓
Kleineres, schnelleres Modell (80% der Größe, ~95% der Qualität)
```

### Prozess

**1. Lade Modell**
```python
model = YOLO(str(source_weights))
```

**2. Finde alle Conv2d-Layer**
```python
for module in model.model.modules():
    if isinstance(module, torch.nn.Conv2d):
        prune.ln_structured(module, name="weight", amount=prune_ratio, n=2, dim=0)
        prune.remove(module, "weight")
```

`ln_structured`: Pruning entlang Output-Kanäle (dim=0)  
`amount=0.20`: 20% der Kanäle entfernen

**3. Recovery-Training**
```python
model.train(
    data=str(data_yaml_abs),
    epochs=recovery_epochs,  # Z.B. 20 Epochen
    lr0=0.0005,  # Niedriger Learning-Rate (wir starten vom trainierten Modell)
    mosaic=0.0,  # Keine Augmentation (zu disruptiv)
    patience=max(5, recovery_epochs // 2),
)
```

Niedriger Learning-Rate + kurzes Training = Modell erholt sich schnell.

### Output

```
Pruning auf 34 Conv-Layern angewendet.
Recovery-Fine-Tuning startet...
Pruning+Recovery abgeschlossen: roboterarm_seg_opt_v001.../weights/best.pt
```

---

## `_export_int8_onnx(source_weights: Path, calibration_dir: Path, export_name: str) -> Optional[Path]`

```python
def _export_int8_onnx(source_weights: Path, calibration_dir: Path, export_name: str) -> Optional[Path]:
    """Exportiert ONNX und quantisiert mit ONNX Runtime auf INT8 (static)."""
```

### Was ist INT8-Quantisierung?

Quantisierung = **Reduziert Modell-Präzision** von 32-Bit (Float) auf 8-Bit (Integer).

```
Float32: 4 Bytes pro Wert, hohe Präzision
    ↓
INT8: 1 Byte pro Wert, 4× kleiner, etwas niedrigere Präzision
    ↓
Modell-Größe: 4× kleiner
Inferenz-Geschwindigkeit: 2-3× schneller (auf spezialisierter Hardware)
```

### Prozess

**1. Exportiere zu ONNX (Float32)**
```python
model = YOLO(str(source_weights))
onnx_fp_path = Path(model.export(format="onnx", imgsz=IMG_SIZE, opset=20, simplify=True))
```

**2. Lade Kalibrierungsbilder**
```python
images = []
for ext in ("*.png", "*.jpg", "*.jpeg", "*.bmp"):
    images.extend(sorted(calibration_dir.glob(ext)))
images = images[:CALIBRATION_IMAGES]  # Maximal 64 Bilder
```

Kalibrierungsbilder = **repräsentative Bilder** zur Bestimmung von INT8-Wertebereichen.

**3. Kalibrierungsleser (CalibrationDataReader)**
```python
class _Reader(CalibrationDataReader):
    def _preprocess(self, path):
        img = cv2.imread(str(path))
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        img = cv2.resize(img, (IMG_SIZE, IMG_SIZE))
        img = img.astype(np.float32) / 255.0
        img = np.transpose(img, (2, 0, 1))
        return np.expand_dims(img, axis=0)
    
    def get_next(self):
        # Liefert nächstes Bild als Tensor
```

**4. Quantisiere**
```python
quantize_static(
    model_input=str(onnx_fp_path),
    model_output=str(quant_out),
    calibration_data_reader=reader,
    quant_format=QuantFormat.QDQ,
    activation_type=QuantType.QInt8,
    weight_type=QuantType.QInt8,
    per_channel=True,
    calibrate_method=CalibrationMethod.MinMax,
)
```

**QDQ Format:** Quantize-Dequantize (lässt Wertebereiche eingebettet)  
**per_channel:** Pro Kanal unterschiedliche Wertebereiche (präziser)  
**MinMax:** Kalibrierungsmethode

### Output

```
FP-ONNX: /path/to/model.onnx (z.B. 100 MB)
INT8-ONNX: /path/to/model_int8.onnx (z.B. 25 MB)
```

---

## `_run_optimization_pipeline(...) -> None`

```python
def _run_optimization_pipeline(
    model_name: str,
    source_weights: Path,
    data_yaml: Path,
    calibration_dir: Path,
    optimized_run_name: str,
) -> None:
    """Führt optionales Pruning + INT8-Export für ein trainiertes Modell aus."""
```

**Orchestriert zwei Schritte:**
1. Pruning + Recovery-Training
2. ONNX-Export + INT8-Quantisierung

```
Trainiertes Modell (best.pt, ~50 MB)
    ↓
[1] Pruning + Recovery (20% Kanäle entfernt)
    ↓
Gepruntes Modell (~40 MB)
    ↓
[2] ONNX-Export + INT8-Quantisierung
    ↓
INT8-ONNX (~10 MB)
```

---

# 🎓 Trainings-Funktionen

## Mode 1: Virtuelles Training

### `_prepare_seg_dataset()`

```python
def _prepare_seg_dataset():
    """Erstellt YOLO-Ordnerstruktur und verteilt Bilder + Labels in Train/Val."""
```

**Input:**
```
dataset/
├── images/        (oder dataset/*.png)
└── labels/        (oder dataset/*.txt)
```

**Output:**
```
yolo_dataset/
├── images/
│   ├── train/  (70%)
│   └── val/    (30%)
└── labels/
    ├── train/  (70%)
    └── val/    (30%)
```

**Prozess:**
1. Bilder von `dataset/images` sammeln
2. Dazugehörige Labels finden
3. Zufällig in 70/30 aufteilen (mit Seed)
4. In YOLO-Struktur kopieren

---

### `_train_seg_blender()`

```python
def _train_seg_blender():
    """Trainiert YOLO-Seg auf Blender-Daten."""
```

**Vorbereitung:**
```python
_prepare_seg_dataset()
```

**Modell:**
```python
model = YOLO(str(YOLO11N_SEG))  # YOLO11n (Nano)
```

**Training-Parameter:**
```python
model.train(
    epochs=100,
    imgsz=IMG_SIZE,            # 640
    batch=BATCH_SIZE,          # 16
    device=DEVICE,
    seed=SEED,
    # Augmentation für Robustheit
    hsv_h=0.01, hsv_s=0.3, hsv_v=0.3,
    degrees=15.0, translate=0.1, scale=0.3,
    fliplr=0.5, flipud=0.0,
    mosaic=0.5,
    patience=20,  # Early Stopping
)
```

**Output:**
```
Trained_Models/AI-Models/roboterarm_seg_v001_20260416/
├── weights/
│   ├── best.pt      ← Bestes Modell
│   └── last.pt
├── results.csv
└── ...
```

---

### `_train_pose_blender()`

```python
def _train_pose_blender():
    """Trainiert YOLO-Pose auf Blender-Daten."""
```

**Besonderheit:** Konservativere Augmentation für Keypoints

```python
model.train(
    epochs=100,
    # Weniger aggressive Augmentation
    degrees=8.0, translate=0.05, scale=0.2,
    fliplr=0.0,        # Keine Horizontal-Flip (würde Keypoints verwirren)
    mosaic=0.0,        # Kein Mosaic-Augmentation
    patience=20,
)
```

**Warum konservativ?** Keypoints sind sensitiv — zu aggressive Augmentation kann Qualität verschlechtern.

---

## Mode 2: Echtes Training (Fine-Tuning)

### `_finetune_seg_real()`

```python
def _finetune_seg_real():
    """Fine-Tuning: Blender-Seg-Modell auf echten Daten."""
```

**Eingangsmodell:** Blender-trainiertes `roboterarm_seg` (Transfer Learning)

```python
pretrained_seg = _best_weights_for_base("roboterarm_seg", PRETRAINED_SEG)
model = YOLO(str(pretrained_seg))
```

**Fine-Tuning-Strategie:**
```python
model.train(
    epochs=100,
    freeze=10,          # Friere erste 10 Layer ein (Blender-Wissen behalten)
    lr0=0.001,          # Niedrigerer Learning-Rate (fine-tuning)
    lrf=0.01,
    # Aggressive Augmentation weil reale Bilder schwieriger
    hsv_h=0.015, hsv_s=0.5, hsv_v=0.4,
    degrees=10.0, translate=0.15, scale=0.4,
    fliplr=0.5, flipud=0.0,
    mosaic=0.8, mixup=0.1,
    patience=15,
)
```

**Unterschied zu Blender-Training:**
- **freeze=10:** Erste 10 Layer nicht trainieren (Blender-Features behalten)
- **lr0=0.001:** Viel niedrigerer LR (statt 0.01)
- **Aggressivere Augmentation:** Reale Daten sind vielfältiger

---

### `_train_pose_real()`

```python
def _train_pose_real():
    """Trainiert YOLO-Pose auf echten Bildern."""
```

**Mit oder ohne Blender-Modell:**

```python
pretrained_pose = _best_weights_for_base("roboterarm_pose", PRETRAINED_POSE)

if pretrained_pose is not None:
    # Transfer Learning von Blender
    model = YOLO(str(pretrained_pose))
    freeze = 10
    lr0 = 0.001
else:
    # Von Grund auf trainieren
    model = YOLO(str(YOLO11N_POSE))
    freeze = 0
    lr0 = 0.01
```

---

# ✅ Validierung und Export

## `validate_model()`

```python
def validate_model():
    """[3] Validiert das beste verfügbare Modell."""
```

**Priorität (testet diese Modelle in der Reihenfolge):**
1. Pose (Real, Opt)
2. Pose (Real)
3. Pose (Blender, Opt)
4. Pose (Blender)
5. Seg (Real, Opt)
6. Seg (Real)
7. Seg (Blender, Opt)
8. Seg (Blender)

**Für jedes Modell:**
```python
model = YOLO(str(path))
metrics = model.val(data=str(yaml_abs))

# Metriken extrahieren
if hasattr(metrics, 'seg') and metrics.seg is not None:
    print(f"mAP50 (Mask): {metrics.seg.map50:.4f}")
elif hasattr(metrics, 'pose') and metrics.pose is not None:
    print(f"mAP50 (Pose): {metrics.pose.map50:.4f}")
```

---

## `export_model(quantize_override: Optional[bool] = None)`

```python
def export_model(quantize_override: Optional[bool] = None):
    """[4] Exportiert nur Real-Modelle (Seg + Pose), optional mit INT8-Quantisierung."""
```

**Priorität für Export:**
1. Optimierte Real-Modelle (falls vorhanden)
2. Real-Modelle (Fallback)

**Prozess:**
```python
for name, path in export_jobs:
    model = YOLO(str(path))
    
    # 1. FP32-ONNX
    exported_path = Path(model.export(format="onnx", imgsz=IMG_SIZE, opset=20))
    
    # 2. Kopiere zu Edge Export
    shutil.copy2(exported_path, EXPORT_DIR / exported_path.name)
    
    # 3. Optional: INT8-Quantisierung
    if do_quantize:
        _export_int8_onnx(path, calibration_dir, run_name)
```

---

# 🚀 Praktische Beispiele

## Beispiel 1: Normales Virtuelles Training

```bash
python EdgeAI.py 1
# oder
python EdgeAI.py 1 --standard

# Menü:
# [1] Virtuell trainieren (Seg + Pose)
# [2] Echt trainieren (Seg + Pose)
# [3] Validierung
# [4] Export (ONNX)
# 
# Schritt wählen (1-4): 1
# Device: cuda:0
# 
# Trainingsmodus:
#   [1] Standard (wie bisher)
#   [2] Optimiert (Structured Pruning + INT8 ONNX)
# Modus wählen (1/2, Enter=1): 1
# Optimierungsmodus: AUS
#
# === VIRTUELL TRAINIEREN ===
# 
# SEGMENTIERUNG — Blender-Daten
# Gefunden: 1800 Bilder
# Training wird gestartet...
# Epoch 1/100: loss=2.34...
# ...
# Epoch 100/100: loss=0.12... (Best: 0.11)
#
# POSE — Blender-Daten
# Dataset: 1260 Train, 540 Val
# Training wird gestartet...
# Epoch 1/100: loss=1.45...
# ...
# Epoch 100/100: loss=0.08... (Best: 0.07)
#
# === VIRTUELL KOMPLETT! ===
# Seg:  Trained_Models/AI-Models/roboterarm_seg_v001_20260416
# Pose: Trained_Models/AI-Models/roboterarm_pose_v001_20260416
```

---

## Beispiel 2: Optimiertes Training mit Pruning + Quantisierung

```bash
python EdgeAI.py 1 --opt

# Trainingsmodus: AUS (weil --opt gesetzt)
# Optimierungsmodus: AN
#
# === VIRTUELL TRAINIEREN (Seg + Pose) ===
# (normales Training)
# ...
# 
# === OPTIMIERUNG: Seg (Blender) ===
# Pruning startet: roboterarm_seg_v001.../weights/best.pt (ratio=0.20)
# Pruning auf 34 Conv-Layern angewendet.
# Recovery-Fine-Tuning startet...
# Epoch 1/20: loss=1.23...
# Epoch 20/20: loss=1.01... (Best: 1.00)
# Pruning+Recovery abgeschlossen: roboterarm_seg_opt_v001.../weights/best.pt
#
# ONNX-Export + INT8-Quantisierung: roboterarm_seg_opt_v001.../weights/best.pt
# FP-ONNX: .ultralytics_tmp/roboterarm_seg_opt_v001.onnx
# INT8-ONNX: Edge Export/roboterarm_seg_opt_v001_int8.onnx
#
# (ähnlich für Pose)
```

---

## Beispiel 3: Echt trainieren (Fine-Tuning)

```bash
python EdgeAI.py 2

# Trainingsmodus:
#   [1] Standard (wie bisher)
#   [2] Optimiert (Structured Pruning + INT8 ONNX)
# Modus wählen (1/2, Enter=1): 1
# Optimierungsmodus: AUS
#
# === ECHT TRAINIEREN ===
#
# SEGMENTIERUNG — Echte Bilder (Fine-Tuning)
# Starte von Blender-Seg-Modell: Trained_Models/AI-Models/roboterarm_seg_v001.../weights/best.pt
# Bilder mit Labels: 150
# Train: 105, Val: 45
# Training wird gestartet...
# (erste 10 Layer gefroren, niedriger LR)
# ...
#
# POSE — Echte Bilder
# Dataset: 100 Train, 43 Val
# Training wird gestartet...
# ...
#
# === ECHT KOMPLETT! ===
# Seg:  Trained_Models/AI-Models/roboterarm_real_v001_20260417/weights/best.pt
# Pose: Trained_Models/AI-Models/roboterarm_pose_real_v001_20260417/weights/best.pt
```

---

## Beispiel 4: Validierung

```bash
python EdgeAI.py 3

# === VALIDIERUNG ===
# Validiere: Pose (Real) (...)
# Epochs: 1/1
# ...
# 
# === Pose (Real) — Validierungsergebnisse ===
# mAP50 (Box):     0.8234
# mAP50 (Pose):    0.7891
# mAP50-95 (Pose): 0.6234
```

---

## Beispiel 5: Export mit Quantisierung

```bash
python EdgeAI.py 4 --quantize

# === EXPORT ===
# Exportiere: Seg (Real, Opt) (...)
# ONNX erzeugt: .ultralytics_tmp/roboterarm_real_opt_v001.onnx
# ONNX exportiert nach: Edge Export/roboterarm_real_opt_v001.onnx
#
# ONNX-Export + INT8-Quantisierung: roboterarm_real_opt_v001.../weights/best.pt
# FP-ONNX: .ultralytics_tmp/roboterarm_real_opt_v001.onnx
# INT8-ONNX: Edge Export/roboterarm_real_opt_v001_int8.onnx
#
# (ähnlich für Pose)
```

---

# 📊 Modi und Flags

## Training-Modi: Standard vs. Optimiert

| Aspekt | **Standard** | **Optimiert** |
|--------|-----------|--------------|
| Training | ✅ YOLO Training | ✅ YOLO Training |
| Pruning | ❌ | ✅ 20% der Kanäle entfernt |
| Recovery | ❌ | ✅ 20 Epochen Fine-Tuning |
| ONNX-Export | ❌ | ✅ FP32 |
| INT8-Quantisierung | ❌ | ✅ 8-Bit |
| Modellgröße | 100% | ~25% |
| Inferenz-Speed | 1× | ~2-3× schneller |
| Genauigkeit | 100% | ~95-98% |

### CLI-Flags

```bash
# Modus wählen per Flag
python EdgeAI.py 1 --standard    # Standard
python EdgeAI.py 1 --opt         # Optimiert

# Export-Quantisierung
python EdgeAI.py 4 --quantize    # Mit INT8
python EdgeAI.py 4 --no-quantize # Nur FP32
```

### Interaktiv

```bash
# Ohne Flag: Menü-Abfrage
python EdgeAI.py 1
# Trainingsmodus:
#   [1] Standard (wie bisher)
#   [2] Optimiert (Structured Pruning + INT8 ONNX)
# Modus wählen (1/2, Enter=1): 
```

---

# 🎯 Modell-Übersicht

## Blender-Modelle

| Modell | Base | Trainingsdaten | Optimiert |
|--------|------|----------------|-----------|
| `roboterarm_seg` | YOLO11n | Blender Seg-Labels | `roboterarm_seg_opt` |
| `roboterarm_pose` | YOLO11n | Blender Pose-Labels | `roboterarm_pose_opt` |

## Real-Modelle (Fine-Tuned)

| Modell | Base | Trainingsdaten | Optimiert |
|--------|------|----------------|-----------|
| `roboterarm_real` | roboterarm_seg | Echte Seg-Labels | `roboterarm_real_opt` |
| `roboterarm_pose_real` | roboterarm_pose | Echte Pose-Labels | `roboterarm_pose_real_opt` |

---

# ⚠️ Häufige Fehler & Lösungen

| Fehler | Ursache | Lösung |
|--------|--------|--------|
| `CUDA out of memory` | BATCH_SIZE zu groß | Reduziere BATCH_SIZE (default 16 → 8) |
| `Keine NVIDIA-GPU erkannt` | GPU nicht richtig erkannt | `YOLO_DEVICE=cuda:0 python EdgeAI.py 1` |
| `FileNotFoundError: dataset/images` | Blender-Datengenerierung nicht ausgeführt | Erst `Blender_Segment_Keypoint_Generation.py` ausführen |
| `FEHLER: Kein vortrainiertes Seg-Modell` | Schritt 1 noch nicht gemacht | Erst `python EdgeAI.py 1` ausführen |
| `Pruning übersprungen, torch/prune nicht verfügbar` | PyTorch nicht installiert | `pip install torch` |
| `Quantisierung übersprungen, onnxruntime fehlt` | ONNX Runtime nicht installiert | `pip install onnxruntime` |
| `Schlechte Validierungsmetriken` | Zu wenige Trainingsdaten oder schlechte Labels | Mehr echte Bilder labeln und Schritt 2 neu ausführen |

---

# ✅ Checkliste vor Training

- [ ] Blender-Daten generiert (`Blender_Segment_Keypoint_Generation.py`)
- [ ] `dataset/images`, `dataset/labels`, `pose_dataset` vorhanden
- [ ] GPU verfügbar oder CPU akzeptabel
- [ ] YOLO11n-Modelle vorhanden (`yolo11n-seg.pt`, `yolo11n-pose.pt`)
- [ ] CUDA/PyTorch installiert (falls GPU gewünscht)
- [ ] Für Optimierung: `torch`, `onnxruntime` installiert

## Vor Schritt [2] (Echt)

- [ ] Schritt [1] erfolgreich abgeschlossen
- [ ] Echte Bilder erfasst und in `Image_Capture_Python/` gespeichert
- [ ] `Umrisse_in_Polygone.py [2]` ausgeführt
- [ ] Labels manuell überprüft/korrigiert (Label Studio)
- [ ] `real_dataset/images/all` und `real_dataset/labels/all` gefüllt
- [ ] `real_pose_dataset/images/train|val` und `real_pose_dataset/labels/train|val` gefüllt

## Vor Schritt [4] (Export)

- [ ] Schritt [2] erfolgreich abgeschlossen
- [ ] Optionale Optimierung durchgeführt (falls gewünscht)
- [ ] `Edge Export/` Verzeichnis vorhanden oder wird erstellt

---

# 💡 Best Practices

### Training-Strategie
1. **Schritt 1 (Virtuell):** ~1-2 Stunden Training
2. **Schritt 2 (Echt):** Mit nur 100-200 echten Bildern sehr gut möglich (Transfer Learning)
3. **Schritt 3 (Validierung):** Überprüfen Sie mAP-Metriken
4. **Schritt 4 (Export):** Optional quantisieren für Edge-Deployment

### GPU-Optimierung
- **BATCH_SIZE:** 16 ist Standard, 8-32 je nach GPU-RAM
- **IMG_SIZE:** 640 Standard, aber kann auf 512 reduziert werden für schnelleres Training
- **EPOCHS:** 100 ist Standard, kann auf 50 reduziert werden wenn Daten klein

### Freezing beim Fine-Tuning
- `freeze=10`: First 10 layers (features)
- `freeze=0`: Alle Layer trainieren (wenn wenig Blender-Transfer gewünscht)

### Learning Rate
- **Blender (Schritt 1):** `lr0=0.01` (Standard YOLO)
- **Real Fine-Tuning (Schritt 2):** `lr0=0.001` (10× kleiner, konservativer)

---

**Dokumentation erstellt:** 2026-04-17  
**Skript-Version:** basierend auf EdgeAI.py
