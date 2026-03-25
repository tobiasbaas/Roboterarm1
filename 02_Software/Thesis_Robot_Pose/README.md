# Thesis_Robot_Pose

Technische Dokumentation fuer den aktuellen Integrationsstand von STM32Cube AI Studio Code in das CMake-Projekt auf STM32N6570-DK.

## 1. Status (aktuell)

Die folgenden Punkte sind umgesetzt und erfolgreich gebaut:

1. Import des von STM32Cube AI Studio erzeugten Netzwerk-Codes (run-13 Export).
2. Import der ST AI Runtime und NPU Low-Level Runtime.
3. Umstellung der Appli-Linkerstrategie auf externes XIP mit zwei ROM-Baenken.
4. Nicht-blockierende AI-Ausfuehrung in eigenem ThreadX-Thread.
5. Build von CDC_ACM_Appli ist erfolgreich.

Aktuelle Memory-Auslastung (letzter erfolgreicher Link):

- ROM: 26.08%
- ROM2: 84.46%
- RAM: 0.53%
- EXTRAM: 0.00%

## 2. Wichtigste Aenderungen

### 2.1 AI Code-Import

Aus dem Exportordner wurden folgende Bereiche eingebunden:

- Appli/AI/App: Integrations- und Init-Code
- Appli/AI/generated: Netzwerkcode (network.c, Header, Blob-Dateien)
- Middlewares/ST/AI: ST AI Runtime, NPU Runtime, statische Runtime-Library

### 2.2 Build-Integration

Die AI-Integration ist bewusst getrennt von CubeMX-Dateien:

- Appli/CMakeLists.txt bindet zusaetzlich ai-integration.cmake ein.
- Appli/ai-integration.cmake verwaltet:
  - AI Quellen
  - Include-Pfade
  - Compile-Defines fuer LL_ATON
  - Link zur NetworkRuntime1100_CM55_GCC.a
  - ThreadX Quellen fuer den Appli-Kontext

### 2.3 Linker / externes Memory

Aufgrund ROM-Ueberlauf wurde Appli auf ein duales externes ROM-Layout umgestellt:

- Linker-Skript: STM32N657XX_ROMxspi1xspi2_RAMxspi3.ld
- Code (.text) bleibt in ROM
- grosse Konstanten (.rodata) liegen in ROM2

Damit passt die Gesamtauslastung wieder in den verfuegbaren Addressraum.

## 3. Laufzeitarchitektur (Appli)

### 3.1 Main-Ablauf

Main initialisiert AI und startet dann den ThreadX Kernel:

1. HAL Init und Basis-Peripherie
2. STM32CubeAI_Studio_AI_Init
3. MX_ThreadX_Init (tx_kernel_enter)

Der alte blockierende Polling-Ansatz im Main-Loop wird nicht mehr genutzt.

### 3.2 ThreadX AI Thread

In Appli/Core/Src/app_threadx.c laeuft ein eigener AI-Thread:

- zyklischer Aufruf (1 Tick Sleep)
- Input vorbereiten
- aiRun aufrufen
- Output verarbeiten

Die Integration ist ueber weak Hooks vorbereitet:

- App_AI_PrepareInput(uint8_t* input_buffer, size_t input_size)
- App_AI_OnResult(const uint8_t* output_buffer, size_t output_size)

Diese Hooks sind der vorgesehene Punkt fuer Kamera-Preprocessing und LTDC-Overlay-Ausgabe.

## 4. Kamera/LTDC/ThreadX-Schutz

Wichtiges Ziel war, bestehende Laufzeitpfade nicht zu blockieren:

1. AI Prozess ist nicht-blockierend.
2. ISR-Konflikte wurden fuer ThreadX-Runtime abgefangen (SVC/PendSV/SysTick Guards).
3. Kamera/LTDC-spezifische Verarbeitung ist nicht hart verdrahtet im AI-Code, sondern ueber Hooks vorgesehen.

Hinweis: Die konkrete Pose-Visualisierung auf dem Display ist erst dann aktiv, wenn App_AI_OnResult mit deiner Overlay-Logik befuellt ist.

## 5. Relevante Dateien

- Appli/CMakeLists.txt
- Appli/ai-integration.cmake
- Appli/STM32N657XX_ROMxspi1xspi2_RAMxspi3.ld
- Appli/AI/App/app_x-cube-ai.c
- Appli/AI/App/app_x-cube-ai.h
- Appli/Core/Src/main.c
- Appli/Core/Src/app_threadx.c
- Appli/Core/Src/tx_initialize_low_level.S
- Appli/Core/Src/stm32n6xx_it.c

## 6. Build

Getesteter Build-Target:

- CDC_ACM_Appli

Ergebnis:

