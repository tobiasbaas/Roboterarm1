#!/usr/bin/env python3
"""
Serial Monitor für STM32 CDC ACM (COM4)
========================================
Zeigt den gesamten Datenverkehr auf der seriellen Schnittstelle an,
inklusive Hex-Dump, Timestamps und Protokoll-Dekodierung.

Bedienung:
  - Tippe einen Befehl ein + Enter → wird an STM32 gesendet
  - "CAPTURE" → sendet CAPTURE\n und wartet auf IMG:-Header + Bilddaten
  - "quit" / Ctrl+C → Beendet den Monitor

Nutzung:
  python serial_monitor.py [COM_PORT] [BAUDRATE]
  python serial_monitor.py              # Default: COM4, 115200
  python serial_monitor.py COM5         # Anderer Port
  python serial_monitor.py COM4 921600  # Andere Baudrate
"""

import sys
import time
import struct
import threading
import serial

# ── Konfiguration ──────────────────────────────────────────────────────────
DEFAULT_PORT = "COM4"
DEFAULT_BAUD = 115200
READ_CHUNK   = 4096          # Max Bytes pro read()
HEX_LINE_LEN = 32            # Bytes pro Hex-Dump-Zeile
IMG_MAGIC    = b"IMG:"
IMG_HEADER_SIZE = 8           # 4 Bytes Magic + 4 Bytes Payload-Länge (LE)

# ANSI Farben (für Windows Terminal / VS Code Terminal)
C_RESET  = "\033[0m"
C_GREEN  = "\033[32m"         # TX (Senden)
C_CYAN   = "\033[36m"         # RX Text
C_YELLOW = "\033[33m"         # RX Hex
C_RED    = "\033[31m"         # Fehler
C_BLUE   = "\033[34m"         # Info
C_DIM    = "\033[2m"          # Gedimmt

# ── Hilfsfunktionen ───────────────────────────────────────────────────────

def ts():
    """Aktueller Timestamp als String."""
    return time.strftime("%H:%M:%S", time.localtime()) + f".{int(time.time()*1000)%1000:03d}"

def hex_dump(data: bytes, prefix: str = "       ", bytes_per_line: int = HEX_LINE_LEN):
    """Gibt einen formatierten Hex-Dump auf stdout aus."""
    for i in range(0, len(data), bytes_per_line):
        chunk = data[i:i+bytes_per_line]
        hex_part = " ".join(f"{b:02X}" for b in chunk)
        ascii_part = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
        offset = f"{i:06X}"
        print(f"{prefix}{C_DIM}{offset}{C_RESET}  {C_YELLOW}{hex_part:<{bytes_per_line*3}}{C_RESET} {C_DIM}|{ascii_part}|{C_RESET}")

def print_rx_text(data: bytes):
    """Versucht Daten als Text zu decodieren und gibt sie aus."""
    try:
        text = data.decode('utf-8', errors='replace').rstrip('\r\n')
        if text:
            print(f"  {C_CYAN}[RX TXT]{C_RESET} {text}")
    except Exception:
        pass

def decode_img_header(header: bytes):
    """Dekodiert einen IMG:-Header und gibt die Payload-Größe zurück."""
    if len(header) >= IMG_HEADER_SIZE and header[:4] == IMG_MAGIC:
        payload_size = struct.unpack('<I', header[4:8])[0]
        return payload_size
    return None


# ── Receive-Thread ─────────────────────────────────────────────────────────

