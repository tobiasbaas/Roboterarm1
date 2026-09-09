# New_Image_Capture

Ausfuehrliche technische Dokumentation zur Implementierung und Konfiguration der verwendeten Hardware-Komponenten im Projekt.

Schwerpunkt dieser Dokumentation:
- USB CDC (USBX Device)
- Kamera (IMX335 ueber CMW + DCMIPP)
- Display (LTDC auf RK050HR18, 640x480)
- ThreadX-Ausfuehrungsmodell

## 1. Projektueberblick

Das Projekt ist ein STM32N6570-DK-basiertes Embedded-System mit Live-Kamerapipeline auf das LCD und USB-CDC-Ausgabe von Einzelbildern.

Wichtige Beobachtung zur Projektstruktur:
- Die produktive Logik liegt im FSBL-Projektteil.
- Dort sind Kamera, Display, USBX/CDC und ThreadX vollstaendig implementiert.

Kernidee des Systems:
1. Kamera liefert Daten in DCMIPP.
2. DCMIPP skaliert/croppt auf 640x480 RGB565.
3. Framebuffer liegt in AXISRAM und wird direkt vom LTDC angezeigt.
4. Bei USB-Befehl CAPTURE wird ein eingefrorener Frame ueber CDC an den Host gestreamt.

## 2. Architektur der drei Hauptkomponenten

## 2.1 USB CDC

USB wird als USBX Device Stack mit CDC ACM Klasse betrieben.

Relevante Implementierungsdateien:
- FSBL/USBX/App/app_usbx.c
- FSBL/USBX/App/app_usbx_device.c
- FSBL/USBX/App/ux_device_cdc_acm.c
- FSBL/USBX/App/ux_device_descriptors.c
- FSBL/USBX/App/ux_device_descriptors.h
- FSBL/Core/Src/main.c
- FSBL/Core/Src/stm32n6xx_hal_msp.c
- FSBL/Core/Src/stm32n6xx_it.c

### USB Device-Stack und Klassenregistrierung

In app_usbx.c wird USBX initialisiert:
- USBX-Memory wird aus einem separaten Byte-Pool reserviert.
- Danach startet MX_USBX_Device_Init().

In app_usbx_device.c:
- Device-Framework (HS/FS + String + Language) wird aufgebaut.
- CDC ACM Klasse wird am Device-Stack registriert.
- Eine USBX-App-Threadinstanz startet die Device-Hardware.

CDC-Activation Callback:
- In ux_device_cdc_acm.c wird ein globaler Zeiger gesetzt:
  - g_cdc_acm != NULL bedeutet: Host ist enumeriert und CDC aktiv.

### USB Endpunkte und Paketgroessen

Aus den Descriptoren:
- Interrupt IN CMD EP: 0x81 (8 Byte)
- Bulk IN Daten EP: 0x82 (HS: 512 Byte)
- Bulk OUT Daten EP: 0x03 (HS: 512 Byte)

FIFO-Konfiguration (USB OTG HS) in app_usbx_device.c:
- RxFIFO: 0x200 Words
- TxFIFO0 (EP0): 0x10 Words
- TxFIFO1 (CMD): EP-MPS/4
- TxFIFO2 (Bulk IN): 0x100 Words

### USB Hardware-Initialisierung

In main.c (MX_USB1_OTG_HS_PCD_Init):
- USB1_OTG_HS Device, High-Speed, Embedded PHY
- DMA explizit deaktiviert
- VBUS sensing deaktiviert

In stm32n6xx_hal_msp.c:
- USB OTG HS Clock aus HSE direct
- USB PHY Clock aus HSE direct
- VDDUSB und PHY-Reset-Sequenz nach ST-Muster
- IRQ: USB1_OTG_HS_IRQn aktiviert (Prio 7)

### CDC-Befehlsprotokoll im Laufzeitbetrieb

In app_threadx.c (cmd_thread):
- Initial nach Enumeration: 3x READY
- Unterstuetzte Befehle:
  - PING -> PONG
  - TEST -> mehrere Test-Transfers
  - CAPTURE -> Bildtransfer

CAPTURE-Protokoll:
1. Header mit 8 Byte senden:
   - Magic: IMG:
   - 4 Byte Payloadlaenge little-endian
2. Danach rohes RGB565-Framebuffer als Binardaten senden.

