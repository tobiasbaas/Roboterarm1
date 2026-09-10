# Konzeption und Realisierung eines Edge-KI-Systems zur visuellen Posenschätzung eines Roboterarms

Herzlich willkommen zum Repo des Robotersarmes der Arbeitsgruppe. Bitte achtet es handelt sich hierbei 
um eine Erweiterung bereits vorhandener Infrastruktur. Für mehr Infos siehe das bisherige Github Repo:
- (https://github.com/mama1120/rpi_ros_can_module.git)

Sowie als auch das offiziele Wiki zu den vergangegen Projekten im Ilias:
- (https://ilias.h-ka.de/goto.php?target=wiki_787531#il_mhead_t_focus)

<br>

>In dieser Readme wird der Gesamte Aufbau des Roboterarmes inklusive der Erweiterung, die im Rahmen der 
>Bachelorthesis durchgeführt wurden, erläutert. Es soll einen Schnelleinstieg ermöglichen, indem ihr den 
>Roboter mithilfe des RaspberryPI, STM32N6 Board und eurem Laptop bewegen könnt. Für tieferes Verständis
>des Projektes navigiert in die Unterordner des Repo.

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
Nullposition ist dabei die Grundvoraussetzung.

---

## 2. Repository-Struktur

```
.
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

## 3. Schellstart

### 3.1 Voraussetzungen

**Hardware**

| Komponente | Anmerkung |
|---|---|
| Roboterarm mit CAN-Anbindung | 4 angesteuerte Gelenke |
| Raspberry Pi | ROS2 im Docker-Image, CAN-HAT |
| STM32N6570-DK | Kamera IMX335, LCD RK050HR18 |
| Laptop | Windows, MAC |

> [!IMPORTANT]
>Bittet achtet darauf, dass euer Laptop und der Raspberry im selben Netzwerk miteinander verbunden sind!

<br>

**Software auf dem Laptop**

Falls ihr noch kein Python auf euren Laptop verwendet habt, so installiert zuerst **Miniconda**
(https://www.anaconda.com/download/success) und erstellt ein Custom Environment für 
dieses Projekt. 

Dazu öffnet ihr das **Anaconda Powershell** und gibt folgenden Commands **nacheinander**
in das Terminal ein:

```bash
conda create --name Euer_Env_Name python=3.14
```

```bash
conda activate Euer_Env_Name
```

```bash
pip install ultralytics==8.4.146 torch==2.14.0 onnx==1.22.0 onnxruntime==1.29.0 opencv-python==5.0.0.93 numpy==2.4.6 pillow==12.3.0 pandas==3.0.5 matplotlib==3.11.1 pyserial==3.5 notebook==7.6.2 label-studio==1.23.0
```

Zusätzlich benötigt ihr für die Inbetriebnahme des STM32N6 Boards die folgende Software 
von ST-Microelectronics:

- STM32 VsCode Extension  (https://www.st.com/content/st_com/en/campaigns/stm32-vs-code-extension-z11.html)
- STM32 Cube CLT          (https://www.st.com/en/development-tools/stm32cubeclt.html)
- STM32 Cube Programmer   (https://www.st.com/en/development-tools/stm32cubeprog.html)
- STM32Cube AI Studio     (https://www.st.com/content/st_com/en/campaigns/edge-ai-toolchain-for-mcus-z14.html)

### 3.2 Schritt 1: Roboter am Raspberry Pi starten

> [!IMPORTANT]
> Das **Passwort** für den Pi selber und der Ausführung des Codes lautet: **`ubuntu`**.

Auf dem Raspberry Pi genügt **ein Terminal und ein Befehl**. Alle Docker-Workspaces
werden dabei im Hintergrund initialisiert:

```bash
./boot_robot.sh
```

Damit werden gestartet:

* CAN-Bus
* ROS2 inklusive aktiver UDP-Bridge
  

Optional, um die CAN-Nachrichten live mitzulesen (zweites Terminal, erst nach dem
Start des Roboters):

```bash
candump can0
```
> [!IMPORTANT]
> Den Roboter niemals gleichzeitig über RViz und über das Python-Skript
> ansprechen. 

### 3.3 Schritt 2: STM32N6570-DK vorbereiten

1. Boot-Pins **BOOT0 und BOOT1 auf Low** setzen (Flash-Mode, bevorzugte Betriebsart).
2. FSBL und Appli in der CubeIDE bauen.
3. Beide `.bin`-Dateien signieren, siehe [Abschnitt 4](#4-firmware-flashen-signierung).
4. External Flash löschen, dann **erst FSBL, danach Appli** flashen.
5. Board per USB an den Laptop stecken. Es meldet sich als virtueller COM-Port und sendet
   nach der Enumeration dreimal `READY`.

Für die reine Datenaufnahme wird `02_Software/New_Image_Capture` geflasht.

### 3.4 Schritt 3: Bildaufnahme starten

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


---

## 4. Firmware flashen: Signierung

Der STM32N6 führt nur signierte Images aus. FSBL und Appli müssen **getrennt** signiert
werden.
<br><br>

> [!IMPORTANT]
>Das Skript `sign_binaries.bat` im jeweiligen Firmware-Projekt automatisiert die folgenden Schritte.

<br>
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

---

## 5. Schnittstellen im Detail

### 5.1 Laptop zu Raspberry Pi (UDP)

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


---

## 6. Troubleshooting

| Symptom | Ursache und Abhilfe |
|---|---|
| `capture_image.py` bleibt bei "Warte auf Lebenszeichen" | Board sendet kein `READY`. Board resetten, COM-Port im Gerätemanager prüfen, ggf. Firmware nicht korrekt geflasht |
| Roboter reagiert nicht auf UDP | `boot_robot.sh` gestartet? UDP-Bridge aktiv? Laptop und Pi im selben WLAN? IP in `capture_image.py` korrekt? |
| Roboter fährt ruckartig oder unvorhersehbar | RViz läuft parallel. Nur eine Steuerquelle gleichzeitig verwenden |
| Bilder und Winkel passen nicht zusammen | Nullposition nicht korrekt angefahren, oder Wartezeit nach der Bewegung zu kurz |
| Board startet nach dem Flashen nicht | Signierung fehlt oder falsche Reihenfolge. External Flash löschen, erst FSBL, dann Appli |
| Bild kommt unvollständig an | `CAPTURE_TIMEOUT` erhöhen, anderes USB-Kabel oder anderen Port verwenden |

---