class SerialMonitor:
    def __init__(self, port: str, baudrate: int):
        self.port = port
        self.baudrate = baudrate
        self.ser = None
        self.running = False
        self.waiting_for_image = False   # Flag: erwarten wir Bilddaten?
        self.image_remaining = 0         # Verbleibende Bilddaten-Bytes
        self.image_received = 0          # Bereits empfangene Bilddaten-Bytes
        self.image_start_time = 0.0
        self._rx_lock = threading.Lock()

    def open(self):
        """Öffnet die serielle Schnittstelle."""
        print(f"{C_BLUE}[INFO]{C_RESET} Öffne {self.port} @ {self.baudrate} baud ...")
        self.ser = serial.Serial(
            port=self.port,
            baudrate=self.baudrate,
            timeout=0.1,          # Nicht-blockierend für regelmäßiges Polling
            write_timeout=2,
        )
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()
        print(f"{C_BLUE}[INFO]{C_RESET} {self.port} geöffnet. DTR={self.ser.dtr}, RTS={self.ser.rts}")
        print(f"{C_BLUE}[INFO]{C_RESET} Tippe Befehle + Enter zum Senden. 'quit' zum Beenden.\n")

    def close(self):
        """Schließt die serielle Schnittstelle."""
        self.running = False
        if self.ser and self.ser.is_open:
            self.ser.close()
            print(f"\n{C_BLUE}[INFO]{C_RESET} {self.port} geschlossen.")

    def send(self, data: bytes):
        """Sendet Daten und zeigt sie im Monitor an."""
        if not self.ser or not self.ser.is_open:
            print(f"{C_RED}[ERR]{C_RESET} Port nicht offen!")
            return False
        print(f"  {C_GREEN}[TX {ts()}]{C_RESET} {len(data)} Bytes: {data!r}")
        if len(data) <= 64:
            hex_dump(data, prefix=f"  {C_GREEN}[TX HEX]{C_RESET} ")
        try:
            self.ser.write(data)
            self.ser.flush()
            return True
        except serial.SerialTimeoutException:
            print(f"  {C_RED}[TX TIMEOUT]{C_RESET} Write-Timeout! STM32 antwortet nicht.")
            return False
        except serial.SerialException as e:
            print(f"  {C_RED}[TX ERR]{C_RESET} Serieller Fehler: {e}")
            return False

    def rx_loop(self):
        """Empfangsschleife (läuft in eigenem Thread)."""
        # Buffer für Protokoll-Erkennung
        pending = b""

        while self.running:
            try:
                if not self.ser or not self.ser.is_open:
                    time.sleep(0.1)
                    continue

                # Image transfer timeout detection (15 seconds)
                if self.waiting_for_image and self.image_remaining > 0:
                    elapsed = time.time() - self.image_start_time
                    if elapsed > 15.0:
                        total = self.image_received + self.image_remaining
                        print(f"\n  {C_RED}[IMG TIMEOUT]{C_RESET} "
                              f"Bildtransfer abgebrochen nach {elapsed:.1f}s! "
                              f"{self.image_received}/{total} Bytes empfangen")
                        self.waiting_for_image = False
                        self.image_remaining = 0

                waiting = self.ser.in_waiting
                if waiting == 0:
                    time.sleep(0.01)  # Kurze Pause, kein Busy-Wait
                    continue

                data = self.ser.read(min(waiting, READ_CHUNK))
                if not data:
                    continue

                with self._rx_lock:
                    self._process_rx(data)

            except serial.SerialException as e:
                print(f"\n{C_RED}[ERR]{C_RESET} Serieller Fehler: {e}")
                self.running = False
                break
            except Exception as e:
                print(f"\n{C_RED}[ERR]{C_RESET} Unerwarteter Fehler: {e}")
                time.sleep(0.1)

    def _process_rx(self, data: bytes):
        """Verarbeitet empfangene Daten mit Protokoll-Erkennung."""
        timestamp = ts()

        # ── Fall 1: Wir empfangen gerade Bilddaten ──
        if self.waiting_for_image and self.image_remaining > 0:
            consumed = min(len(data), self.image_remaining)
            self.image_received += consumed
            self.image_remaining -= consumed

            # Fortschritt anzeigen (alle 10% oder am Ende)
            total = self.image_received + self.image_remaining
            pct = (self.image_received / total * 100) if total > 0 else 100
            elapsed = time.time() - self.image_start_time
            rate_kbps = (self.image_received / 1024) / elapsed if elapsed > 0 else 0

            print(f"  {C_CYAN}[RX IMG {timestamp}]{C_RESET} "
                  f"+{consumed} Bytes | "
                  f"{self.image_received}/{total} ({pct:.1f}%) | "
                  f"{rate_kbps:.1f} KB/s | "
                  f"{elapsed:.2f}s",
                  end="\r")

            if self.image_remaining <= 0:
                elapsed = time.time() - self.image_start_time
                rate_kbps = (self.image_received / 1024) / elapsed if elapsed > 0 else 0
                print(f"\n  {C_BLUE}[IMG DONE]{C_RESET} "
                      f"{self.image_received} Bytes empfangen in {elapsed:.2f}s "
                      f"({rate_kbps:.1f} KB/s)")
                self.waiting_for_image = False

            # Falls noch Rest-Bytes nach dem Bild übrig sind
            leftover = data[consumed:]
            if leftover:
                self._process_rx(leftover)
            return

        # ── Fall 2: Prüfe auf IMG:-Header ──
        img_idx = data.find(IMG_MAGIC)
        if img_idx >= 0 and len(data) >= img_idx + IMG_HEADER_SIZE:
            # Alles VOR dem Header als Text ausgeben
            if img_idx > 0:
                pre = data[:img_idx]
                print(f"  {C_CYAN}[RX {timestamp}]{C_RESET} {len(pre)} Bytes (vor Header):")
                print_rx_text(pre)

            # Header dekodieren
            header = data[img_idx:img_idx + IMG_HEADER_SIZE]
            payload_size = decode_img_header(header)
            print(f"\n  {C_BLUE}[RX HDR {timestamp}]{C_RESET} IMG:-Header erkannt!")
            print(f"       Magic: {header[:4]!r}")
            print(f"       Payload: {payload_size} Bytes ({payload_size/1024:.1f} KB)")
            hex_dump(header, prefix="       ")

            if payload_size and payload_size > 0:
                self.waiting_for_image = True
                self.image_remaining = payload_size
                self.image_received = 0
                self.image_start_time = time.time()

                # Rest nach dem Header als Bilddaten verarbeiten
                after_header = data[img_idx + IMG_HEADER_SIZE:]
                if after_header:
                    self._process_rx(after_header)
            return

        # ── Fall 3: Normale Daten (Text / Hex) ──
        print(f"  {C_CYAN}[RX {timestamp}]{C_RESET} {len(data)} Bytes:")
        # Versuche Text-Dekodierung
        print_rx_text(data)
        # Hex-Dump (nur bei kleinen Datenmengen oder wenn nicht druckbar)
        if len(data) <= 256:
            hex_dump(data, prefix="       ")
        else:
            hex_dump(data[:128], prefix="       ")
            print(f"       {C_DIM}... ({len(data) - 128} weitere Bytes){C_RESET}")