Wichtige Details:
- TX Chunk-Groesse: 8192 Byte
- Retry-Mechanik bei Schreibfehlern
- Gesamter Transfer hat Timeout-Fenster
- Nach Header/Binardaten wird bewusst kein Text auf CDC ausgegeben, um den Binardatenstrom nicht zu desynchronisieren.

## 2.2 Kamera (IMX335 + CMW + DCMIPP)

Relevante Implementierungsdateien:
- FSBL/Core/Src/app_threadx.c
- FSBL/Core/Src/main.c
- FSBL/Core/Src/stm32n6xx_hal_msp.c
- FSBL/Core/Inc/cmw_camera_conf.h
- FSBL/Core/Inc/imx335_isp_param_conf.h
- FSBL/Core/Inc/isp_conf.h
- Middlewares/Camera_Middleware/*

### Sensor- und Pipelineparameter

In app_threadx.c:
- Sensor-Sollgroesse: 2592x1944
- Zielausgabe: 640x480
- Ausgabeformat: RGB565
- Kamera-FPS Sollwert: 30

Ablauf im camera_thread:
1. Sensorname ermitteln.
2. Kamera via CMW_CAMERA_Init() initialisieren.
3. DCMIPP IPPlug konfigurieren.
4. Pipe-Konfiguration setzen (PIPE1, RGB565, Crop-Modus).
5. Kamera kontinuierlich starten auf den LCD-Framebuffer.
6. ISP Warmup-Zeit abwarten.
7. Pipe suspendieren (Frame Freeze).
8. Dauerhaft CMW_CAMERA_Run() im Loop.

### DCMIPP/CSI Clockkonfiguration

In main.c (MX_DCMIPP_ClockConfig):
- DCMIPP Clock ueber IC17 von PLL1 mit Teiler 4 (300 MHz)
- CSI PHY Ref Clock ueber IC18 von PLL1 mit Teiler 60 (20 MHz)

In stm32n6xx_hal_msp.c:
- DCMIPP und CSI Clocks aktiviert
- DCMIPP_IRQn und CSI_IRQn aktiviert (Prio 7)

### ISP-Konfiguration

Die Kamera-ISP-Parameter werden ueber Konfigurationsheader und CMW-Middleware bereitgestellt (u. a. AWB/AEC/Demosaicing-Parameter fuer IMX335).

## 2.3 Display (LTDC + RK050HR18)

Relevante Implementierungsdateien:
- FSBL/Core/Src/main.c
- FSBL/Core/Src/stm32n6xx_hal_msp.c
- FSBL/Core/Inc/main.h
- FSBL/Core/Inc/stm32n6570_discovery_conf.h
- Drivers/BSP/STM32N6570-DK/stm32n6570_discovery_lcd.*

### Framebuffer und Pixelformat

In main.c:
- Framebuffer-Adresse: 0x34200000
- Aufloesung: 640x480
- Farbraum: RGB565 (2 Byte/Pixel)
- Framebuffer-Groesse: 640 * 480 * 2 = 614400 Byte

AXISRAM3/4 wird frueh aktiviert, damit LTDC und Kamera auf den Framebuffer zugreifen koennen.

### LTDC Timing-Konfiguration

LTDC ist in main.c manuell in USER CODE implementiert:
- HorizontalSync = 4
- VerticalSync = 4
- AccumulatedHBP = 12
- AccumulatedVBP = 12
- AccumulatedActiveW = 812
- AccumulatedActiveH = 492
- TotalWidth = 820
- TotalHeigh = 500

Layer 0:
- Fenster 0..640 x 0..480
- PixelFormat RGB565
- FBStartAddress = 0x34200000

LTDC Clock in MSP:
- IC16 von PLL1 mit Teiler 48 (Pixelclock 25 MHz)

GPIO:
- RGB/Sync/DE-Pins auf AF14 LCD
- zusaetzliche LCD-Control-Leitungen (Reset, OnOff, Backlight) werden als GPIO gesetzt.

## 3. ThreadX-Ausfuehrungsmodell

Wichtiger Punkt zur Anforderung "arbeiten in einem Thread":

Der aktuelle Code verwendet zwei Anwendungs-Threads:
- camera_thread
- cmd_thread

Zusatzthread:
- USBX Device App Thread fuer USB-Start/Stack-Integration

### Thread-Konfiguration

In app_threadx.c:
- camera_thread:
  - Stack: 8192
  - Priority: 15
- cmd_thread:
  - Stack: 4096
  - Priority: 12

Interpretation:
- Kamera/Display-Pipeline laeuft im camera_thread.
- USB-Befehle und Bildausgabe laeuft im cmd_thread.
- Die Komponenten arbeiten also logisch zusammen, aber nicht in exakt einem einzigen Thread.

## 4. Zusammenspiel USB + Kamera + Display

Laufzeitfluss:
1. camera_thread startet Sensor + DCMIPP und aktualisiert den Framebuffer.
2. LTDC zeigt den Framebuffer permanent an.
3. cmd_thread wartet auf USB-Befehle.
4. Bei CAPTURE:
   - Kamera wird kurz resumed (settle)
   - danach suspendiert (Freeze)
   - aktueller Frame wird via USB CDC gesendet.

Dadurch ist der gesendete Frame konsistent und entspricht dem eingefrorenen Displaybild.

## 5. Speicher, MPU, Cache und Security

## 5.1 MPU und Cache

In main.c wird MPU vor HAL_Init konfiguriert:
- Non-cacheable Region fuer USBX/USB-Strukturen ueber Linker-Symbole (__snoncacheable, __enoncacheable)

Caches:
- I-Cache aktiviert
- D-Cache absichtlich deaktiviert

Grund im Codekommentar:
- Fruehere D-Cache-Aktivierung fuehrte zu HardFaults bzw. Datenkonsistenzproblemen mit DMA/Kamera/USB.

## 5.2 RIF / Security Attribute

SystemIsolation_Config() setzt Sicherheitsattribute fuer relevante Bus-Master/Peripherien:
- DCMIPP
- LTDC
- GPDMA Kanaele fuer UCPD

Damit wird der Zugriff im Secure/Privileged-Kontext explizit festgelegt.

## 5.3 ThreadX/USBX Memory Pools

In app_azure_rtos_config.h:
- TX_APP_MEM_POOL_SIZE = 1024
- UX_APP_MEM_POOL_SIZE = 38912 + 16*1024
- USBPD_DEVICE_APP_MEM_POOL_SIZE = 5000

In app_azure_rtos.c:
- Separate Byte-Pools fuer ThreadX, USBX und USBPD
- USBX-Pool liegt in spezieller Section (.UsbxPoolSection)

## 6. Weitere konfigurierte Peripherie (derzeit bewusst deaktiviert)

In main.c sind mehrere MX_*_Init-Funktionen bewusst per fruehem return stillgelegt:
- ADC
- MDF
- SAI
- SDMMC2
- USB2 HCD
- XSPI1/XSPI2

Damit bleibt die Laufzeit auf die benoetigten Funktionen fokussiert (USB CDC Device, Kamera, LTDC, ThreadX).

## 7. Build- und Projektintegration

Relevante Build-Dateien:
- CMakeLists.txt (Projektroot)
- FSBL/CMakeLists.txt
- FSBL/CMakePresets.json
- FSBL/mx-generated.cmake

Kernaussagen:
- FSBL Ziel: CDC_ACM_FSBL
- Toolchain: arm-none-eabi
- Post-Build erzeugt .bin
- custom target flash ruft sign_binaries.bat auf
- mx-generated.cmake bindet HAL, ThreadX, USBX, USBPD, Camera Middleware und BSP ein

## 8. Zusammenfassung

Im implementierten Stand bildet das Projekt eine stabile Echtzeitkette:
- Kameraaufnahme (IMX335 -> DCMIPP)
- Live-Anzeige (LTDC auf RK050HR18)
- On-Demand-Bildtransfer ueber USB CDC

Wesentlich fuer die Stabilitaet sind:
- klare Trennung von Kamera- und Kommando-Thread
- Freeze/Resume-Mechanik fuer konsistente CAPTURE-Frames
- MPU Non-cacheable Bereich fuer USB-Strukturen
- deaktivierter D-Cache zur Vermeidung von DMA-Kohaerenzproblemen

Wenn die Architektur zwingend auf einen einzigen Thread umgestellt werden soll, kann das als naechster Schritt umgesetzt werden, indem die CDC-Befehlsbehandlung in den camera_thread integriert wird.