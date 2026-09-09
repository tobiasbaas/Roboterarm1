# KI-Pipeline Flussdiagramme — Roboterarm Pose Estimation

## 1. High-Level: Gesamte KI-Pipeline

Dieses Diagramm zeigt den gesamten Ablauf von der synthetischen Datengenerierung bis zur Echtzeit-Inferenz auf einem Mikrocontroller.

```mermaid
flowchart TB
    subgraph DATENERZEUGUNG["1 — Datenerzeugung"]
        BLENDER["Blender-Szene<br/>(Roboterarm-Modell)"]
        BLENDER_SCRIPT["Blender_Segment_Keypoint_Generation.py<br/>● Zufaellige Posen<br/>● Domain Randomization<br/>● RGB + Masken + Pose-Labels + CSV"]
        BLENDER --> BLENDER_SCRIPT
    end

    subgraph KONVERTIERUNG["2 — Datenkonvertierung"]
        UMRISSE_V["Umrisse_in_Polygone.py [1] Virtuell<br/>● Masken → YOLO-Seg-Labels<br/>● Pose-Labels organisieren<br/>● Train/Val Split"]
        UMRISSE_R["Umrisse_in_Polygone.py [2] Echt<br/>● Auto-Annotation mit Blender-Modell<br/>● Seg → Pose-Labels ableiten"]
        KAMERA["Echte Kamera-Bilder<br/>(Image_Capture_Python)"]
    end

    subgraph TRAINING["3 — Modell-Training"]
        EDGE_V["EdgeAI.py [1] Virtuell<br/>● Seg-Modell (YOLO11n-seg)<br/>● Pose-Modell (YOLO11n-pose)"]
        EDGE_R["EdgeAI.py [2] Echt<br/>● Fine-Tune Seg auf echten Bildern<br/>● Fine-Tune Pose auf echten Bildern"]
    end

    subgraph VALIDIERUNG["4 — Validierung"]
        VAL["Validierung.ipynb<br/>● Loss-Kurven, mAP, Confusion Matrix<br/>● Seg → Centroids → Winkel<br/>● Pose → Keypoints → Winkel<br/>● Blender- und Real-Vergleich"]
        WINKEL["Umrisse_in_Polygone.py [3]<br/>Winkelberechnung"]
    end

    subgraph DEPLOYMENT["5 — Export und Deployment"]
        EXPORT["EdgeAI.py [4] Export<br/>● ONNX-Export<br/>● INT8-Quantisierung"]
        NPZ["npzConverter.py<br/>● Kalibrierungsdaten fuer INT8"]
        EDGE_HW["STM32 / Edge-Hardware<br/>Echtzeit-Inferenz"]
    end

    %% Verbindungen
    BLENDER_SCRIPT -->|"dataset/<br/>images + masks + labels_pose + CSV"| UMRISSE_V
    UMRISSE_V -->|"yolo_dataset/ + pose_dataset/<br/>YOLO-Format"| EDGE_V
    EDGE_V -->|"Blender-Modelle<br/>(roboterarm_seg, _pose)"| UMRISSE_R
    KAMERA -->|"Echte Fotos"| UMRISSE_R
    UMRISSE_R -->|"real_dataset/ + real_pose_dataset/<br/>YOLO-Format"| EDGE_R
    EDGE_V -->|"Pre-trained Weights"| EDGE_R
    EDGE_V --> VAL
    EDGE_R --> VAL
    EDGE_R -->|"Trainierte Modelle<br/>(best.pt)"| WINKEL
    EDGE_R -->|"best.pt"| EXPORT
    NPZ -->|"calibration.npz"| EXPORT
    EXPORT -->|"ONNX / INT8"| EDGE_HW

    %% Styling
    style DATENERZEUGUNG fill:#e8f4fd,stroke:#1976d2,stroke-width:2px
    style KONVERTIERUNG fill:#fff3e0,stroke:#e65100,stroke-width:2px
    style TRAINING fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px
    style VALIDIERUNG fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px
    style DEPLOYMENT fill:#fce4ec,stroke:#c62828,stroke-width:2px
```

### Zusammenfassung der Phasen