# ── Main ───────────────────────────────────────────────────────────────────

def main():
    port = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_PORT
    baud = int(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_BAUD

    print()
    print(f"{'='*60}")
    print(f"  Serial Monitor — STM32 CDC ACM Debug Tool")
    print(f"{'='*60}")
    print(f"  Port:     {port}")
    print(f"  Baudrate: {baud}")
    print(f"  Befehle:  Tippe Text + Enter → Senden an STM32")
    print(f"            'CAPTURE' → Sendet CAPTURE + wartet auf Bilddaten")
    print(f"            'quit'    → Monitor beenden")
    print(f"{'='*60}\n")

    mon = SerialMonitor(port, baud)

    try:
        mon.open()
    except Exception as e:
        print(f"{C_RED}[ERR]{C_RESET} Kann {port} nicht öffnen: {e}")
        sys.exit(1)

    # Empfangs-Thread starten
    mon.running = True
    rx_thread = threading.Thread(target=mon.rx_loop, daemon=True)
    rx_thread.start()

    # Eingabeschleife im Hauptthread
    try:
        while mon.running:
            try:
                user_input = input()
            except EOFError:
                break

            if not user_input:
                continue

            cmd = user_input.strip()

            if cmd.lower() == "quit":
                print(f"{C_BLUE}[INFO]{C_RESET} Beende Monitor...")
                break

            if cmd.lower() == "help":
                print(f"  {C_BLUE}Befehle:{C_RESET}")
                print(f"    CAPTURE   — Sendet CAPTURE\\n, erwartet IMG:-Header + Bilddaten")
                print(f"    READY     — Sendet READY\\n (Test)")
                print(f"    hex:AABB  — Sendet rohe Hex-Bytes")
                print(f"    quit      — Beendet den Monitor")
                continue

            # Hex-Modus: "hex:48454C4C4F"
            if cmd.lower().startswith("hex:"):
                try:
                    raw = bytes.fromhex(cmd[4:])
                    mon.send(raw)
                except ValueError as e:
                    print(f"  {C_RED}[ERR]{C_RESET} Ungültige Hex-Daten: {e}")
                continue

            # Normaler Textbefehl → mit \n senden
            if not mon.send((cmd + "\n").encode()):
                print(f"  {C_YELLOW}[WARN]{C_RESET} Senden fehlgeschlagen. Port evtl. blockiert.")

    except KeyboardInterrupt:
        print(f"\n{C_BLUE}[INFO]{C_RESET} Ctrl+C — Beende Monitor...")

    finally:
        mon.close()
        print("Bye!")


if __name__ == "__main__":
    main()
