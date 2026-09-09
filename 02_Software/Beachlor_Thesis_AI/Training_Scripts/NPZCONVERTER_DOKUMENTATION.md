# npzConverter.py — Detaillierte Dokumentation

## 📋 Übersicht

Das Skript **`npzConverter.py`** erstellt **Kalibrierungsdateien** für die INT8-Quantisierung von YOLO-Modellen mit STM32Cube.AI Studio oder ONNX Runtime.

Die Ausgabe ist eine **`.npz`-Datei** (NumPy Zip Format), die ein normalisiertes Bild-Array im Format **(N, 3, H, W)** enthält — genau wie YOLO es erwartet.

### 🎯 Kernaufgaben

1. **Bilder sammeln** aus verschiedenen Quellen (Ordner, Datei, TXT-Liste, YAML)
2. **Bilder vorverarbeiten** wie YOLO (Letterbox, BGR→RGB, Normalisierung)
3. **Array speichern** als `.npz` für Quantisierungs-Kalibrierung

---

## 🔄 Warum ist Kalibrierung wichtig?

Quantisierung braucht **Beispieldaten**, um zu lernen, wie man Float32-Werte auf Int8 (8-Bit) komprimiert, ohne zu viel Genauigkeit zu verlieren.

```
Original-Modell (FP32)
    ↓
INT8-Quantisierung (mit Kalibrierbildern)
    ↓
Kleines, schnelles Edge-Modell (INT8) ← für STM32
```

**Resultat:** Modell wird 4× kleiner, läuft schneller auf Microcontroller-Hardware.

---

## 📁 Unterstützte Eingabe-Formate

Das Skript kann Bilder aus verschiedenen Quellen laden:

### 1. Direkter Bild-Ordner
```bash
python npzConverter.py \
  --images "./dataset/images" \
  --out calibration.npz \
  --imgsz 640 \
  --count 200
```

### 2. Einzelne Bilddatei
```bash
python npzConverter.py \
  --images "./image.png" \
  --out single_calibration.npz
```

### 3. TXT-Liste mit Bildpfaden
```bash
# imagelist.txt:
# ./images/img1.png
# ./images/img2.jpg
# ./images/img3.png

python npzConverter.py \
  --images "./imagelist.txt" \
  --out calibration.npz
```

### 4. YOLO dataset.yaml
```bash
python npzConverter.py \
  --dataset "./dataset.yaml" \
  --split val \
  --out calibration.npz
```

---

# 🔍 Funktionen im Detail

## Konfiguration und Konstanten

```python
SUPPORTED_EXTS = ('.jpg', '.jpeg', '.png', '.bmp', '.tif', '.tiff')
```

Alle unterstützten Bilddateitypen. Das Skript filtert automatisch auf diese Extensions.

---

## Datei-Zugriffsfunktionen

### `read_dataset_yaml(path: str) -> dict`

```python
def read_dataset_yaml(path: str) -> dict:
    with open(path, 'r') as f:
        return yaml.safe_load(f)
```

#### Zweck
Liest eine YOLO-`dataset.yaml` in Python-Dict.

#### Beispiel-Input
```yaml
# dataset.yaml
path: /absolute/path/to/dataset
train: images/train
val: images/val
test: images/test
nc: 5
names: [Base, Joint_1, Joint_2, Joint_3, Joint_4]
```

#### Rückgabe
```python
{
    'path': '/absolute/path/to/dataset',
    'train': 'images/train',
    'val': 'images/val',
    'test': 'images/test',
    'nc': 5,
    'names': ['Base', 'Joint_1', 'Joint_2', 'Joint_3', 'Joint_4']
}
```

---

### `resolve_image_list(entry: str, dataset_dir: Path) -> list[str]`

```python
def resolve_image_list(entry: str, dataset_dir: Path) -> list[str]:
    """
    entry: path in dataset.yaml (dir or txt listing images)
    dataset_dir: directory containing dataset.yaml (for relative paths)
    returns: list of absolute image file paths
    """
```

#### Zweck
Konvertiert einen **Pfad-String** aus `dataset.yaml` in eine **Liste absoluter Bildpfade**.

Dies ist die Schlüsselfunktion für flexible Pfad-Auflösung.

#### Logik (Schrittweise)

**1. Pfad zu Path-Objekt konvertieren**
```python
p = Path(entry)
```