- Build erfolgreich
- Warning vorhanden: LOAD segment with RWX permissions
  - diese Warning kommt vom aktuellen Linker-Layout und ist fuer den funktionalen Build nicht blockierend

## 6.1 Flash-Layout und Ladevorgang (alle Programmabschnitte)

Im aktuellen Stand werden drei Programmabschnitte in den externen Flash geschrieben:

1. FSBL signiertes Image
2. APPLI Core signiertes Image (Code plus nicht-ROM2 Anteile)
3. APPLI ROM2 Datenblock (aus .rodata)

### Adressen im Flash

- FSBL: 0x70000000
- APPLI Core (trusted): 0x70100000
- APPLI ROM2 Daten: 0x70200400

### Warum in drei Teile?

Die Appli verwendet zwei ROM-Bereiche (ROM und ROM2). Ein einzelnes flaches .bin aus dem gesamten ELF fuehrt wegen der Adressluecke zu einem extrem grossen Sparse-Binaerfile. Deshalb wird APPLI gesplittet:

- Core-Teil wird signiert und nach 0x70100000 programmiert.
- ROM2-Teil wird separat als rodata-Binaer erzeugt und nach 0x70200400 programmiert.

### Technischer Ablauf im Skript

Das Skript in sign_binaries.bat fuehrt diese Schritte aus:

1. FSBL .bin signieren zu CDC_ACM_FSBL-trusted.bin.
2. APPLI aus CDC_ACM_Appli.elf aufteilen:
  - CDC_ACM_Appli-core.bin (ohne .rodata)
  - CDC_ACM_Appli-rom2.bin (nur .rodata)
3. APPLI Core signieren zu CDC_ACM_Appli-trusted.bin.
4. Mit STM32_Programmer_CLI programmieren:
  - FSBL-trusted nach 0x70000000
  - APPLI-trusted nach 0x70100000
  - APPLI-rom2 nach 0x70200400

Verwendete Optionen fuer robustes Flashing:

- Connect mode: Under Reset
- Reset mode: Hardware reset
- reduzierte SWD-Frequenz (4 MHz angefordert)

### Ausfuehren

Empfohlen im Projektwurzelordner:

```bat
cube-cmake --build C:/Dev/Thesis_Robot_Pose/build/Debug --target flash --
```

Alternativ direkt:

```bat
sign_binaries.bat
```

### Erfolgskriterium im Log

Der Upload gilt als erfolgreich, wenn fuer alle drei Files jeweils File download complete gemeldet wird.

Hinweis:

- Ein optionaler MCU-Reset am Ende kann fehlschlagen, ohne den Programmiervorgang ungueltig zu machen.
- Falls die Anwendung nicht automatisch startet, RESET-Taste am Board druecken.

## 7. Was als naechstes zu tun ist

1. App_AI_PrepareInput mit realem Kamera-Frame (RGB565 -> Modellinput) implementieren.
2. App_AI_OnResult mit Pose-Decoding und LTDC-Overlay aktualisieren.
3. End-to-End Test auf Target (Kamera live, Pose-Punkte sichtbar, USB/ThreadX stabil).
4. Optional: Resultatuebergabe an USB CDC um Status/Pose-Werte abzufragen.

## 8. Hinweise fuer Regeneration

1. CubeMX-generierte Dateien moeglichst unveraendert lassen.
2. Benutzerlogik in separaten Dateien halten (z. B. ai-integration.cmake, app_threadx.c).
3. Nach Linker- oder Memory-Aenderungen immer Full Rebuild durchfuehren.

## 9. Vorherige Projektdoku (ergaenzt, nicht entfernt)

Die folgenden Inhalte stammen aus der vorherigen README-Version und bleiben hier als Referenz erhalten.

### 9.1 Ziel des Projekts (vorheriger Stand)

Das Projekt bildet eine Edge-AI Pipeline auf dem STM32N6 ab:

1. Kameraaufnahme ueber IMX335 + CMW + DCMIPP
2. Live-Ausgabe auf LCD (LTDC, RGB565)
3. Kontinuierliche Bildverarbeitung fuer KI-Inferenz
4. USB CDC bleibt als Steuer- und Diagnosekanal aktiv
5. Ausfuehrung und Datenhaltung mit externen Speichern (OctoFlash / HyperRAM je nach Mapping)

Der vorher dokumentierte Laufzeitfokus:

- Kein Freeze der Kamera fuer CAPTURE
- Kein aktiver Bildtransfer ueber CDC
- USB bleibt enumerierbar und fuer einfache Kommandos nutzbar

### 9.2 Projektstruktur (vorher dokumentiert)