| Phase | Skript | Eingabe | Ausgabe |
|-------|--------|---------|---------|
| **1. Datenerzeugung** | `Blender_Segment_Keypoint_Generation.py` | Blender-Szene | RGB, Masken, Pose-Labels, CSV |
| **2a. Konvertierung (Virtuell)** | `Umrisse_in_Polygone.py [1]` | Blender-Dataset | YOLO-Seg + Pose Datasets |
| **2b. Konvertierung (Echt)** | `Umrisse_in_Polygone.py [2]` | Kamera-Bilder + Blender-Modell | YOLO-Seg + Pose Datasets (Real) |
| **3a. Training (Virtuell)** | `EdgeAI.py [1]` | YOLO-Datasets | Blender-Modelle (best.pt) |
| **3b. Training (Echt)** | `EdgeAI.py [2]` | Real-Datasets + Pre-trained | Real-Modelle (best.pt) |
| **4. Validierung** | `Validierung.ipynb` | Trainierte Modelle | Metriken, Winkelvergleich |
| **5. Export** | `EdgeAI.py [4]` + `npzConverter.py` | best.pt + Kalibrierbilder | ONNX + INT8 |

---

## 2. Detail-Diagramm: Datengenerierung und Konvertierung

Detaillierter Ablauf innerhalb der Blender-Datengenerierung (mit Domain Randomization) und der anschliessenden Konvertierung in YOLO-Trainingsformat.

```mermaid
flowchart TB
    subgraph BLENDER_INIT["Initialisierung (einmalig)"]
        SCENE["Blender-Szene laden<br/>(Roboterarm + Kamera + Licht)"]
        CALIB["keypoint_axis_calibration.py<br/>● Marker platzieren<br/>● Offsets berechnen<br/>● KEYPOINT_SPECS erstellen"]
        SNAPSHOT["Snapshot erstellen<br/>● Kamera-Position/Rotation<br/>● Licht-Intensitaet/Farbe<br/>● World-Hintergrund<br/>● Belichtung"]
        DIRS["Output-Verzeichnisse erstellen<br/>images/ masks/ labels_pose/"]
        SCENE --> CALIB --> SNAPSHOT --> DIRS
    end

    subgraph LOOP["Hauptschleife (pro Sample i = 0 .. N)"]
        direction TB
        POSE["1. Zufaellige Roboter-Pose<br/>● Gelenkwinkel randomisieren<br/>● Armature Bones rotieren"]

        subgraph DR["2. Domain Randomization"]
            DR_CAM["Kamera<br/>± 3cm Position<br/>± 2° Rotation"]
            DR_LIGHT["Beleuchtung<br/>0.4x - 2.5x Intensitaet<br/>± 0.15 Farbverschiebung"]
            DR_BG["Hintergrund<br/>Zufaellige Farbe<br/>0.01 - 0.25 Helligkeit"]
            DR_EXP["Belichtung<br/>-2.0 bis +1.5 EV"]
        end

        RENDER_RGB["3. RGB-Render (EEVEE)<br/>640x640 PNG"]
        NOISE["4. Post-Render Noise (numpy)<br/>Gauss σ = 0.0 - 0.12<br/>Auf RGB anwenden"]

        RESET_EXP["5. Belichtung zuruecksetzen<br/>exposure = 0.0"]

        RENDER_MASK["6. Masken-Render (Workbench)<br/>Farbkodiert, sauber<br/>OHNE Augmentierung"]

        LABELS["7. Labels generieren<br/>● YOLO-Pose-Label (.txt)<br/>  9 Keypoints + BBox<br/>● CSV-Zeile (Gelenkwinkel)"]

        RESTORE["8. Restore<br/>Kamera, Licht, World, Exposure"]

        POSE --> DR
        DR --> RENDER_RGB --> NOISE --> RESET_EXP --> RENDER_MASK --> LABELS --> RESTORE
    end

    subgraph OUTPUT_RAW["Blender-Output (dataset/)"]
        OUT_IMG["images/<br/>image_0000.png .. image_N.png<br/>(RGB + Noise + Domain Rand.)"]
        OUT_MASK["masks/<br/>mask_0000.png .. mask_N.png<br/>(Saubere Farbmasken)"]
        OUT_POSE["labels_pose/<br/>image_0000.txt .. image_N.txt<br/>(YOLO-Pose 9 KP)"]
        OUT_CSV["joint_data.csv<br/>(Gelenkwinkel Ground Truth)"]
    end

    subgraph KONV_VIRTUELL["Umrisse_in_Polygone.py [1] Virtuell"]
        MASK_HSV["Masken → HSV-Analyse<br/>● Farbkodierung:<br/>  Rot=Base, Gruen=J1<br/>  Blau=J2, Gelb=J3<br/>  Magenta=J4"]
        CONTOUR["Konturen extrahieren<br/>● cv2.findContours()<br/>● Polygon-Koordinaten"]
        YOLO_LABEL["YOLO-Seg-Labels schreiben<br/>● Klasse + normalisierte Polygone<br/>● Format: cls x1 y1 x2 y2 ..."]
        SPLIT["Train/Val Split (70/30)<br/>● Bilder kopieren<br/>● Labels kopieren<br/>● YAML-Konfig erstellen"]

        MASK_HSV --> CONTOUR --> YOLO_LABEL --> SPLIT
    end

    subgraph OUTPUT_YOLO["YOLO-Trainingsformat"]
        YOLO_SEG["yolo_dataset/<br/>├ images/train + val/<br/>├ labels/train + val/<br/>└ dataset.yaml"]
        YOLO_POSE["pose_dataset/<br/>├ images/train + val/<br/>├ labels/train + val/<br/>└ pose_dataset.yaml"]
    end

    subgraph KONV_REAL["Umrisse_in_Polygone.py [2] Echt"]
        REAL_IMG["Echte Kamera-Bilder<br/>(Image_Capture_Python)"]
        AUTO_SEG["Auto-Annotation<br/>● Blender-Seg-Modell laden<br/>● Inferenz auf echte Bilder<br/>● Masken → YOLO-Seg-Labels"]
        SEG2POSE["Seg → Pose ableiten<br/>● Centroids berechnen<br/>● Keypoint-Labels generieren"]
        SPLIT_R["Train/Val Split<br/>● real_dataset/ + real_pose_dataset/<br/>● YAML-Konfigs"]

        REAL_IMG --> AUTO_SEG --> SEG2POSE --> SPLIT_R
    end

    subgraph OUTPUT_REAL["YOLO Real-Format"]
        REAL_SEG["real_dataset/<br/>├ images/all/<br/>└ labels/all/"]
        REAL_POSE_DS["real_pose_dataset/<br/>├ images/train + val/<br/>├ labels/train + val/<br/>└ real_pose_dataset.yaml"]
    end

    %% Verbindungen
    DIRS --> LOOP
    LOOP --> OUT_IMG & OUT_MASK & OUT_POSE & OUT_CSV
    OUT_MASK --> MASK_HSV
    OUT_IMG --> SPLIT
    OUT_POSE --> SPLIT
    SPLIT --> YOLO_SEG & YOLO_POSE
    SPLIT_R --> REAL_SEG & REAL_POSE_DS

    %% Styling
    style BLENDER_INIT fill:#e3f2fd,stroke:#1565c0,stroke-width:2px
    style LOOP fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px
    style DR fill:#fff8e1,stroke:#f9a825,stroke-width:1px
    style OUTPUT_RAW fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px
    style KONV_VIRTUELL fill:#fff3e0,stroke:#e65100,stroke-width:2px
    style OUTPUT_YOLO fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px
    style KONV_REAL fill:#fce4ec,stroke:#c62828,stroke-width:2px
    style OUTPUT_REAL fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px
```