**2. Relative Pfade auflösen**
```python
if not p.is_absolute():
    p = (dataset_dir / p).resolve()
```

Falls `entry = "images/train"` und `dataset_dir = "/data/myproject"`, dann wird zu `/data/myproject/images/train`.

**3. Je nachdem was `p` ist:**

#### Fall A: Einzelne Bilddatei
```python
if p.is_file() and p.suffix.lower() in SUPPORTED_EXTS:
    return [str(p)]
```

Rückgabe: Liste mit einer einzigen Bilddatei.

#### Fall B: TXT-Datei mit Bildlisten
```python
if p.is_file():  # Aber nicht .jpg/.png etc.
    imgs = []
    with open(p, 'r') as fh:
        for line in fh:
            line = line.strip()
            if not line:
                continue
            ip = Path(line)
            if not ip.is_absolute():
                ip = (p.parent / ip).resolve()  # Relative zum TXT-Ordner
            if ip.exists():
                imgs.append(str(ip))
    return imgs
```

**Beispiel:**
```
# list.txt (in /data/images/)
subfolder/img1.jpg
subfolder/img2.jpg
../other/img3.png

# Alle Pfade werden relativ zu /data/images/ aufgelöst:
/data/images/subfolder/img1.jpg
/data/images/subfolder/img2.jpg
/data/other/img3.png
```

#### Fall C: Verzeichnis mit Bildern
```python
if p.is_dir():
    imgs = []
    for ext in SUPPORTED_EXTS:
        imgs.extend(sorted([str(x.resolve()) for x in p.glob(f'*{ext}')]))
    return imgs
```

Sucht alle Bilder im Verzeichnis und sortiert sie alphabetisch.

#### Fall D: Fallback (Wildcard-Suche)
```python
for ext in SUPPORTED_EXTS:
    imgs.extend(sorted([str(x.resolve()) for x in base.rglob(f'*{ext}')]))
```

Falls der Pfad nicht existiert, sucht rekursiv im `dataset_dir` nach allen Bildern (`rglob`).

#### Rückgabe
```python
[
    '/absolute/path/to/images/img1.jpg',
    '/absolute/path/to/images/img2.jpg',
    '/absolute/path/to/images/img3.png',
]
```

---

### `collect_images_from_dataset_yaml(yaml_path: str, split: str) -> list[str]`

```python
def collect_images_from_dataset_yaml(yaml_path: str, split: str) -> list[str]:
```

#### Zweck
Sammelt alle Bilder aus einem bestimmten Split einer `dataset.yaml`.

#### Parameter
- `yaml_path`: Pfad zur Datei (z.B. `"/data/dataset.yaml"`)
- `split`: `"train"`, `"val"`, `"test"`, oder `"all"`

#### Prozess

1. **YAML lesen**
   ```python
   data = read_dataset_yaml(yaml_path)
   dataset_dir = Path(yaml_path).parent
   imgs = []
   ```

2. **Keys bestimmen** (welche Splits sollen genutzt werden?)
   ```python
   if split == 'all':
       keys = [k for k in ('train', 'val', 'test') if k in data]
   else:
       keys = [split]
   ```

3. **Für jeden Key Bilder auflösen**
   ```python
   for k in keys:
       entry = data.get(k)
       if entry:
           imgs.extend(resolve_image_list(entry, dataset_dir))
   ```

4. **Duplikate entfernen** (aber Reihenfolge behalten)
   ```python
   seen = set()
   imgs_unique = []
   for p in imgs:
       if p not in seen:
           seen.add(p)
           imgs_unique.append(p)
   return imgs_unique
   ```

#### Beispiel
```python
# dataset.yaml:
# train: images/train
# val: images/val

result = collect_images_from_dataset_yaml("dataset.yaml", split="val")
# → ['/absolute/path/images/val/img1.jpg', '/absolute/path/images/val/img2.jpg', ...]
```

---

### `collect_images_from_input(images_entry: str) -> list[str]`

```python
def collect_images_from_input(images_entry: str) -> list[str]:
    """
    Resolve images directly from an image source:
    - image file
    - directory with images
    - txt file containing image paths
    """
```

#### Zweck
**Einfachere** Variante von `resolve_image_list()` — direkter Zugriff ohne dataset.yaml.

