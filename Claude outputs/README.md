# Lokalisierung eines Roboterarms mittels Edge-KI auf Embedded-Plattformen

Bachelorarbeit, Hochschule Karlsruhe, Fakultät für Maschinenbau und Mechatronik.

Dieses Repository enthält die komplette Toolkette, um die Gelenkwinkel eines Roboterarms
allein aus einem Kamerabild zu schätzen. Die Inferenz läuft nicht auf einem PC, sondern
direkt auf der NPU eines **STM32N6570-DK** (Edge-KI).

> **Für neue Studierende:** Abschnitt [2. Quickstart](#2-quickstart) reicht aus, um das
> bestehende System in Betrieb zu nehmen. Die Abschnitte danach erklären, warum es so
> aufgebaut ist und wo man weiterentwickeln kann.

---

## 1. Systemübersicht

Drei Geräte arbeiten zusammen. Der Laptop ist der Taktgeber, er steuert den Roboter und
löst die Bildaufnahme aus.

```
        ┌──────────────────────────────┐
        │  Laptop (Windows)            │
        │  capture_image.py            │
        └───────┬──────────────┬───────┘
                │              │
   UDP (WLAN)   │              │  USB CDC (virtueller COM-Port)
   4x float32   │              │  Kommandos: PING / TEST / CAPTURE
   Winkel [rad] │              │  Antwort: Bild 640x480 RGB565
                ▼              ▼
   ┌───────────────────┐   ┌──────────────────────────┐
   │ Raspberry Pi      │   │ STM32N6570-DK            │
   │ ROS2 + UDP-Bridge │   │ Kamera IMX335 + NPU      │
   │ CAN-Bus           │   │ Live-Bild auf LCD        │
   └────────┬──────────┘   └──────────────────────────┘
            │ CAN
            ▼
     ┌──────────────┐
     │ Roboterarm   │   Open-Loop, 4 Gelenke, keine Encoder
     └──────────────┘
```

**Warum dieser Aufbau?**
Der Roboterarm ist ein Open-Loop-System, es gibt keine Rotary-Encoder oder Hallsensoren,
die den tatsächlichen Winkel zurückmelden. Für das KI-Training muss aber jedes Bild exakt
zu einem bekannten Winkelsatz gehören. Deshalb wird der Roboter nicht über RViz von Hand
bewegt, sondern von einem eigenen Python-Skript auf eine bekannte Zielpose gefahren.
Das Skript wartet, bis die Bewegung sicher abgeschlossen ist, löst dann die Bildaufnahme
aus und schreibt Bild und Winkel gemeinsam in eine CSV-Datei. Eine korrekt angefahrene
Nullposition ist dabei die Grundvoraussetzung, sonst stimmen die absoluten Winkel nicht.

---

## 2. Quickstart

### 2.1 Voraussetzungen

**Hardware**

| Komponente | Anmerkung |
|---|---|
| Roboterarm mit CAN-Anbindung | 4 angesteuerte Gelenke |
| Raspberry Pi | ROS2 im Docker-Image, CAN-HAT |
| STM32N6570-DK | Kamera IMX335, LCD RK050HR18 |
| Laptop | Windows, WLAN im selben Netz wie der Pi, USB-Kabel zum Board |

**Software auf dem Laptop**

```bash
pip install pyserial numpy pillow ultralytics onnx onnxruntime opencv-python
```

Zusätzlich für die Firmware: STM32CubeIDE, STM32CubeProgrammer, STM32Cube AI Studio.

### 2.2 Schritt 1: Roboter am Raspberry Pi starten

Auf dem Raspberry Pi genügt **ein Terminal und ein Befehl**. Alle Docker-Workspaces
werden dabei im Hintergrund initialisiert:

```bash
./boot_robot.sh
```

Damit werden gestartet:

* CAN-Bus
* ROS2 inklusive **aktiver UDP-Bridge**

Fragt der Pi nach dem Passwort für die CAN-Kommunikation, lautet es `ubuntu`.

Optional, um die CAN-Nachrichten live mitzulesen (zweites Terminal, erst **nach** dem
Start des Roboters):

```bash
candump can0
```

> ⚠️ **Wichtig:** Den Roboter niemals gleichzeitig über RViz und über das Python-Skript
> ansprechen. Beide Quellen erzeugen konkurrierende Bewegungsbefehle, es gibt dann keinen
> definierten Bewegungsablauf mehr.

### 2.3 Schritt 2: STM32N6570-DK vorbereiten

1. Boot-Pins **BOOT0 und BOOT1 auf Low** setzen (Flash-Mode, bevorzugte Betriebsart).
2. FSBL und Appli in der CubeIDE bauen.
3. Beide `.bin`-Dateien signieren, siehe [Abschnitt 4](#4-firmware-flashen-signierung).
4. External Flash löschen, dann **erst FSBL, danach Appli** flashen.
5. Board per USB an den Laptop stecken. Es meldet sich als virtueller COM-Port und sendet
   nach der Enumeration dreimal `READY`.

Für die reine Datenaufnahme wird `02_Software/New_Image_Capture` geflasht,
für den Betrieb mit KI-Inferenz `02_Software/Thesis_Robot_Pose`.

### 2.4 Schritt 3: Bildaufnahme starten

In `02_Software/Image_Capture_Python/capture_image.py` oben die Konfiguration prüfen:

```python
COM_PORT = "COM4"              # COM-Port des STM32 im Gerätemanager nachsehen
UDP_IP   = "192.168.4.152"     # IP des Raspberry Pi
UDP_PORT = 5005
num_positions = 1000           # Anzahl aufzunehmender Posen
```

Dann starten:

```bash
cd 02_Software/Image_Capture_Python
python capture_image.py
```

Ablauf pro Bild: Zielpose per UDP senden, 4 s warten, `CAPTURE` über USB senden,
Bild empfangen, als PNG speichern, Zeile in `labels.csv` schreiben.

Ergebnis liegt in `dataset/images/<lauf_zeitstempel>/pose_XXXX.png` plus `dataset/labels.csv`.

### 2.5 Schritt 4: Trainieren

```bash
cd 02_Software/Beachlor_Thesis_AI/Training_Scripts
python Umrisse_in_Polygone.py    # Datenaufbereitung
python EdgeAI.py                 # Training, Menü mit 4 Schritten
```

Details siehe `Training_Scripts/README.md`.

### 2.6 Schritt 5: Modell auf das Board bringen

ONNX-Export aus `EdgeAI.py 4 --quantize`, danach Import in STM32Cube AI Studio und
Übernahme des generierten Netzwerkcodes nach `Thesis_Robot_Pose/Appli/AI/generated`.
Siehe `02_Software/Thesis_Robot_Pose/README.md`.

---

## 3. Schnittstellen im Detail

### 3.1 Laptop zu Raspberry Pi (UDP)

| Eigenschaft | Wert |
|---|---|
| Protokoll | UDP, unidirektional (Laptop sendet) |
| Ziel | `192.168.4.152:5005` |
| Nutzdaten | 4 x `float32`, Big-Endian (`struct` Format `!4f`) |
| Einheit | **Radiant**, Reihenfolge Joint 1 bis Joint 4 |
| Gelenkgrenzen | J1 -95° bis 95°, J2 0° bis 95°, J3 -80° bis 80°, J4 -10° bis 50° |

Vor dem Start fährt das Skript dreimal die Nullposition `[0,0,0,0]` an und wartet 4 s.

**Änderung am Raspberry Pi:** Die UDP-Bridge wurde in
`robot_moveit_config/launch/planning_execution.launch.py` ergänzt. Die betroffenen Stellen
sind in der Datei markiert, weitere Anpassungen sind für Folgeprojekte nicht nötig.
Aktiviert wird sie über den Launch-Parameter:

```bash
ros2 launch robot_moveit_config planning_execution.launch.py use_udp_bridge:=true
```

`boot_robot.sh` setzt diesen Parameter bereits. Der normale ROS2-Workflow bleibt
unverändert, RViz funktioniert weiterhin (aber nicht gleichzeitig, siehe Warnung oben).

### 3.2 Laptop zu STM32 (USB CDC)

| Eigenschaft | Wert |
|---|---|
| Transport | USBX Device, CDC ACM, virtueller COM-Port, 115200 Baud |
| Lebenszeichen | Board sendet nach Enumeration 3x `READY` |
| Kommandos | `PING` (Antwort `PONG`), `TEST`, `CAPTURE` |
| Bildformat | 640 x 480, RGB565, 2 Byte pro Pixel |
| Header | `IMG:` (4 Byte) + Payload-Länge (4 Byte) |
| Payload | 768000 Byte Rohdaten |
| Timeout | 6 s pro Bild, inklusive ca. 0,5 s Settle-Zeit des Boards |

Der Framebuffer wird für die Übertragung eingefroren, so dass Bild und Winkel garantiert
zusammenpassen.

### 3.3 tf2-Transformationen (optional)

Die ROS2 Transform-Listener-Funktion hält die Koordinateninformationen über einen Zeitraum
vor. Diese Werte werden ebenfalls per UDP an das steuernde Endgerät übergeben und können
später als zusätzliche Referenz beim Training genutzt werden. Das ist gerade bei einem
System ohne interne Sensorik hilfreich.

---

## 4. Firmware flashen: Signierung

Der STM32N6 führt nur signierte Images aus. FSBL und Appli müssen **getrennt** signiert
werden.

1. In der CubeIDE beide Umgebungen bauen. Die `.bin` liegt im `Debug`-Ordner des jeweiligen
   Projekts.
2. Eingabeaufforderung öffnen und in dieses Verzeichnis wechseln:

   ```bat
   cd /d <Pfad zum Debug-Ordner>
   ```

3. Signieren:

   ```bat
   STM32_SigningTool_CLI.exe -bin <name>.bin -t fsbl -la 0x80000000 -hv 2.3 -nk -o <name>-trusted.bin -align -dump <name>-trusted.bin
   ```

   Das Ergebnis der Signierung steht **ganz oben** in der Ausgabe, nicht am Ende.
4. Die entstandene `*-trusted.bin` mit dem STM32CubeProgrammer flashen.
   Reihenfolge: External Flash löschen, dann FSBL, dann Appli.

Das Skript `sign_binaries.bat` im jeweiligen Firmware-Projekt automatisiert diese Schritte.

---

## 5. Repository-Struktur

```
.
├── 01_Dokumentation/           # Arbeit, Anleitungen, Messprotokolle
├── 02_Software/
│   ├── Beachlor_Thesis_AI/     # KI-Pipeline (Training, Datenaufbereitung, Export)
│   ├── Image_Capture_Python/   # Host-Skripte: Robotersteuerung + Bildaufnahme
│   ├── New_Image_Capture/      # STM32-Firmware: nur Kamera, LCD, USB CDC
│   └── Thesis_Robot_Pose/      # STM32-Firmware: zusätzlich KI-Inferenz auf der NPU
├── 03_Hardware/
│   ├── STM32N6_Thesis_Board/   # Eigenes KiCad-Board (STM32N657X0H3Q)
│   └── Nucleo_PCB_FIles/       # Altium-Daten des MB1940-Referenzdesigns
└── README.md                   # diese Datei
```

| Ordner | Einstiegspunkt |
|---|---|
| `Beachlor_Thesis_AI` | `Training_Scripts/README.md`, `EdgeAI.py` |
| `Image_Capture_Python` | `capture_image.py` |
| `New_Image_Capture` | `README.md`, `FSBL/Core/Src/main.c` |
| `Thesis_Robot_Pose` | `README.md`, `Appli/AI/App/app_x-cube-ai.c` |

---

## 6. Das KI-Modell in Kurzform

* **Aufgabe:** Segmentierung bzw. Pose-Erkennung von 5 Sektionen des Arms.
  Klassen: `Base`, `Joint_1`, `Joint_2`, `Joint_3`, `Joint_4_Finger`.
* **Netz:** YOLO11n in den Varianten `-seg` und `-pose`, 9 Keypoints.
* **Training:** zweistufig. Erst Pre-Training auf synthetischen Blender-Daten für das
  Formwissen, dann Fine-Tuning auf echten Kamerabildern.
* **Edge-Optimierung:** Structured Pruning (20 %), kurzes Recovery-Training,
  ONNX-Export (opset 20), statische INT8-Quantisierung.
* **Auf dem Board:** Pose- und Segmentierungsnetz sind parallel eingebunden und zur
  Laufzeit umschaltbar. Das Postprocessing inklusive NMS und Overlay-Zeichnung läuft
  vollständig auf dem STM32.
* **Winkelberechnung:** Richtungsvektoren entlang der Kette
  `Base → Joint_1 → Joint_2 → Joint_3 → Joint_4 → TCP`.

---

## 7. Troubleshooting

| Symptom | Ursache und Abhilfe |
|---|---|
| `capture_image.py` bleibt bei "Warte auf Lebenszeichen" | Board sendet kein `READY`. Board resetten, COM-Port im Gerätemanager prüfen, ggf. Firmware nicht korrekt geflasht |
| Roboter reagiert nicht auf UDP | `boot_robot.sh` gestartet? UDP-Bridge aktiv? Laptop und Pi im selben WLAN? IP in `capture_image.py` korrekt? |
| Roboter fährt ruckartig oder unvorhersehbar | RViz läuft parallel. Nur eine Steuerquelle gleichzeitig verwenden |
| Bilder und Winkel passen nicht zusammen | Nullposition nicht korrekt angefahren, oder Wartezeit nach der Bewegung zu kurz |
| Board startet nach dem Flashen nicht | Signierung fehlt oder falsche Reihenfolge. External Flash löschen, erst FSBL, dann Appli |
| Bild kommt unvollständig an | `CAPTURE_TIMEOUT` erhöhen, anderes USB-Kabel oder anderen Port verwenden |

---

## 8. Weiterführende Dokumentation

| Thema | Datei |
|---|---|
| KI-Trainingspipeline im Detail | `02_Software/Beachlor_Thesis_AI/Training_Scripts/README.md` |
| Trainingsdokumentation | `.../EDGEAI_TRAINING_DOKUMENTATION.md` |
| Flussdiagramme der Pipeline | `.../KI_PIPELINE_FLUSSDIAGRAMME.md` |
| Label-Studio-Workflow | `.../label_studio_sync_ANLEITUNG.md` |
| Kamera, USB, Display, ThreadX | `02_Software/New_Image_Capture/README.md` |
| Boot, Speicherlayout, NPU-Integration | `02_Software/Thesis_Robot_Pose/README.md` |
| ROS2-Änderung am Raspberry Pi | `01_Dokumentation/Aenderung_ros2_launch_RaspberryPi.md` |
| Signierung der Binaries | `01_Dokumentation/Workflow_Signierte_bin.md` |

---

## 9. Bekannte Einschränkungen

* Der Roboterarm ist ein Open-Loop-System. Alle Winkellabels sind Sollwerte, keine
  gemessenen Istwerte.
* Die Trainingsdaten sind auf die Lichtverhältnisse und den Hintergrund des Laboraufbaus
  zugeschnitten. Bei einem neuen Aufbau ist ein erneutes Fine-Tuning nötig.
* Das Training läuft auf der CPU, eine GPU-Unterstützung ist nicht eingerichtet.

---

## Autor

Marcel, Bachelorarbeit Mechatronik, Hochschule Karlsruhe.
Betreuung: <Name Professor>.
