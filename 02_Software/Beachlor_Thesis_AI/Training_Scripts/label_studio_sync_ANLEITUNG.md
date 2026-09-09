# Label Studio Sync - Praktische Anleitung

## Überblick
Dieses Skript automatisiert den Import von annotierten Daten aus **Label Studio** in deine lokalen YOLO-Trainingsdatensätze.

Es unterstützt zwei Workflows:
- **Pose-Daten**: Keypoint-Annotationen (YOLO-Format) → `real_pose_dataset`
- **Segmentierungen**: Polygon-Annotationen (COCO-Format) → `real_dataset`

---

## Schritt 1: In Label Studio exportieren

### Für Pose-Daten (Keypoints):
1. In Label Studio: Projekt → **Export** → **YOLO**
2. Speicher die ZIP-Datei irgendwo lokal (z.B. Downloads)
3. Entpacke sie → du erhältst einen Ordner mit `labels/` und ggf. Bildern

### Für Segmentierungen:
1. In Label Studio: Projekt → **Export** → **COCO**
2. Speicher die Datei (wird als `result_coco.json` o.ä. exportiert)

---

## Schritt 2: Skript ausführen

### Option A: Interaktives Menü (einfach)
```bash
python Training_Scripts/label_studio_sync.py
```

Das Skript fragt dich dann:
```
[1] Pose importieren (YOLO Export)
[2] Segmente importieren (COCO Export)
[3] Beides importieren
[0] Abbrechen
```

Anschließend:
- ✅ **Dry-Run?** → Erst testen, ob alles funktioniert (empfohlen!)
- 📂 **Pose Export-Ordner?** → Pfad zum Label-Studio-Export eingeben
- 📂 **Ziel-Dataset?** → Wo sollen die Daten hin? (Default: `real_pose_dataset`)
- 📄 **YAML?** → Pfad zur `real_pose_dataset.yaml` (enthält Keypoint-Infos)

### Option B: Command-Line (für Automatisierung)
```bash
# Nur Pose
python Training_Scripts/label_studio_sync.py pose-yolo \
  --export-dir "C:/Users/Downloads/project-123-at-2026-04-23" \
  --dataset-dir ./real_pose_dataset \
  --pose-yaml ./real_pose_dataset.yaml

# Nur Segmentierung
python Training_Scripts/label_studio_sync.py seg-coco \
  --coco "C:/Users/Downloads/project-456-at-2026-04-23/result_coco.json" \
  --images-dir ./real_dataset/images/all \
  --labels-dir ./real_dataset/labels/all

# Beides
python Training_Scripts/label_studio_sync.py all \
  --pose-export-dir ./label_studio_exports/pose \
  --seg-coco ./label_studio_exports/seg/result_coco.json \
  --pose-dataset-dir ./real_pose_dataset \
  --seg-images-dir ./real_dataset/images/all \
  --seg-labels-dir ./real_dataset/labels/all
```

---

## Schritt 3: Was passiert beim Import?

### Für Pose-Daten:
```
Label Studio Export (labels/*.txt)
        ↓
1. Dateien werden erkannt und in train/val Ordner sortiert
        ↓
2. Fehlerhafte Formate werden "normalisiert" (z.B. Kommas → Punkte)
        ↓
3. Automatische 80/20 Train-Val Split (optional)
        ↓
real_pose_dataset/
  ├── images/
  │   ├── train/
  │   └── val/
  └── labels/
      ├── train/  ← Die eigentlichen Label-Dateien
      └── val/
```

### Für Segmentierungen:
```
COCO JSON (result_coco.json)
        ↓
1. Polygone werden aus dem COCO-Format extrahiert
        ↓
2. Zu lokale Bilder zugeordnet
        ↓
3. Im YOLO-Polygon-Format geschrieben
        ↓
real_dataset/labels/all/
  ├── bild1.txt
  ├── bild2.txt
  └── ...
```

---

## Wichtige Details

### Train/Val Split
Das Skript erstellt automatisch einen **80/20 Split** (80% Training, 20% Validation):
- Zufällig aber deterministisch (immer gleich mit `--split-seed 42`)
- Kann man deaktivieren: `--no-auto-split`
- Verhältnis anpassen: `--pose-val-ratio 0.30` (für 70/30)