#### Logik
```python
p = Path(images_entry)
if not p.exists():
    raise FileNotFoundError(...)

base_dir = Path.cwd()  # Aktuelles Verzeichnis
imgs = resolve_image_list(images_entry, base_dir)
```

Nutzt `resolve_image_list()` intern, aber mit dem **aktuellen Arbeitsverzeichnis** als Basis.

#### Beispiel
```bash
python npzConverter.py --images "./myimages" --out calibration.npz

# Sucht alle Bilder in ./myimages/
# (relativ zum Verzeichnis, von dem das Skript gestartet wurde)
```

---

## Bildverarbeitung (Preprocessing)

### `letterbox_bgr(img_bgr, new_shape, color=(114, 114, 114), auto=False, stride=32) -> np.ndarray`

```python
def letterbox_bgr(
    img_bgr: np.ndarray,
    new_shape: int,
    color: tuple[int, int, int] = (114, 114, 114),
    auto: bool = False,
    stride: int = 32
) -> np.ndarray:
    """
    YOLO-typische Letterbox:
    - Aspect Ratio beibehalten
    - auf new_shape x new_shape in ein gepaddetes Canvas legen
    - Padding-Farbe wie YOLO üblich: 114
    """
```

#### Zweck
Skaliert Bilder auf **YOLO-Standard-Eingabegröße**, ohne das Seitenverhältnis zu verzerren.

Dies ist **kritisch** für korrekte Quantisierung — das Modell erwartet exakt dieses Preprocessing!

#### Visuelle Erklärung

**Hartes Resize (falsch):**
```
Original: 800×600 (4:3)
↓
Resize auf 640×640
↓
Verzerrtes Bild (1:1) — Trainingsdaten sehen so nicht aus!
```

**Letterbox (richtig):**
```
Original: 800×600 (4:3)
↓
Skaliere auf 640×480 (behält 4:3)
↓
Lege auf 640×640 Canvas (Padding oben+unten grau)
↓
Das Modell sieht das gleiche Preprocessing wie beim Training!
```

#### Mathematik

**1. Scale-Faktor berechnen**
```python
r = min(new_shape / w, new_shape / h)
```

```
Beispiel:
  new_shape = 640
  Original: w=800, h=600
  r = min(640/800, 640/600) = min(0.8, 1.067) = 0.8
```

**2. Neue Größe (ungepaddet)**
```python
new_unpad = (int(round(w * r)), int(round(h * r)))
```

```
  new_unpad = (800*0.8, 600*0.8) = (640, 480)
```

**3. Padding berechnen**
```python
dw = new_shape - new_unpad[0]  # = 640 - 640 = 0
dh = new_shape - new_unpad[1]  # = 640 - 480 = 160

dw /= 2  # = 0
dh /= 2  # = 80
```

Also: 80 Pixel Padding oben + 80 Pixel Padding unten.

**4. Resize durchführen**
```python
if (w, h) != new_unpad:
    interp = cv2.INTER_AREA if r < 1.0 else cv2.INTER_LINEAR
    img_bgr = cv2.resize(img_bgr, new_unpad, interpolation=interp)
```

- **Downscaling** (r < 1.0): `INTER_AREA` (bessere Qualität)
- **Upscaling** (r ≥ 1.0): `INTER_LINEAR` (Interpolation)

**5. Padding hinzufügen**
```python
top = int(round(dh - 0.1))
bottom = int(round(dh + 0.1))
left = int(round(dw - 0.1))
right = int(round(dw + 0.1))
img_bgr = cv2.copyMakeBorder(img_bgr, top, bottom, left, right, cv2.BORDER_CONSTANT, value=color)
```

`value=color` ist standardmäßig `(114, 114, 114)` — grauer Farbton, den YOLO nutzt.

**6. Sicherheits-Check** (Rounding-Fehler)
```python
img_bgr = img_bgr[:new_shape, :new_shape]
if img_bgr.shape[0] != new_shape or img_bgr.shape[1] != new_shape:
    img_bgr = cv2.resize(img_bgr, (new_shape, new_shape), interpolation=cv2.INTER_LINEAR)
```

Falls durch Rounding-Fehler die Größe nicht exakt stimmt, wird nochmal resized.

#### Parameter: `auto` und `stride`

**`auto=False` (Standard, empfohlen):**
```
Padding wird nicht angepasst.
Ergebnis ist immer exakt new_shape × new_shape.
```

