# Image_Capture_Python: Host-Laptop für die Aufnahme der Trainingsdaten

Das sind die Skripte, die auf eurem Laptop laufen. Es fährt den Roboterarm auf
zufällige Posen, lösen am STM32 eine Bildaufnahme aus und schreiben Bild und Winkel
gemeinsam in eine CSV-Datei. Das Ergebnis ist der Datensatz, mit dem die KI trainiert
wird.

## Inhalt

- [Image\_Capture\_Python: Host-Laptop für die Aufnahme der Trainingsdaten](#image_capture_python-host-laptop-für-die-aufnahme-der-trainingsdaten)
  - [Inhalt](#inhalt)
  - [1. Was diese Skripte machen](#1-was-diese-skripte-machen)
  - [2. Projektstruktur](#2-projektstruktur)
  - [3. Schnellstart](#3-schnellstart)
    - [3.1 Voraussetzungen](#31-voraussetzungen)
    - [3.2 Konfiguration anpassen](#32-konfiguration-anpassen)
    - [3.3 Aufnahme starten](#33-aufnahme-starten)
  - [4. Ablauf einer Aufnahmesession](#4-ablauf-einer-aufnahmesession)
  - [5. Die Skripte im Detail](#5-die-skripte-im-detail)
    - [5.1 capture\_image.py](#51-capture_imagepy)
    - [5.2 serial\_monitor.py](#52-serial_monitorpy)
  - [6. Wichtige Konstanten](#6-wichtige-konstanten)
  - [7. Troubleshooting](#7-troubleshooting)

---

## 1. Was diese Skripte machen


```
   ┌────────────────────────────────────────┐
   │  Laptop                                │
   │  capture_image.py                      │
   └──────┬──────────────────────┬──────────┘
          │                      │
   UDP    │                      │  USB CDC
   4x float32                    │  "CAPTURE"
   Winkel in Radiant             │
          ▼                      ▼
   ┌──────────────────┐   ┌──────────────────┐
   │  Raspberry Pi    │   │  STM32N6570-DK   │
   │  ROS2 + CAN      │   │  Kamera + LCD    │
   └────────┬─────────┘   └────────┬─────────┘
            │                      │
            ▼                      │  "IMG:" + 768000 Byte
      ┌───────────┐                │
      │ Roboterarm│                ▼
      └───────────┘         dataset/images/...
                            dataset/labels_*.csv
```

**Warum dieser Aufbau?**

Der Roboterarm hat keine Encoder, er meldet seine tatsächliche Stellung also nicht
zurück. Für das Training muss trotzdem jedes Bild exakt zu einem bekannten Winkelsatz
gehören. Deshalb übernimmt das Skript beide Rollen: es bestimmt die Zielpose, wartet
bis die Bewegung sicher abgeschlossen ist, und löst erst dann die Aufnahme aus. Weil es
den Winkel selbst vorgegeben hat, kennt es ihn auch.

>[!IMPORTANT]
>Die Nullposition muss vorher korrekt angefahren sein. Alles, was das Skript
>aufschreibt, ist ein Sollwert relativ zu dieser Nullposition. Stimmt sie nicht, sind
>alle Labels im Datensatz systematisch verschoben.

---

## 2. Projektstruktur

```
Image_Capture_Python/
├── capture_image.py        # Hauptskript: Roboter fahren + Bilder aufnehmen
├── serial_monitor.py       # Debug-Werkzeug für die USB-Verbindung
├── dataset/                # entsteht beim ersten Lauf
│   ├── images/
│   │   └── <LABEL>_<Zeitstempel>/
│   │       ├── pose_0001.png
│   │       └── ...
│   └── labels_<LABEL>_<Zeitstempel>.csv
└── README.md               # diese Datei
```

---

## 3. Schnellstart

### 3.1 Voraussetzungen

| Was | Anmerkung |
|---|---|
| Roboter läuft | `./boot_robot.sh` auf dem Raspberry Pi ist gestartet |
| STM32 ist geflasht | mit der Firmware aus `02_Software/New_Image_Capture` |
| USB-Kabel | Board am Laptop, meldet sich als virtueller COM-Port |
| Netzwerk | Laptop und Raspberry Pi im selben WLAN |
| Python-Umgebung | siehe Haupt-Readme, Abschnitt Voraussetzungen |

<br>

> [!IMPORTANT]
> Startet niemals gleichzeitig RViz und dieses Skript. Zwei Steuerquellen bedeuten
> unvorhersehbare Bewegungen.

### 3.2 Konfiguration anpassen

Alles Einstellbare steht oben in `capture_image.py`. Bevor ihr startet, prüft diese
vier Werte:

```python
COM_PORT = "COM4"            # COM-Port des STM32, im Gerätemanager nachsehen
UDP_IP   = "192.168.4.152"   # IP des Raspberry Pi
UDP_PORT = 5005
num_positions = 1000         # Anzahl der aufzunehmenden Posen
```

Den COM-Port findet ihr im Windows-Gerätemanager unter "Anschlüsse (COM & LPT)". Dort
stehen meist zwei Einträge: einer ist das Board, der andere der ST-Link. Wenn ihr
unsicher seid, nehmt `serial_monitor.py` und schickt ein `PING`, siehe Abschnitt 5.2.

### 3.3 Aufnahme starten

```bash
cd 02_Software/Image_Capture_Python
python capture_image.py
```

Das Skript fragt euch zuerst nach einem Namen für die Session:

```
[?] Welches Objekt nimmst du auf? (z.B. TEST):
```

Daraus entsteht der Ordnername, zum Beispiel `2000bilder_20260327_074923`. Wählt den Namen so, 
dass ihr diesen leicht zuordnen könnt.

>[!IMPORTANT]
>Mit **Strg+C** könnt ihr jederzeit abbrechen,
>der Roboter fährt dabei trotzdem sauber in die Nullposition zurück.

---

## 4. Ablauf einer Aufnahmesession

```
1. Session-Name abfragen           Ordner und CSV-Datei anlegen
2. COM-Port öffnen                 Verbindung zum STM32
3. Auf "READY" warten              max. 30 s, sonst Abbruch
4. UDP-Socket öffnen               Verbindung zum Raspberry Pi
5. Countdown 3 Sekunden            Zeit, die Hände wegzunehmen
6. Nullposition anfahren           3x senden, 4 s warten

   ── Schleife über alle Posen ──────────────────────────
   7.  Zielpose per UDP senden
   8.  4 s warten                  Bewegung + Stabilisierung
   9.  "CAPTURE" über USB senden
   10. Header und Bilddaten empfangen
   11. RGB565 zu RGB888 wandeln, als PNG speichern
   12. CSV-Zeile schreiben und sofort auf Platte schreiben
   ──────────────────────────────────────────────────────

13. Nullposition anfahren          auch bei Abbruch oder Fehler
14. Verbindungen schließen
```

Die Schritte 13 und 14 stehen in einem `finally`-Block. Sie laufen also auch dann,
wenn ihr mit Strg+C abbrecht oder ein Fehler auftritt.

**Die CSV-Datei** bekommt für jedes erfolgreiche Bild eine Zeile:

| Spalte | Inhalt |
|---|---|
| `image_file` | Pfad relativ zu `dataset/`, zum Beispiel `images/TEST_2026.../pose_0042.png` |
| `joint1_deg` bis `joint4_deg` | Zielwinkel in Grad, auf eine Nachkommastelle gerundet |
| `joint1_rad` bis `joint4_rad` | dieselben Winkel in Radiant |
| `label` | der Session-Name, den ihr eingegeben habt |
| `timestamp` | Zeitpunkt der Aufnahme |

Nach jeder Zeile wird `flush()` aufgerufen. Bricht der Lauf ab, sind alle bis dahin
aufgenommenen Bilder korrekt in der CSV verzeichnet.

---

## 5. Die Skripte im Detail

### 5.1 capture_image.py

**`wait_for_heartbeat()`** wartet nach dem Öffnen des Ports bis zu 30 Sekunden auf das
Schlüsselwort `READY`. Das Board sendet es dreimal nach der USB-Enumeration. Kommt
nichts, bricht das Skript ab, statt blind loszufahren.

**`generate_random_positions()`** erzeugt die Posenfolge. Das ist kein reiner Zufall,
sondern eine Zufallsbewegung mit Schrittbegrenzung:

```python
current = [0.0, 0.0, 0.0, 0.0]          # Start in der Nullposition
for _ in range(num_positions - 1):
    for i, val in enumerate(current):
        min_val = max(val - max_jump_deg, JOINT_LIMITS[i]['min'])
        max_val = min(val + max_jump_deg, JOINT_LIMITS[i]['max'])
        # ... zufälligen Wert aus diesem Fenster wählen
```

Jedes Gelenk darf sich pro Schritt um höchstens `max_jump_deg` (30 Grad) bewegen und
bleibt dabei innerhalb seiner Grenzen. Die möglichen Werte liegen auf einem 1-Grad-Raster.

**`capture_image_from_stm32()`** macht die eigentliche Übertragung:

1. Eingangspuffer leeren, `CAPTURE\n` senden
2. 8 Byte Header lesen und prüfen: Magic `IMG:` und Payload-Länge 768000
3. Bilddaten in 4-KB-Blöcken lesen, mit Fortschrittsanzeige alle 10 Prozent
4. RGB565 in RGB888 wandeln und als PIL-Image zurückgeben

Stimmt der Header nicht oder läuft der Empfang in einen Timeout, gibt die Funktion
`None` zurück und ruft vorher `drain_serial()` auf. Das verwirft alle noch anstehenden
Bytes, damit der nächste Versuch nicht auf einem verschobenen Datenstrom aufsetzt.

**`rgb565_to_rgb888()`** rechnet das Kameraformat in ein normales RGB-Bild um.

```python
r = (r << 3) | (r >> 2)   # 5 Bit auf 8 Bit
g = (g << 2) | (g >> 4)   # 6 Bit auf 8 Bit
```

### 5.2 serial_monitor.py

Ein Debug-Werkzeug für die USB-Verbindung. Damit könnt ihr von Hand Befehle schicken
und seht den kompletten Datenverkehr mit Zeitstempel, farbig getrennt nach Senden und
Empfangen, bei Binärdaten als Hex-Dump.

```bash
python serial_monitor.py              # Default: COM4, 115200
python serial_monitor.py COM5         # anderer Port
python serial_monitor.py COM4 921600  # andere Baudrate
```

Im laufenden Monitor tippt ihr einfach den Befehl ein und drückt Enter:

| Eingabe | Wirkung |
|---|---|
| `PING` | Board sollte mit `PONG` antworten |
| `CAPTURE` | sendet `CAPTURE`, dekodiert den `IMG:`-Header und zeigt die Bilddaten |
| `hex:AABB` | sendet rohe Hex-Bytes |
| `help` | zeigt die verfügbaren Befehle |
| `quit` | beendet den Monitor |

Das ist das erste Werkzeug, zu dem ihr greifen solltet, wenn `capture_image.py` nicht
läuft.

---

## 6. Wichtige Konstanten

Alle oben in `capture_image.py`:

| Konstante | Wert | Bedeutung |
|---|---|---|
| `COM_PORT` | `"COM4"` | COM-Port des STM32 |
| `BAUDRATE` | 115200 | bei USB CDC ohne echte Wirkung, muss aber gesetzt sein |
| `HEARTBEAT_TIMEOUT` | 30 | Sekunden Wartezeit auf `READY` |
| `HEARTBEAT_KEYWORD` | `"READY"` | erwartetes Lebenszeichen |
| `IMG_WIDTH` / `IMG_HEIGHT` | 640 / 480 | Bildgröße |
| `IMG_BPP` | 2 | RGB565 |
| `IMG_PAYLOAD_SIZE` | 768000 | 640 * 480 * 2 |
| `IMG_HEADER_MAGIC` | `b"IMG:"` | Kennung im Header |
| `CAPTURE_TIMEOUT` | 6 | Sekunden je Bild |
| `UDP_IP` / `UDP_PORT` | `192.168.4.152` / 5005 | Ziel am Raspberry Pi |
| `UDP_FORMAT_SEND` | `"!4f"` | 4 float32, Big-Endian |
| `COUNTDOWN_SEC` | 3 | Countdown vor dem Start |
| `JOINT_LIMITS` | siehe unten | Gelenkgrenzen in Grad |
| `STEP_SIZE_DEG` | 1.0 | Raster der möglichen Winkel |
| `num_positions` | 1000 | Anzahl der Posen |
| `max_jump_deg` | 30.0 | maximale Änderung je Gelenk und Schritt |

**Gelenkgrenzen:**

| Gelenk | Minimum | Maximum |
|---|---|---|
| Joint 1 | -95° | 95° |
| Joint 2 | 0° | 95° |
| Joint 3 | -80° | 80° |
| Joint 4 | -10° | 50° |

> [!IMPORTANT]
> Die Winkel gehen in **Radiant** über UDP, nicht in Grad. Die Grenzen oben sind in
> Grad angegeben, das Skript rechnet vor dem Senden mit `math.radians()` um.

---

## 7. Troubleshooting

| Symptom | Ursache und Abhilfe |
|---|---|
| "Warte auf Lebenszeichen" läuft in den Timeout | Board sendet kein `READY`. RESET am Board drücken, COM-Port prüfen, ggf. Firmware neu flashen |
| `Fehler beim Öffnen von COM4` | Falscher Port, oder ein anderes Programm hat ihn geöffnet. Serial Monitor oder Terminal schließen |
| Roboter reagiert nicht auf die Posen | Läuft `boot_robot.sh`? Stimmt `UDP_IP`? Sind Laptop und Pi im selben Netz? |
| Roboter fährt ruckartig oder unvorhersehbar | RViz läuft parallel. Nur eine Steuerquelle gleichzeitig |
| `Header unvollständig` oder `Ungültiger Header-Magic` | Der Datenstrom ist verschoben. Das Skript räumt selbst auf, tritt es dauerhaft auf, Board resetten |
| `Timeout beim Bilddaten-Empfang` | `CAPTURE_TIMEOUT` erhöhen, anderes USB-Kabel oder anderen Port probieren |
| Bilder und Winkel passen nicht zusammen | Nullposition stimmt nicht, oder die 4 Sekunden Wartezeit reichen für den Bewegungsumfang nicht aus |
| Viele Posen, aber wenig Bilder in der CSV | Übertragungen schlagen fehl. Die Konsole zeigt bei jedem Bild `OK` oder `FEHLGESCHLAGEN`, dort nachsehen |