### Farbkodierung der Segmentierungsmasken

| Klasse | Farbe | HSV-Bereich (H) | Roboterteil |
|--------|-------|------------------|-------------|
| 0 | Rot | 0-10 / 170-179 | Base |
| 1 | Gruen | 40-85 | Joint 1 |
| 2 | Blau | 90-135 | Joint 2 |
| 3 | Gelb | 20-35 | Joint 3 |
| 4 | Magenta | 140-165 | Joint 4 (Finger) |

### Domain Randomization Parameter

| Parameter | Bereich | Zweck |
|-----------|---------|-------|
| Kamera Position | ± 3 cm | Leichte Perspektivverschiebung |
| Kamera Rotation | ± 2° | Kippvariation |
| Licht Intensitaet | 0.4x - 2.5x | Helle/dunkle Szenen |
| Licht Farbe | ± 0.15 RGB | Farbtemperatur-Variation |
| Hintergrund | 0.01 - 0.25 | Dunkle bis helle Hintergruende |
| Belichtung | -2.0 bis +1.5 EV | Unter-/Ueberbelichtung |
| Gauss-Noise | σ 0.0 - 0.12 | Kamerarauschen simulieren |

---

## 3. Detail-Diagramm: Training, Validierung und Export

Detaillierter Ablauf des YOLO-Trainings in beiden Modi (Virtuell/Echt), der Validierung und des Exports fuer Edge-Deployment.