**`auto=True` (YOLO-Inference-Modus):**
```
Padding wird so angepasst, dass die Gesamtgröße
ein Vielfaches von stride ist.
Beispiel mit stride=32:
  640 ist 20×32 → OK
  Aber bei 642: würde auf 64×32=640 reduzieren
```

Für Kalibrierung ist `auto=False` sinnvoller — feste Größe für konsistente Quantisierung.

#### Rückgabe
```python
# shape: (640, 640, 3) — BGR-Format, uint8
numpy.ndarray
```

---

### `load_and_preprocess(img_paths, imgsz, norm_to_1=True, use_letterbox=True, ...) -> np.ndarray`

```python
def load_and_preprocess(
    img_paths: list[str],
    imgsz: int,
    norm_to_1: bool = True,
    use_letterbox: bool = True,
    letterbox_auto: bool = False,
    letterbox_stride: int = 32,
    show_progress: bool = True
) -> np.ndarray:
    """
    Load images, YOLO-preprocess:
    - BGR read
    - force 3-channel BGR
    - letterbox to imgsz x imgsz (default) OR direct resize
    - BGR -> RGB
    - HWC -> CHW
    - float32 and normalize to [0,1] if requested
    Returns numpy array shape (N,3,H,W) float32.
    """
```

#### Zweck
**Komplette Vorverarbeitung** — lädt Bilder und konvertiert sie in das YOLO-Standard-Format.

Dies ist die Kernfunktion für die Kalibrierungsdatei-Erzeugung.

#### Prozess (für jedes Bild)

**1. Bild mit OpenCV laden**
```python
img = cv2.imread(p, cv2.IMREAD_UNCHANGED)
if img is None:
    print(f"Warning: could not read {p}, skipping.")
    continue
```

**2. In 3-Kanal-BGR konvertieren**
```python
if len(img.shape) == 2:  # Grayscale
    img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
elif img.shape[2] == 4:  # RGBA
    img = cv2.cvtColor(img, cv2.COLOR_BGRA2BGR)
```

Sichert, dass jedes Bild genau 3 Kanäle hat.

**3. Auf Zielgröße skalieren**
```python
if use_letterbox:
    img = letterbox_bgr(img, new_shape=imgsz, auto=letterbox_auto, stride=letterbox_stride)
else:
    img = cv2.resize(img, (imgsz, imgsz), interpolation=cv2.INTER_LINEAR)
```

Letterbox bevorzugt (für Kalibrierung empfohlen).

**4. BGR → RGB konvertieren**
```python
img_rgb = img[:, :, ::-1]
```

OpenCV lädt Bilder in BGR, aber neuronale Netze trainieren mit RGB.
`::-1` kehrt die Kanal-Reihenfolge um: `[B, G, R]` → `[R, G, B]`.

**5. HWC → CHW (Channel-First)**
```python
img_chw = np.transpose(img_rgb, (2, 0, 1)).astype(np.float32)
```

YOLO erwartet das Format `(Batch, Channels, Height, Width)` = NCHW.

```
Original:  (640, 640, 3) = HWC
Transposed: (3, 640, 640) = CHW
```

**6. Optional: Normalisierung auf [0, 1]**
```python
if norm_to_1:
    img_chw /= 255.0
```

```
uint8 [0, 255] → float32 [0.0, 1.0]

Beispiel:
  128 (uint8) → 128/255.0 ≈ 0.502 (float32)
```

**Diese Normalisierung ist essentiell** — YOLO-Modelle erwarten normalisierte Eingaben!

**7. Zum Output hinzufügen**
```python
out.append(img_chw)
```

**8. Stack zu (N, 3, H, W)**
```python
if not out:
    raise RuntimeError("No images were loaded...")
return np.stack(out, axis=0)
```

```
Beispiel mit 200 Bildern:
  np.stack([arr1, arr2, ..., arr200], axis=0)
  → shape (200, 3, 640, 640)
  → dtype float32
```

#### Parameter

| Parameter | Standard | Bedeutung |
|-----------|----------|-----------|
| `imgsz` | 640 | Zielbildgröße (muss zum Modell passen!) |
| `norm_to_1` | True | Normalisieren auf [0,1] (YOLO-Standard) |
| `use_letterbox` | True | Letterbox statt direktes Resize |
| `letterbox_auto` | False | Stride-Alignment (für Inferenz, nicht Kalibrierung) |
| `letterbox_stride` | 32 | Stride für Auto-Mode |
| `show_progress` | True | Progress-Bar anzeigen |

