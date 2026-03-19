################################################################################
# Skript zur automatischen Robotersteuerung + Bildaufnahme für CNN-Training
# Kommunikation: UDP (Roboter) + USB CDC (STM32 Kamera)
################################################################################

import socket      # Für Netzwerkkommunikation (UDP)
import struct      # Für Binärdaten-Packing
import math        # Mathematische Funktionen (z.B. Winkel)
import time        # Zeitfunktionen (Pause, Timer)
import random      # Zufallsgenerator für Positionen
import os          # Betriebssystem-Funktionen
import csv         # CSV-Schreib-Funktionen
from datetime import datetime  # Zeitstempel
from itertools import product  # Für kartesisches Produkt aller Joint-Winkel
import serial      # Für serielle Kommunikation mit STM32
import numpy as np             # Array-Verarbeitung
from PIL import Image          # Bildverarbeitung / Speichern

###############################
# --- KONFIGURATION ---

# --- STM32 COM-Port Konfiguration ---
COM_PORT = "COM4"            # COM-Port für STM32 (leicht änderbar)
BAUDRATE = 115200            # Baudrate für STM32
SERIAL_TIMEOUT = None        # Blocking Mode
HEARTBEAT_TIMEOUT = 30       # Max. Sekunden auf STM32 Lebenszeichen warten
HEARTBEAT_KEYWORD = "READY"  # Erwartetes Lebenszeichen vom STM32

# --- Bild-Protokoll Konstanten ---
IMG_WIDTH = 800               # Breite des Kamerabilds (LCD-Framebuffer)
IMG_HEIGHT = 480              # Höhe des Kamerabilds
IMG_BPP = 2                   # Bytes pro Pixel (RGB565)
IMG_PAYLOAD_SIZE = IMG_WIDTH * IMG_HEIGHT * IMG_BPP  # 768000 Bytes
IMG_HEADER_MAGIC = b"IMG:"    # Magisches Header-Prefix
IMG_HEADER_SIZE = 8           # 4 Bytes Magic + 4 Bytes Payload-Länge
CAPTURE_TIMEOUT = 6          # Max. Sekunden auf ein Bild warten (inkl. ~0.5s STM32 settle)

# --- Ausgabe-Verzeichnisse ---
OUTPUT_DIR = "dataset"        # Hauptverzeichnis für Trainingsdaten
IMAGES_DIR = os.path.join(OUTPUT_DIR, "images")  # Bilder-Ordner
CSV_FILE   = os.path.join(OUTPUT_DIR, "labels.csv")  # CSV-Datei

UDP_IP = "192.168.4.152"    # Ziel-IP des Roboters (optional, falls UDP noch genutzt wird)
UDP_PORT = 5005              # Ziel-Port
UDP_FORMAT_SEND = "!4f"      # Format für 4 floats (Joint-Winkel)

# Zeitparameter

COUNTDOWN_SEC = 3            # Countdown vor Start

# Joint-Grenzen (in Grad)
JOINT_LIMITS = [
    {'min':-95.0, 'max': 95.0},   # Joint 1
    {'min': 0.0,  'max': 95.0},   # Joint 2
    {'min':-80.0, 'max': 80.0},   # Joint 3
    {'min':-10.0, 'max': 50.0}    # Joint 4
]

STEP_SIZE_DEG = 1.0  # Schrittweite in Grad
num_positions=1000
max_jump_deg=30.0

def frange(start, stop, step):
    """Hilfsfunktion: Erzeugt eine Liste von Werten von start bis stop mit Schrittweite step."""
    vals = []
    x = start
    while x <= stop + 1e-6:  # +1e-6 für Rundungsfehler
        vals.append(round(x, 1))
        x += step
    return vals