```mermaid
flowchart TB
    subgraph INPUTS["Trainings-Eingaben"]
        YOLO_DS["yolo_dataset/<br/>(Blender Seg)"]
        POSE_DS["pose_dataset/<br/>(Blender Pose)"]
        REAL_DS["real_dataset/<br/>(Echte Seg)"]
        REAL_POSE["real_pose_dataset/<br/>(Echte Pose)"]
        BASE_SEG["yolo11n-seg.pt<br/>(YOLO Base Weights)"]
        BASE_POSE["yolo11n-pose.pt<br/>(YOLO Base Weights)"]
    end

    subgraph TRAIN_V["EdgeAI.py [1] Virtuell — Pre-Training"]
        V_SEG["Phase 1: Seg-Training<br/>● YOLO11n-seg auf Blender-Daten<br/>● 640px, Batch 16<br/>● Val Split 30%"]
        V_POSE["Phase 2: Pose-Training<br/>● YOLO11n-pose auf Blender-Daten<br/>● 9 Keypoints (flip_idx konfiguriert)<br/>● Val Split 30%"]
        V_OUT_SEG["roboterarm_seg_vXXX/<br/>weights/best.pt"]
        V_OUT_POSE["roboterarm_pose_vXXX/<br/>weights/best.pt"]

        V_SEG --> V_OUT_SEG
        V_POSE --> V_OUT_POSE
    end

    subgraph TRAIN_R["EdgeAI.py [2] Echt — Fine-Tuning"]
        R_SEG["Phase 1: Seg Fine-Tuning<br/>● Blender-Modell als Basis<br/>● Fine-Tune auf echte Bilder<br/>● Transfer Learning"]
        R_POSE["Phase 2: Pose Fine-Tuning<br/>● Blender-Pose als Basis<br/>● Fine-Tune auf echte Pose-Daten"]
        R_OUT_SEG["roboterarm_real_vXXX/<br/>weights/best.pt"]
        R_OUT_POSE["roboterarm_pose_real_vXXX/<br/>weights/best.pt"]

        R_SEG --> R_OUT_SEG
        R_POSE --> R_OUT_POSE
    end

    subgraph OPTIM["Optionaler Optimierungsmodus"]
        PRUNE["Pruning (20%)<br/>● Kanaele entfernen<br/>● Modell komprimieren"]
        RECOVERY["Recovery-Training<br/>● 20 Epochen Nachtraining<br/>● Genauigkeit wiederherstellen"]
        PRUNE --> RECOVERY
    end

    subgraph RUNS["Run-Management"]
        INDEX["runs_index.json<br/>● latest: aktuellste Runs<br/>● history: alle Versionen<br/>● Format: base_vXXX_YYYYMMDD"]
    end

    subgraph VAL_NOTEBOOK["Validierung.ipynb — Auswertung"]
        direction TB
        VAL_CURVES["1-3. Training Curves<br/>● Box/Seg/Cls/DFL Loss<br/>● mAP50, mAP50-95<br/>● Precision, Recall"]
        VAL_CM["4-5. Confusion Matrix<br/>● Absolute + Normalized<br/>● PR-Kurven (Mask)"]
        VAL_BATCH["6-7. Batch-Vergleich<br/>● Ground Truth vs. Predictions<br/>● Trainings-Stichproben"]
        VAL_INFER["8-10. Inferenz-Tests<br/>● Blender Seg auf Blender-Val<br/>● Real Seg auf echte Bilder"]

        subgraph VAL_ANGLES["12-15. Winkelberechnung"]
            SEG_ANGLES["Seg → Centroids → Winkel<br/>● 5 Masken → 5 Schwerpunkte<br/>● Schwerpunkt-Kette → 4 Winkel<br/>● Auf echten + Blender-Bildern"]
            POSE_ANGLES["Pose → Keypoints → Winkel<br/>● 9 Keypoints direkt<br/>● Armkette → 4 Winkel<br/>● Praeziser als Centroid-Ansatz"]
        end

        VAL_CURVES --> VAL_CM --> VAL_BATCH --> VAL_INFER --> VAL_ANGLES
    end

    subgraph EXPORT_FLOW["EdgeAI.py [4] Export + Quantisierung"]
        ONNX["ONNX-Export<br/>● Float32-Modell<br/>● Plattformunabhaengig"]
        CALIB_DATA["npzConverter.py<br/>● Bilder laden + Letterbox<br/>● BGR → RGB, Normalisieren<br/>● Array (N,3,640,640) als .npz"]
        INT8["INT8-Quantisierung<br/>● 4x kleiner<br/>● Schneller auf MCU<br/>● Mit Kalibrierungsdaten"]
        STM32["Edge Export/<br/>● ONNX-Datei<br/>● Fuer STM32Cube.AI"]

        ONNX --> INT8
        CALIB_DATA --> INT8
        INT8 --> STM32
    end

    subgraph ANGLE_CALC["Winkelberechnungs-Logik"]
        UP["Referenz: up = (0, -1)<br/>(Vertikale nach oben)"]
        J1_ANG["J1 = angle(up, Base→J1)<br/>Neigung zur Vertikalen"]
        J2_ANG["J2 = angle(Base→J1, J1→J2)<br/>Biegung am Ellbogen 1"]
        J3_ANG["J3 = angle(J1→J2, J2→J3)<br/>Biegung am Ellbogen 2"]
        J4_ANG["J4 = angle(J3→J4, J4→TCP)<br/>Gripper-Winkel"]
        UP --> J1_ANG --> J2_ANG --> J3_ANG --> J4_ANG
    end

    %% Verbindungen
    BASE_SEG --> V_SEG
    YOLO_DS --> V_SEG
    BASE_POSE --> V_POSE
    POSE_DS --> V_POSE

    V_OUT_SEG -->|"Pre-trained"| R_SEG
    V_OUT_POSE -->|"Pre-trained"| R_POSE
    REAL_DS --> R_SEG
    REAL_POSE --> R_POSE

    V_SEG -.->|"optional"| OPTIM
    R_SEG -.->|"optional"| OPTIM

    V_OUT_SEG & V_OUT_POSE & R_OUT_SEG & R_OUT_POSE --> INDEX
    R_OUT_SEG & R_OUT_POSE --> VAL_NOTEBOOK
    V_OUT_SEG & V_OUT_POSE --> VAL_NOTEBOOK

    R_OUT_SEG --> ONNX
    R_OUT_POSE --> ONNX

    %% Styling
    style INPUTS fill:#e3f2fd,stroke:#1565c0,stroke-width:2px
    style TRAIN_V fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px
    style TRAIN_R fill:#c8e6c9,stroke:#1b5e20,stroke-width:2px
    style OPTIM fill:#fff8e1,stroke:#f9a825,stroke-width:1px,stroke-dasharray:5
    style RUNS fill:#f5f5f5,stroke:#616161,stroke-width:1px
    style VAL_NOTEBOOK fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px
    style VAL_ANGLES fill:#ede7f6,stroke:#4527a0,stroke-width:1px
    style EXPORT_FLOW fill:#fce4ec,stroke:#c62828,stroke-width:2px
    style ANGLE_CALC fill:#e0f7fa,stroke:#00838f,stroke-width:1px
```