#### Rückgabe
```python
numpy.ndarray
  shape: (N, 3, H, W)
  dtype: float32
  Wertebereich: [0.0, 1.0] (wenn norm_to_1=True)
```

---

## Hauptprogramm

### `main()`

```python
def main():
    parser = argparse.ArgumentParser(description="Create calibration .npz for STM32 quantization from images (YOLO letterbox)")
```

#### Argument-Parser

Alle verfügbaren Kommando-Zeilen-Optionen:

```python
--images, -i          # Bildquelle (Ordner/Datei/TXT)
--dataset, -d         # Alternativ: dataset.yaml
--split               # Split wählen: train/val/test/all (default: val)
--out, -o             # Output .npz (default: calibration.npz)
--imgsz               # Zielgröße (default: 640) ⚠️ MUSS zum Modell passen!
--count               # Anzahl Bilder (default: 200)
--seed                # Random Seed (default: 42)
--shuffle             # Bilder vor Sampling mischen
--norm_to_1           # Normalisierung auf [0,1] (empfohlen!)
--no_letterbox        # Letterbox deaktivieren (nicht empfohlen)
--letterbox_auto      # Stride-Aligned Padding
--letterbox_stride    # Stride-Wert (default: 32)
--verbose             # Verbose Output
```

#### Logik

**1. Eingabe-Validierung**
```python
if not args.images and not args.dataset:
    print("Error: provide either --images or --dataset.", file=sys.stderr)
    sys.exit(2)
```

Entweder `--images` oder `--dataset` muss gesetzt sein.

**2. Bilder sammeln**
```python
if args.images:
    imgs = collect_images_from_input(args.images)
else:
    yaml_path = Path(args.dataset)
    if not yaml_path.exists():
        print(f"Error: dataset.yaml not found at {yaml_path}", file=sys.stderr)
        sys.exit(2)
    imgs = collect_images_from_dataset_yaml(str(yaml_path), args.split)
```

**3. Fehlerbehandlung**
```python
if len(imgs) == 0:
    print("No images found for the requested split. Exiting.", file=sys.stderr)
    sys.exit(2)
```

**4. Optionales Mischen**
```python
if args.shuffle:
    random.Random(args.seed).shuffle(imgs)
```

Deterministische Randomisierung (gleicher Seed = gleiche Reihenfolge).

**5. Sampling**
```python
count = min(args.count, len(imgs))
sampled = imgs[:count]
```

Nimmt bis zu `--count` Bilder.

**6. Verbose-Output**
```python
if args.verbose:
    print(f"Found {len(imgs)} images total; sampling {count} images for calibration.")
    print("Sampled example files:")
    for p in sampled[:5]:
        print("  ", p)
    print(f"Letterbox: {'OFF' if args.no_letterbox else 'ON'} ...")
    print(f"Normalize to [0,1]: {'ON' if args.norm_to_1 else 'OFF'}")
```

Zeigt die Einstellungen an.

**7. Vorverarbeitung**
```python
arr = load_and_preprocess(
    sampled,
    imgsz=args.imgsz,
    norm_to_1=args.norm_to_1,
    use_letterbox=(not args.no_letterbox),
    letterbox_auto=args.letterbox_auto,
    letterbox_stride=args.letterbox_stride,
    show_progress=not args.verbose
)
```

Ruft die Kernfunktion auf.

**8. Output-Verzeichnis erstellen**
```python
out_path = Path(args.out)
if out_path.parent != Path(''):
    out_path.parent.mkdir(parents=True, exist_ok=True)
```

Falls das Output-Verzeichnis nicht existiert, wird es erstellt.

**9. .npz speichern**
```python
np.savez(out_path, input=arr)
print(f"Saved calibration file: {out_path} with shape {arr.shape} and dtype {arr.dtype}")
```

Speichert als NumPy-Zip mit Schlüssel `"input"`.

---

# 📊 Output-Format: .npz

Die `.npz`-Datei ist ein **ZIP-Archiv** mit NumPy-Arrays.

### Struktur
```python
# Speichern:
np.savez(out_path, input=arr)

# Laden:
data = np.load('calibration.npz')
calibration_data = data['input']  # shape (N, 3, 640, 640), dtype float32
```