### Backup
Bevor Daten überschrieben werden, wird ein **automatisches Backup** erstellt:
```
real_pose_dataset/
  └── labels_backup_before_pose_sync_20260423_143022/
      ├── train/
      └── val/
```

### Normalisierung
Das Skript "repariert" häufige Fehler:
- ❌ Kommas statt Punkte (`1,5` → `1.5`)
- ❌ Fehlende Keypoints (fügt `0 0 0` Platzhalter ein)
- ❌ Alte Cache-Dateien (löscht `train.cache`, `val.cache`)

---

## Praktisches Beispiel

### Szenario: Du hast 50 Bilder mit Keypoints in Label Studio annotiert

1. **In Label Studio exportieren**
   ```
   Downloads/project-robot-pose-at-2026-04-23/
   ├── labels/
   │   ├── abc123__robot_frame_001.txt
   │   ├── abc123__robot_frame_002.txt
   │   └── ...
   └── images/
       ├── robot_frame_001.jpg
       ├── robot_frame_002.jpg
       └── ...
   ```

2. **Skript ausführen**
   ```bash
   python Training_Scripts/label_studio_sync.py
   ```

3. **Im Menü auswählen: [1] Pose importieren**

4. **Pfade eingeben**
   ```
   Pose Export-Ordner [Downloads/project-robot-pose-at-2026-04-23]: 
   Pose Ziel-Dataset [real_pose_dataset]: 
   Pose YAML [real_pose_dataset.yaml]: 
   ```

5. **Fertig!** Deine 50 Bilder sind jetzt in:
   ```
   real_pose_dataset/labels/train/   ← 40 Labels (80%)
   real_pose_dataset/labels/val/     ← 10 Labels (20%)
   ```

---

## Fehlersuche

### "Pose export labels folder not found"
→ Der Label-Studio Export ist falsch. Stelle sicher, dass der Ordner `labels/*.txt` Dateien enthält.

### "Required dataset folder not found"
→ `real_pose_dataset` existiert noch nicht. Erstelle zuerst die Struktur:
```bash
mkdir -p real_pose_dataset/images/train
mkdir -p real_pose_dataset/images/val
mkdir -p real_pose_dataset/labels/train
mkdir -p real_pose_dataset/labels/val
```

### "No COCO json found"
→ Die COCO-Datei heißt nicht `result_coco.json`. Prüfe den genauen Namen im Export-Ordner.

### Labels verschwunden?
→ Kein Problem! Es gibt ein **Backup**:
```
real_pose_dataset/labels_backup_before_pose_sync_20260423_143022/
```
Du kannst die alten Dateien von dort zurückholen.

---

## Tipps & Best Practices

✅ **Zuerst Dry-Run machen**
```bash
# Im interaktiven Menü auf "Dry-Run" antworten oder:
python Training_Scripts/label_studio_sync.py pose-yolo \
  --export-dir ./label_studio_exports/pose \
  --dry-run
```
Das zeigt dir, was passieren würde, ohne etwas zu ändern.

✅ **Mehrmals import-safe**
Das Skript löscht alte Labels und schreibt nur die neuen. Du kannst es mehrmals ausführen, ohne Duplikate zu bekommen.

✅ **Konsistenter Split**
Mit demselben `--split-seed` bekommst du immer die gleiche 80/20 Aufteilung. Das ist wichtig für Reproduzierbarkeit!

✅ **YAML muss stimmen**
Die `real_pose_dataset.yaml` muss die korrekte Keypoint-Anzahl (`kpt_shape`) enthalten, damit die Normalisierung funktioniert.

---

## Workflow-Zusammenfassung

```
1. In Label Studio annotieren → Fertig ✓
2. Label Studio Export herunterladend
3. python Training_Scripts/label_studio_sync.py starten
4. Menü auswählen → Pfade eingeben
5. Import läuft automatisch
   - Labels normalisiert
   - Train/Val Split erstellt
   - Backup wird gemacht
6. Bereit zum Training! 🚀
```

Das war's! Das Skript kümmert sich um den Rest.