def wait_for_heartbeat(ser, keyword=HEARTBEAT_KEYWORD, timeout=HEARTBEAT_TIMEOUT):
    """Blockiert, bis das STM32-Board ein Lebenszeichen (z.B. 'READY') sendet.
       Gibt True zurück, wenn das Lebenszeichen empfangen wurde, sonst False."""
    print(f"[*] Warte auf Lebenszeichen ('{keyword}') vom STM32 über USB CDC (max {timeout}s)...")
    start = time.time()
    ser.timeout = 1  # Kurzes Timeout (1s) damit wir regelmäßig prüfen können
    ser.reset_input_buffer()  # Alte Daten im Puffer verwerfen
    while True:
        elapsed = time.time() - start
        if elapsed >= timeout:
            print(f"\n[!] TIMEOUT: Kein Lebenszeichen nach {timeout}s empfangen!")
            ser.timeout = SERIAL_TIMEOUT  # Original-Timeout wiederherstellen
            return False
        try:
            line = ser.readline()
            if not line:
                remaining = int(timeout - elapsed)
                print(f"    Warte... ({remaining}s verbleibend)", end="\r")
                continue
            decoded = line.decode(errors='ignore').strip()
            if decoded:
                print(f"    STM32 -> '{decoded}'")
            if keyword in decoded:
                print(f"[+] Lebenszeichen empfangen! STM32 ist bereit.")
                ser.timeout = SERIAL_TIMEOUT  # Original-Timeout wiederherstellen
                return True
        except serial.SerialException as e:
            print(f"[!] Serieller Fehler: {e}")
            ser.timeout = SERIAL_TIMEOUT
            return False
        except Exception as e:
            print(f"[!] Fehler beim Lesen: {e}")
            ser.timeout = SERIAL_TIMEOUT
            return False


def rgb565_to_rgb888(raw_data, width, height):
    """Konvertiert RGB565-Rohdaten (Little-Endian) in ein RGB888 numpy-Array."""
    # Rohdaten als uint16 Array interpretieren (Little-Endian)
    pixel_data = np.frombuffer(raw_data, dtype=np.uint16)
    pixel_data = pixel_data.reshape((height, width))

    # RGB565 -> RGB888 Extraktion
    r = ((pixel_data >> 11) & 0x1F).astype(np.uint8)  # 5 Bit Rot
    g = ((pixel_data >>  5) & 0x3F).astype(np.uint8)  # 6 Bit Grün
    b = ((pixel_data >>  0) & 0x1F).astype(np.uint8)  # 5 Bit Blau

    # Auf 8-Bit skalieren
    r = (r << 3) | (r >> 2)  # 5-Bit -> 8-Bit
    g = (g << 2) | (g >> 4)  # 6-Bit -> 8-Bit
    b = (b << 3) | (b >> 2)  # 5-Bit -> 8-Bit

    return np.stack([r, g, b], axis=-1)


def drain_serial(ser, drain_time=0.5):
    """Liest und verwirft alle pending Daten auf der seriellen Schnittstelle.
       Hilft nach fehlgeschlagenen Transfers, um den Zustand zu bereinigen."""
    ser.timeout = 0.1
    end_time = time.time() + drain_time
    total = 0
    while time.time() < end_time:
        chunk = ser.read(4096)
        if chunk:
            total += len(chunk)
        else:
            break
    if total > 0:
        print(f"    [drain] {total} Bytes verworfen")
    ser.timeout = SERIAL_TIMEOUT


def capture_image_from_stm32(ser, timeout=CAPTURE_TIMEOUT):
    """Sendet CAPTURE-Befehl an STM32 und empfängt das Bild als RGB565-Rohdaten.
       Gibt ein PIL.Image zurück oder None bei Fehler."""
    # Puffer leeren
    ser.reset_input_buffer()

    # CAPTURE-Befehl senden
    ser.write(b"CAPTURE\n")
    ser.flush()

    # --- Header empfangen (8 Bytes: "IMG:" + uint32 Länge) ---
    old_timeout = ser.timeout
    ser.timeout = timeout
    header = b""
    try:
        header = ser.read(IMG_HEADER_SIZE)
    except Exception as e:
        print(f"[!] Fehler beim Header-Empfang: {e}")
        ser.timeout = old_timeout
        return None

    if len(header) < IMG_HEADER_SIZE:
        print(f"[!] Header unvollständig ({len(header)}/{IMG_HEADER_SIZE} Bytes)")
        ser.timeout = old_timeout
        drain_serial(ser)
        return None

    # Magic prüfen
    if header[:4] != IMG_HEADER_MAGIC:
        print(f"[!] Ungültiger Header-Magic: {header[:4]}")
        ser.timeout = old_timeout
        drain_serial(ser)
        return None

    # Payload-Größe lesen (Little-Endian uint32)
    payload_size = struct.unpack('<I', header[4:8])[0]
    if payload_size != IMG_PAYLOAD_SIZE:
        print(f"[!] Unerwartete Payload-Größe: {payload_size} (erwartet {IMG_PAYLOAD_SIZE})")
        ser.timeout = old_timeout
        drain_serial(ser)
        return None

    # --- Bilddaten empfangen ---
    raw_data = b""
    remaining = payload_size
    start = time.time()
    last_progress = 0
    ser.timeout = 2  # Kurzes Timeout für einzelne Reads (nicht blockieren)
    while remaining > 0:
        chunk = ser.read(min(remaining, 4096))  # 4 KB Blöcke (matches STM32 chunk size)
        if not chunk:
            if time.time() - start > timeout:
                print(f"\n[!] Timeout beim Bilddaten-Empfang ({len(raw_data)}/{payload_size} Bytes)")
                ser.timeout = old_timeout
                drain_serial(ser)
                return None
            continue
        raw_data += chunk
        remaining -= len(chunk)
        # Fortschritt anzeigen alle 10%
        progress = int((len(raw_data) / payload_size) * 100)
        if progress >= last_progress + 10:
            print(f"      Empfange: {len(raw_data)}/{payload_size} ({progress}%)", end="\r")
            last_progress = progress
    print(f"      Empfangen: {len(raw_data)}/{payload_size} (100%)    ")

    ser.timeout = old_timeout

    # RGB565 -> RGB888 -> PIL Image
    rgb_array = rgb565_to_rgb888(raw_data, IMG_WIDTH, IMG_HEIGHT)
    img = Image.fromarray(rgb_array, 'RGB')
    return img