### Content
```
calibration.npz
├── input.npy  (eigentliches Array)
└── metadata.json  (optional)
```

### Format des `input`-Arrays
```
Shape: (N, 3, H, W)
  N = Anzahl der Bilder (z.B. 200)
  3 = RGB-Kanäle
  H, W = Höhe, Breite (z.B. 640, 640)
  
Dtype: float32
Values: [0.0, 1.0] (normalisiert)
```

### Beispiel
```python
import numpy as np

# Laden
data = np.load('calibration.npz')
arr = data['input']

print(arr.shape)   # (200, 3, 640, 640)
print(arr.dtype)   # float32
print(arr.min())   # 0.0
print(arr.max())   # 1.0
print(arr[0, 0, :3, :3])  # Erste 3×3 Pixel des ersten Bildes, Rot-Kanal
```

---

# 🚀 Praktische Beispiele

## Beispiel 1: Einfacher Ordner

```bash
cd C:\Dev\AI\Beachlor_Thesis_AI\Training_Scripts

python npzConverter.py \
  --images "..\..\..\Image_Capture_Python\dataset\images\<capture_ordner>" \
  --out calibration.npz \
  --imgsz 640 \
  --count 200 \
  --norm_to_1
```

**Was passiert:**
1. Sucht alle `.png` und `.jpg` in der Capture-Ordner
2. Nimmt erste 200 Bilder
3. Letterbox auf 640×640
4. Normalisiert auf [0, 1]
5. Speichert als `calibration.npz`

**Output:**
```
Saved calibration file: calibration.npz with shape (200, 3, 640, 640) and dtype float32
```

---

## Beispiel 2: Mit dataset.yaml

```bash
python npzConverter.py \
  --dataset ".\pose_dataset\pose_dataset.yaml" \
  --split val \
  --out pose_calibration.npz \
  --imgsz 640 \
  --count 100 \
  --norm_to_1 \
  --verbose
```

**Was passiert:**
1. Liest `pose_dataset.yaml`
2. Nutzt nur `val`-Split (Validierungsbilder)
3. Nimmt bis zu 100 Bilder
4. Zeigt Infos an (verbose)

**Output:**
```
Found 150 images total; sampling 100 images for calibration.
Sampled example files:
   /absolute/path/pose_dataset/images/val/img_001.png
   /absolute/path/pose_dataset/images/val/img_002.png
   /absolute/path/pose_dataset/images/val/img_003.png
   /absolute/path/pose_dataset/images/val/img_004.png
   /absolute/path/pose_dataset/images/val/img_005.png
Letterbox: ON (auto=False, stride=32)
Normalize to [0,1]: ON
Loading images: 100%|██████████| 100/100 [00:05<00:00, 19.80 it/s]
Saved calibration file: pose_calibration.npz with shape (100, 3, 640, 640) and dtype float32
```

---

## Beispiel 3: Mit TXT-Bildliste

```bash
# imagelist.txt erstellen:
cat > imagelist.txt << EOF
./images/img1.png
./images/img2.png
./images/img3.png
EOF

python npzConverter.py \
  --images imagelist.txt \
  --out calibration.npz \
  --imgsz 640 \
  --count 3
```

---

## Beispiel 4: Mit Shuffle und Seed

```bash
python npzConverter.py \
  --images "./dataset/images" \
  --out calibration.npz \
  --imgsz 640 \
  --count 200 \
  --shuffle \
  --seed 42 \
  --norm_to_1
```

**Reproduzierbarkeit:** Der gleiche `--seed` erzeugt immer die gleiche Reihenfolge.

---

## Beispiel 5: Ohne Letterbox (nicht empfohlen)

```bash
python npzConverter.py \
  --images "./dataset/images" \
  --out calibration.npz \
  --imgsz 640 \
  --count 200 \
  --no_letterbox
```

⚠️ **Warnung:** Dies verzerrt Bilder! Nur wenn das Modell auch ohne Letterbox trainiert wurde.

---

# 📋 Spezifikation für STM32Cube.AI

STM32Cube.AI erwartet die Kalibrierungsdatei in genau diesem Format:

| Parameter | Anforderung |
|-----------|------------|
| Format | `.npz` (NumPy-Zip) |
| Schlüssel | `"input"` |
| Shape | `(N, C, H, W)` |
| C (Kanäle) | 3 (RGB) |
| H, W | **Muss zum Modell passen** (meist 640×640) |
| Dtype | `float32` |
| Value Range | `[0.0, 1.0]` (normalisiert) |
| Min. Bilder (N) | 100-200 empfohlen |

**Kritisch:** Die Bildgröße muss zum Modell passen!

```bash
# Wenn das Modell 640×640 erwartet:
--imgsz 640

# Wenn es 512×512 erwartet:
--imgsz 512
```

---

# 🐛 Häufige Fehler & Lösungen

| Fehler | Ursache | Lösung |
|--------|--------|---------|
| `Error: provide either --images or --dataset` | Keine Eingabequelle | `--images` oder `--dataset` setzen |
| `FileNotFoundError: Image source not found` | Pfad existiert nicht | Absoluten Pfad verwenden oder `cd` in richtiges Verzeichnis |
| `No images found for the requested split` | Keine Bilder im Split | Split überprüfen oder `--split all` nutzen |
| `Error resolving input images: ...` | Pfad-Auflösungsfehler | Relative/absolute Pfade konsistent halten |
| `Shape mismatch in STM32Cube.AI` | `imgsz` stimmt nicht mit Modell | Modell-Eingabegröße überprüfen |
| `Quantisierung funktioniert nicht gut` | Kalibrierbilder nicht repräsentativ | Mehr Bilder nutzen (200-500) oder andere Bilder |

---

# ✅ Checkliste

- [ ] Modell-Eingabegröße bekannt (z.B. 640×640)
- [ ] Bilder verfügbar (100-500 empfohlen)
- [ ] Dataset-Ordner oder YAML vorbereitet
- [ ] `npzConverter.py` mit richtiger `--imgsz` ausgeführt
- [ ] `calibration.npz` erfolgreich erstellt
- [ ] Dateigröße plausibel (200 Bilder × 3 Kanäle × 640 × 640 × 4 Bytes ≈ 491 MB)
- [ ] `calibration.npz` in STM32Cube.AI geladen
- [ ] Quantisierung durchgeführt
- [ ] Quantisiertes Modell getestet

---

# 🔗 Integration mit dem Gesamtworkflow

```
Blender/Reale Bilder
    ↓
Umrisse_in_Polygone.py [1 oder 2]
    ↓
EdgeAI.py [1 oder 2] (Training → best.pt)
    ↓
EdgeAI.py [4] (Export → .onnx)
    ↓
npzConverter.py (Kalibrierbilder → calibration.npz) ← DU BIST HIER
    ↓
STM32Cube.AI (INT8-Quantisierung → .tflite)
    ↓
Embedded-Deployment (Mikrocontroller)
```

---

# 💡 Best Practices

### Bild-Auswahl
- **Repräsentativ:** Nutze Bilder aus verschiedenen Bedingungen (Beleuchtung, Winkel)
- **Anzahl:** 100-200 Bilder reichen meist, 500+ für höchste Genauigkeit
- **Clean:** Bilder sollten ähnlich aussehen wie Trainingsbilder

### Normalisierung
- **Immer `--norm_to_1` nutzen** (YOLO-Standard)
- Ohne Normalisierung wird Quantisierung suboptimal

### Letterbox vs. Resize
- **Immer Letterbox nutzen** (Standard)
- Das trainierte Modell sieht das gleiche Preprocessing
- `--no_letterbox` nur wenn das Modell speziell dafür trainiert wurde

### Reproducibility
- **Seed setzen:** `--seed 42` für reproduzierbare Kalibrierungen
- Gleicher Seed = gleiche Bilder-Reihenfolge = gleiche Quantisierung

### Debugging
- **Verbose-Modus:** `--verbose` zeigt geladene Bilder und Einstellungen
- **Shape-Überprüfung:** Output sollte `(N, 3, 640, 640)` sein
- **Ladetest:** `np.load('calibration.npz')` manuell überprüfen

---

# 📚 Abhängigkeiten

```
numpy          # Array-Operationen
opencv-python  # Bildladen + Preprocessing
pyyaml         # dataset.yaml parsen
tqdm           # Progress-Bars
```

Installation:
```bash
pip install numpy opencv-python pyyaml tqdm
```

---

**Dokumentation erstellt:** 2026-04-16  
**Skript-Version:** basierend auf npzConverter.py