### Modell-Versionen und Runs

Das Run-Management in `EdgeAI.py` versioniert alle Trainingslaeufe automatisch:

```
Trained_Models/AI-Models/
├── roboterarm_seg_v001_20260416/      ← Blender Seg (1. Lauf)
│   └── weights/best.pt
├── roboterarm_seg_v006_20260328/      ← Blender Seg (aktuellster)
│   └── weights/best.pt
├── roboterarm_pose_v006_20260328/     ← Blender Pose
│   └── weights/best.pt
├── roboterarm_real_v008_20260324/     ← Real Seg (Fine-Tuned)
│   └── weights/best.pt
├── roboterarm_pose_real_v006_20260324/← Real Pose (Fine-Tuned)
│   └── weights/best.pt
└── runs_index.json                    ← Index aller Laeufe
```

### Zwei Wege zur Winkelberechnung

| Methode | Quelle | Vorteile | Nachteile |
|---------|--------|----------|-----------|
| **Seg → Centroids** | Segmentierungsmasken | Robust, funktioniert mit weniger Trainingsdaten | Centroid-Position abhaengig von Maskenform |
| **Pose → Keypoints** | Pose-Keypoints direkt | Praeziser, direkte Gelenkpositionen | Benoetigt mehr Trainingsdaten, empfindlicher |

### Trainings-Parameter

| Parameter | Wert | Beschreibung |
|-----------|------|--------------|
| `IMG_SIZE` | 640 | Bildgroesse fuer Training und Inferenz |
| `BATCH_SIZE` | 16 | Batch-Groesse pro GPU |
| `VAL_SPLIT` | 0.3 | 30% Validierung, 70% Training |
| `SEED` | 42 | Reproduzierbarkeit |
| `PRUNE_RATIO` | 0.20 | Pruning: 20% der Kanaele (optional) |
| `PRUNE_RECOVERY_EPOCHS` | 20 | Recovery nach Pruning (optional) |
| `CALIBRATION_IMAGES` | 64 | Bilder fuer INT8-Kalibrierung |