def generate_random_positions(max_jump_deg, num_positions):
    """Erzeugt eine Zufallssequenz von Positionen, wobei die maximale Differenz pro Achse max_jump_deg nicht überschreitet."""
    joint_ranges = []
    for joint in JOINT_LIMITS:
        joint_range = frange(joint['min'], joint['max'], STEP_SIZE_DEG)
        joint_ranges.append(joint_range)

    # Starte in Nullposition
    current = [0.0, 0.0, 0.0, 0.0]
    positions = [tuple(math.radians(a) for a in current)]
    for _ in range(num_positions-1):
        next_pos = []
        for i, val in enumerate(current):
            min_val = max(val - max_jump_deg, JOINT_LIMITS[i]['min'])
            max_val = min(val + max_jump_deg, JOINT_LIMITS[i]['max'])
            possible_vals = [v for v in joint_ranges[i] if min_val <= v <= max_val]
            next_val = random.choice(possible_vals)
            next_pos.append(next_val)
        current = next_pos
        positions.append(tuple(math.radians(a) for a in current))
    return positions


def main():
    print("\n" + "="*50)  # Trennlinie
    print("  ROBOTER-TEST + BILDAUFNAHME FÜR CNN-TRAINING")  # Titel
    print("="*50)  # Trennlinie

    label = input("[?] Welches Objekt nimmst du auf? (z.B. TEST): ").strip()  # Benutzerabfrage

    # --- Ausgabe-Verzeichnisse erstellen ---
    session_name = f"{label}_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    session_images_dir = os.path.join(IMAGES_DIR, session_name)
    os.makedirs(session_images_dir, exist_ok=True)
    csv_path = os.path.join(OUTPUT_DIR, f"labels_{session_name}.csv")
    print(f"[*] Bilder:  {session_images_dir}")
    print(f"[*] CSV:     {csv_path}")

    # Serielle Verbindung zum STM32 über USB CDC herstellen
    print(f"[*] Verbinde mit STM32 über USB CDC ({COM_PORT})...")
    try:
        stm_serial = serial.Serial(COM_PORT, BAUDRATE, timeout=SERIAL_TIMEOUT)
        print(f"[+] Verbindung zu {COM_PORT} erfolgreich.")
    except Exception as e:
        print(f"[!] Fehler beim Öffnen von {COM_PORT}: {e}")
        return

    # --- Warte auf Lebenszeichen vom STM32 über USB CDC ---
    if not wait_for_heartbeat(stm_serial):
        print("[!] STM32 hat kein Lebenszeichen gesendet. Abbruch.")
        stm_serial.close()
        return

    # UDP-Socket initialisieren
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # UDP-Socket

    # CSV-Datei vorbereiten
    csv_file = open(csv_path, 'w', newline='')
    csv_writer = csv.writer(csv_file)
    csv_writer.writerow(["image_file", "joint1_deg", "joint2_deg", "joint3_deg", "joint4_deg",
                         "joint1_rad", "joint2_rad", "joint3_rad", "joint4_rad", "label", "timestamp"])

    try:
        print(f"\n[*] Ziel-IP: {UDP_IP} | Modus: Sende + Capture")  # Info
        print(f"[*] Countdown startet...")  # Info
        for i in range(COUNTDOWN_SEC, 0, -1):  # Countdown
            print(f"    {i}...", end="\r")
            time.sleep(1)
        print("    SYSTEM START!             \n")  # Startmeldung

        # --- SICHERHEITS-PROZEDUR: NULLPOSITION ---
        print("\n" + "-"*50)  # Trennlinie
        print("[*] Fahre in Nullposition...")  # Info

        zero_angles = [0.0, 0.0, 0.0, 0.0]  # Nullposition
        zero_packet = struct.pack(UDP_FORMAT_SEND, *zero_angles)  # Packen

        # Sende Null-Befehl zur Sicherheit 3x
        for _ in range(3):
            sock.sendto(zero_packet, (UDP_IP, UDP_PORT))  # Senden
            time.sleep(0.05)  # Kurze Pause

        print("[*] Warte auf erreichen der Endposition (4s)...")  # Info
        time.sleep(4)  # 4 Sekunden warten bevor die eigentliche Testfahrt startet

        random_positions = generate_random_positions(max_jump_deg=max_jump_deg, num_positions=num_positions)
        print(f"[*] Anzahl zufälliger Positionen: {len(random_positions)}")  # Info
        start_time = time.time()  # Startzeit
        count = 0  # Zähler
        captured = 0  # Erfolgreich aufgenommene Bilder

        for pos_rad in random_positions:  # Für jede Zufalls-Position
            packet = struct.pack(UDP_FORMAT_SEND, *pos_rad)
            sock.sendto(packet, (UDP_IP, UDP_PORT))
            count += 1
            angles_deg = [round(math.degrees(a), 1) for a in pos_rad]
            angles_rad = [round(a, 6) for a in pos_rad]
            elapsed = int(time.time() - start_time)
            print(f"\n[{elapsed}s] Pose #{count:04d} gesendet: {angles_deg} Grad")
            time.sleep(4)  # Zeit Pause für Bewegung + Stabilisierung

            # --- BILD VOM STM32 AUFNEHMEN ---
            print(f"    Bild wird aufgenommen...", end=" ")
            img = capture_image_from_stm32(stm_serial)
            if img is not None:
                # Dateiname: pose_XXXX.png
                img_filename = f"pose_{count:04d}.png"
                img_path = os.path.join(session_images_dir, img_filename)
                img.save(img_path)
                captured += 1
                print(f"OK -> {img_filename} ({captured}/{count})")

                # CSV-Zeile schreiben
                timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                relative_img_path = os.path.join("images", session_name, img_filename)
                csv_writer.writerow([
                    relative_img_path,
                    angles_deg[0], angles_deg[1], angles_deg[2], angles_deg[3],
                    angles_rad[0], angles_rad[1], angles_rad[2], angles_rad[3],
                    label, timestamp
                ])
                csv_file.flush()  # Sofort schreiben (Absicherung gegen Abbruch)
            else:
                print(f"FEHLGESCHLAGEN!")

        print(f"\n[+] Alle {count} Positionen abgefahren! {captured} Bilder aufgenommen.")

    except KeyboardInterrupt:
        print("\n\n[!] ABBRUCH DURCH BENUTZER (STRG+C)")  # Abbruch durch Benutzer
    except Exception as e:
        print(f"\n\n[!] FEHLER IM SYSTEM: {e}")  # Fehlerausgabe

    finally:
        # --- SICHERHEITS-PROZEDUR: NULLPOSITION ---
        print("\n" + "-"*50)  # Trennlinie
        print("[*] Sicherheits-Check: Fahre in Nullposition...")  # Info

        zero_angles = [0.0, 0.0, 0.0, 0.0]  # Nullposition
        zero_packet = struct.pack(UDP_FORMAT_SEND, *zero_angles)  # Packen

        # Sende Null-Befehl zur Sicherheit 3x
        for _ in range(3):
            sock.sendto(zero_packet, (UDP_IP, UDP_PORT))  # Senden
            time.sleep(0.05)  # Kurze Pause

        print("[*] Warte auf mechanischen Abschluss (4s)...")  # Info
        time.sleep(4)  # Warten bis der Roboter sicher in der Nullposition ist

        sock.close()  # Socket schließen
        stm_serial.close()  # Serielle Verbindung schließen
        csv_file.close()  # CSV-Datei schließen
        print("[+] Roboter geparkt. Skript sicher beendet.")  # Abschlussmeldung
        print("-"*50 + "\n")  # Trennlinie

if __name__ == "__main__":
    main()  # Hauptfunktion starten