```text
Thesis_Robot_Pose/
|- CMakeLists.txt
|- CMakePresets.json
|- sign_binaries.bat
|- FSBL/
|  |- Core/
|  |- USBX/
|  |- AZURE_RTOS/
|  |- STM32N657XX_AXISRAM2_fsbl.ld
|  |- CMakeLists.txt
|  |- mx-generated.cmake
|- Appli/
|  |- Core/
|  |- STM32N657XX_ROMxspi2.ld
|  |- CMakeLists.txt
|  |- mx-generated.cmake
|- Drivers/
|- Middlewares/
`- Secure_nsclib/
```

### 9.3 FSBL vs Appli (vorheriger Fokus)

- FSBL (First Stage Boot Loader):
  - Initialisiert Low-Level-Plattform, RTOS-Stack, Kamera, Display, USB und OctoFlash-Memory-Mapped Modus.
  - Enthaelt die produktive Laufzeitlogik fuer Kamera/USB/ThreadX.

- Appli:
  - Secure Application Teil.
  - Linker war in der alten Doku auf externes XSPI2-ROM beschrieben.

### 9.4 Laufzeitarchitektur (vorherige Beschreibung)

Im FSBL wurden zwei wesentliche Threads beschrieben:

1. camera_thread
  - Initialisiert IMX335, CMW und DCMIPP Pipe1
  - Startet kontinuierlichen Kamera-Stream auf den LCD-Framebuffer
  - Fuehrt pro Zyklus den KI-Hook aus (Integrationspunkt)

2. cmd_thread
  - Wartet auf USB CDC Enumeration
  - Beantwortet einfache Kommandos (z. B. PING, STATUS)
  - Sendet keine Binardaten/Bilder

Kamera (vorher dokumentiert):

- Sensor: IMX335
- Sensorziel: 2592x1944
- Ausgabe: 640x480 RGB565
- Pipeline: CMW + DCMIPP Pipe1
- Modus: kontinuierlich (kein Suspend/Freeze fuer Capture)

Display (vorher dokumentiert):

- Panel: RK050HR18 (640x480)
- LTDC Layer 0 mit RGB565
- Framebuffer bei 0x34000000 (AXISRAM1)

USB CDC (vorher dokumentiert):

- USBX Device + CDC ACM aktiv
- Steuer- und Diagnoseinterface
- Kein kontinuierlicher Bildversand im damaligen Build

### 9.5 Speicherlayout und KI-Bezug (vorher dokumentiert)

Warum Framebuffer auf 0x34000000:

- Der Framebuffer wurde aus einem NPU-relevanten Bereich herausgelegt, damit KI-Aktivierungen und Kameraframe sich nicht ueberlagern.

OctoFlash Nutzung (vorher dokumentiert):

- FSBL/Boot-Image Bereich ab 0x70000000
- Appli-Image Bereich ab 0x70100000
- Reservierter Bereich fuer KI-Gewichte ab 0x71000000

Im FSBL-Linker war dafuer eine Sektion vorgesehen:

- .nn_weights in Region OCTOFLASH

### 9.6 Build-, Sign- und Flash-Hinweise (vorher dokumentiert)

Toolchain:

- GNU Arm Embedded (arm-none-eabi)
- CMake + Ninja (STM32Cube Umgebung)

Sign/Flash:

- sign_binaries.bat signiert typischerweise FSBL und Appli
- Dokumentierte Zieladressen:
  - FSBL -> 0x70000000
  - APPLI -> 0x70100000

Hinweis aus der vorherigen Doku:

- Fuer KI-Gewichte in .nn_weights ist ggf. ein separater Programmierschritt nach 0x71000000 erforderlich.

### 9.7 Historische Integrationshinweise (weiterhin nuetzlich)

Der alte Integrationspunkt war im camera_thread (FSBL):

1. Preprocessing gemaess Modellinput
2. AI Inferenzaufruf
3. Postprocessing (Klassen, Boxes, Pose Keypoints)

Beispiel fuer Gewichtssektion:

```c
__attribute__((section(".nn_weights")))
const unsigned char network_weights[] = { ... };
```

### 9.8 Hinweise und naechste Schritte aus der alten Doku

Bekannte Hinweise:

1. D-Cache war bewusst deaktiviert (DMA/Kamera/USB Kohaerenz und Stabilitaet).
2. Nach CubeMX-Regeneration sollten USER CODE Bloecke geprueft werden.
3. Nach Linker-/Adressaenderungen Full Rebuild und Flash-Test.
4. KI-Integration zuerst mit kleinem Testinput und UART/CDC Debug validieren.

Historisch vorgeschlagene naechste Schritte:

1. AI Studio C-Code in den Kamera-Hook integrieren.
2. Pre/Postprocessing fuer das konkrete Modell fixieren.
3. Gewichts-Blob nach 0x71000000 programmieren und Starttest auf Target ausfuehren.
4. Optional USB-Kommandos fuer Ergebnisabfrage erweitern.
