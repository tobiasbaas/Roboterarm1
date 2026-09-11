# New_Image_Capture: STM32-Firmware zur Aufnahme der Trainingsbilder

Das ist die Firmware, die ihr braucht, wenn ihr den Datensatz für das KI-Training
aufnehmen wollt. Falls die KI direkt auf dem Board geflasht werden soll,
ist `02_Software/Thesis_Robot_Pose` das richtige Projekt.

## Inhalt

- [New\_Image\_Capture: STM32-Firmware zur Aufnahme der Trainingsbilder](#new_image_capture-stm32-firmware-zur-aufnahme-der-trainingsbilder)
  - [Inhalt](#inhalt)
  - [1. Was diese Firmware macht](#1-was-diese-firmware-macht)
  - [2. Projektstruktur](#2-projektstruktur)
  - [3. Schnellstart](#3-schnellstart)
    - [3.1 Voraussetzungen](#31-voraussetzungen)
    - [3.2 Bauen, signieren und flashen](#32-bauen-signieren-und-flashen)
    - [3.3 Verbindung testen](#33-verbindung-testen)
  - [4. Aufbau der Firmware](#4-aufbau-der-firmware)
    - [4.1 Der Weg durch main()](#41-der-weg-durch-main)
    - [4.2 Die beiden Threads](#42-die-beiden-threads)
    - [4.3 Ablauf einer Bildaufnahme](#43-ablauf-einer-bildaufnahme)
  - [5. Die Komponenten im Detail](#5-die-komponenten-im-detail)
    - [5.1 USB CDC](#51-usb-cdc)
    - [5.2 Kamera](#52-kamera)
    - [5.3 Display](#53-display)
    - [5.4 Speicher, MPU und Sicherheit](#54-speicher-mpu-und-sicherheit)
    - [5.5 Abgeschaltete Peripherie](#55-abgeschaltete-peripherie)
  - [6. Wichtige Konstanten](#6-wichtige-konstanten)
  - [7. Troubleshooting](#7-troubleshooting)

---

## 1. Was diese Firmware macht

Das Board sitzt zwischen Kamera und Laptop. Es hält immer ein aktuelles Bild bereit und
gibt es heraus, sobald das Python-Skript danach fragt.

```
   ┌────────────────────────┐
   │  Laptop (Windows)      │
   │  capture_image.py      │
   └───────────┬────────────┘
               │  USB CDC (virtueller COM-Port)
               │  Kommandos: PING / TEST / CAPTURE
               │  Antwort:   "IMG:" + 768000 Byte RGB565
               ▼
   ┌────────────────────────────────────────────┐
   │  STM32N6570-DK                             │
   │                                            │
   │  Kamera IMX335 ──► DCMIPP ──► Framebuffer  │
   │                               0x34200000   │
   │                                    │       │
   │                                    ▼       │
   │                                  LTDC      │
   │                                    │       │
   │                                    ▼       │
   │                          Display RK050HR18 │
   └────────────────────────────────────────────┘
```

**Warum dieser Aufbau?**

Der Framebuffer ist der zentrale Treffpunkt. Die Kamera schreibt über die
DCMIPP-Pipeline direkt hinein, der LTDC liest ihn permanent aus und zeigt ihn an, und
bei einem `CAPTURE` wird genau derselbe Speicherbereich über USB verschickt. 

>[!IMPORTANT]
>Damit das Bild während des Versendens nicht mittendrin überschrieben wird, friert die
>Firmware die Kamera-Pipeline vor dem Transfer ein. 
---

## 2. Projektstruktur

```
New_Image_Capture/
├── FSBL/                             # Kompletter STM32N6-Code
│   ├── Core/
│   │   ├── Src/main.c                # Peripherie, Takt, MPU, LTDC, DCMIPP-Clock
│   │   ├── Src/app_threadx.c         # die beiden Threads, USB-Protokoll
│   │   ├── Src/stm32n6xx_hal_msp.c
│   │   └── Inc/                      # Konfigurationsheader für Kamera, ISP, HAL
│   ├── USBX/App/                     # USB-Device-Stack, CDC-ACM-Klasse, Deskriptoren
│   ├── USBPD/                        # USB Power Delivery
│   ├── AZURE_RTOS/App/               # ThreadX- und USBX-Speicherpools
│   └── STM32N657XX_AXISRAM2_fsbl.ld  # Linker-Skript
├── Appli/                            # Nicht benutzt, ist leer
├── Drivers/                          # HAL und BSP, generiert
├── Middlewares/                      # ThreadX, USBX, Camera Middleware
├── New_Image_Capture.ioc             # CubeMX-Projektdatei
├── sign_binaries.bat                 # bauen, signieren, flashen
└── README.md                         # diese Datei
```

> [!IMPORTANT]
> Die gesamte produktive Logik liegt im **FSBL**, nicht in der Appli. 

---

## 3. Schnellstart

### 3.1 Voraussetzungen

**Hardware**

| Komponente | Anmerkung |
|---|---|
| STM32N6570-DK | mit Kamera IMX335 und Display RK050HR18 |
| USB-Kabel | für die Datenverbindung zum Laptop |


> [!IMPORTANT]
> Setzt vor dem Flashen **BOOT0 und BOOT1 auf Low**. Das ist der Flash-Mode.

### 3.2 Bauen, signieren und flashen

Ein Skript erledigt alles nacheinander:

```bat
sign_binaries.bat
```

Alternativ kann über das VsCode Terminal im richtien Projektordner der folgende Command ausgeführt werden: 

```bat
cmake --build build/Debug --target flash
```

Was dabei passiert:

1. CMake baut das Ziel `CDC_ACM_FSBL`
2. `objcopy` erzeugt aus der `.elf` eine `.bin`
3. Das Signing Tool macht daraus `CDC_ACM_FSBL-trusted.bin`
4. Der Programmer löscht den externen Flash und schreibt das Image
5. Reset
   
### 3.3 Verbindung testen

Steckt das Board per USB an den Laptop. Es meldet sich als virtueller COM-Port und
sendet nach der Enumeration dreimal `READY`.

Zum Testen reicht ein beliebiges Terminalprogramm auf dem COM-Port:

| Ihr sendet | Board antwortet |
|---|---|
| `PING` | `PONG` |
| `TEST` | `TEST_A_OK`, `TEST_B_OK`, 60 mal `X`, `TEST_DONE` |
| `CAPTURE` | `IMG:` + 4 Byte Länge + 768000 Byte Bilddaten |

Wenn `PING` und `TEST` funktionieren, stimmt die USB-Strecke.

---

## 4. Aufbau der Firmware

### 4.1 Der Weg durch main()

```c
MPU_Config();                  // muss VOR dem I-Cache kommen
SCB_EnableICache();            // nur I-Cache, D-Cache bleibt bewusst aus
HAL_Init();
SystemClock_Config();
__HAL_RCC_AXISRAM3_MEM_CLK_ENABLE();   // Takt für den Framebuffer
MX_GPIO_Init();
// ... GPDMA1, USART1, DCMIPP, LTDC, RAMCFG, USB
SystemIsolation_Config();
MX_ThreadX_Init();             // tx_kernel_enter, kehrt nie zurück
```

### 4.2 Die beiden Threads

Nach `tx_kernel_enter()` übernimmt der ThreadX-Scheduler. Bei ThreadX gilt: **kleinere
Zahl bedeutet höhere Priorität.**

| Thread | Priorität | Stack | Aufgabe |
|---|---|---|---|
| `cmd_thread` | 12 (höher) | 4 KB | wartet auf USB-Befehle, sendet Bilder |
| `camera_thread` | 15 | 8 KB | bedient die Kamera-Pipeline |

Dazu kommt ein dritter Thread aus dem USBX-Stack, der den USB-Device-Start übernimmt.
Den legt ihr nicht selbst an, er kommt aus der Middleware.

### 4.3 Ablauf einer Bildaufnahme

Das ist der wichtigste Teil der Firmware. Wenn der Laptop `CAPTURE` schickt, passiert
Folgendes:

```
1. Kamera wieder anlaufen lassen   CMW_CAMERA_Resume(DCMIPP_PIPE1)
2. 50 Ticks warten                 damit der ISP sich einschwingt
3. Kamera einfrieren               CMW_CAMERA_Suspend(DCMIPP_PIPE1)
4. Header senden                   "IMG:" + 4 Byte Payload-Länge
5. Framebuffer in Blöcken senden   je 8192 Byte, bis 768000 erreicht sind
6. Kamera bleibt eingefroren       bis zum nächsten CAPTURE
```

Im Code sieht Schritt 1 bis 3 so aus:

```c
if (pipe_suspended) {
  CMW_CAMERA_Resume(DCMIPP_PIPE1);
  pipe_suspended = 0;
}
tx_thread_sleep(CAPTURE_SETTLE_TICKS);

CMW_CAMERA_Suspend(DCMIPP_PIPE1);
pipe_suspended = 1;
```

> [!IMPORTANT]
> Nach dem Header und während der Binärdaten darf **kein Text** mehr über CDC gesendet
> werden. Sonst verschiebt sich der Datenstrom und der Laptop bekommt ein zerstörtes
> Bild. Alle Statusmeldungen gehen deshalb ab diesem Punkt nur noch über UART.

Der Header ist bewusst simpel gehalten, Länge als Little-Endian:

```c
memcpy(header, IMG_HEADER_MAGIC, IMG_HEADER_MAGIC_LEN);   // "IMG:"
header[4] = (UCHAR)((payload_size >>  0) & 0xFF);
header[5] = (UCHAR)((payload_size >>  8) & 0xFF);
header[6] = (UCHAR)((payload_size >> 16) & 0xFF);
header[7] = (UCHAR)((payload_size >> 24) & 0xFF);
```

Jeder Block wird bei einem Fehler bis zu fünfmal wiederholt, und der gesamte Transfer 
bricht nach 10 Sekunden ab. Damit hängt der Thread nicht fest, wenn die USB-Verbindung 
wegbricht.

---

## 5. Die Komponenten im Detail

### 5.1 USB CDC

Betrieben wird der USBX Device Stack mit der CDC-ACM-Klasse.

**Endpunkte:**

| Endpunkt | Typ | Größe |
|---|---|---|
| `0x81` | Interrupt IN, Kommandos | 8 Byte |
| `0x82` | Bulk IN, Daten zum Laptop | 512 Byte (High Speed) |
| `0x03` | Bulk OUT, Daten vom Laptop | 512 Byte (High Speed) |

**FIFO-Aufteilung** in `app_usbx_device.c`, insgesamt stehen 1024 Words zur Verfügung:

```c
HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_HS1, 0x200);   // 512 Words, 2048 Byte
HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS1, 0, 0x10); //  16 Words, EP0 Control
HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS1, 1, USBD_CDCACM_EPINCMD_HS_MPS / 4);
HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS1, 2, 0x100); // 256 Words, Bulk IN
```

Die USB-Hardware läuft als High Speed mit eingebautem PHY, DMA ist ausgeschaltet und
VBUS-Sensing ebenfalls.

### 5.2 Kamera

Der Ablauf im `camera_thread`:

```
1. CMW_CAMERA_GetSensorName()        Sensor identifizieren
2. CMW_CAMERA_Init()                 2592 x 1944, 30 fps, gespiegelt
3. HAL_DCMIPP_SetIPPlugConfig()      Burst 128 Byte, Page 256 Byte, Client 5
4. CMW_CAMERA_SetPipeConfig()        PIPE1 auf 640 x 480 RGB565, Aspect-Ratio-Crop
5. CMW_CAMERA_Start()                kontinuierlich in den Framebuffer
6. 200 Ticks ISP-Warmup              Belichtung und Weißabgleich einschwingen
7. CMW_CAMERA_Suspend()              einfrieren, bis das erste CAPTURE kommt
8. while(1) { CMW_CAMERA_Run(); }
```

Der ISP braucht die Aufwärmphase, sonst sind die ersten Bilder über- oder
unterbelichtet und damit für das Training unbrauchbar.

**Taktkonfiguration** in `main.c`, Funktion `MX_DCMIPP_ClockConfig()`:

| Signal | Quelle | Teiler | Ergebnis |
|---|---|---|---|
| DCMIPP | IC17 von PLL1 | 4 | 300 MHz |
| CSI PHY Ref | IC18 von PLL1 | 60 | 20 MHz |

Die ISP-Parameter für den IMX335 (Weißabgleich, Belichtungsautomatik, Demosaicing)
kommen aus `imx335_isp_param_conf.h` und der Camera Middleware.

### 5.3 Display

Der LTDC ist in `main.c` von Hand konfiguriert, nicht über CubeMX generiert.

| Parameter | Wert |
|---|---|
| Framebuffer | `0x34200000` in AXISRAM |
| Auflösung | 640 x 480 |
| Pixelformat | RGB565, 2 Byte je Pixel |
| Größe | 640 * 480 * 2 = 614400 Byte |
| Pixeltakt | IC16 von PLL1, Teiler 48, also 25 MHz |

Die Timings für das RK050HR18:

```
HorizontalSync     =   4      AccumulatedActiveW = 812
VerticalSync       =   4      AccumulatedActiveH = 492
AccumulatedHBP     =  12      TotalWidth         = 820
AccumulatedVBP     =  12      TotalHeight        = 500
```

AXISRAM3 und AXISRAM4 werden früh in `main()` eingeschaltet, sonst können LTDC und
Kamera nicht auf den Framebuffer zugreifen und das Display bleibt schwarz.

### 5.4 Speicher, MPU und Sicherheit

**MPU.** Für die USB-Strukturen legt `MPU_Config()` einen nicht cachebaren Bereich an.
Die Grenzen kommen aus Linker-Symbolen (`__snoncacheable` und `__enoncacheable`).

**RIF.** `SystemIsolation_Config()` setzt die Sicherheitsattribute für die Bus-Master,
die tatsächlich zugreifen dürfen: DCMIPP, LTDC und die GPDMA-Kanäle für UCPD. Ohne das
werden Zugriffe blockiert.

**Speicherpools** in `app_azure_rtos_config.h`:

| Pool | Größe |
|---|---|
| ThreadX | 1024 Byte |
| USBX | 38912 + 16 * 1024 Byte |
| USBPD | 5000 Byte |

Der USBX-Pool liegt in einer eigenen Linker-Section `.UsbxPoolSection`, damit er
garantiert im richtigen Speicherbereich landet.

### 5.5 Abgeschaltete Peripherie

In `main.c` sind mehrere von CubeMX erzeugte Init-Funktionen durch ein frühes `return`
stillgelegt: ADC, MDF, SAI, SDMMC2, USB2 HCD und XSPI1/XSPI2.

Das ist Absicht und kein Versehen. Die Funktionen bleiben im Code stehen, damit CubeMX
sie bei einer Neugenerierung nicht wieder anlegt, tun aber nichts.

---

## 6. Wichtige Konstanten

Alle in `FSBL/Core/Src/app_threadx.c`, weiter oben in der Datei:

| Konstante | Wert | Bedeutung |
|---|---|---|
| `LCD_FB_ADDRESS` | `0x34200000` | Framebuffer, auch in `main.c` definiert |
| `LCD_WIDTH` / `LCD_HEIGHT` | 640 / 480 | Bildgröße |
| `LCD_BPP` | 2 | RGB565 |
| `SENSOR_IMX335_WIDTH` / `_HEIGHT` | 2592 / 1944 | Sensorauflösung |
| `CAMERA_FPS` | 30 | Sollwert |
| `ISP_WARMUP_TICKS` | 200 | Aufwärmzeit nach dem Start |
| `CAPTURE_SETTLE_TICKS` | 50 | Wartezeit vor dem Einfrieren |
| `CDC_RX_BUF_SIZE` | 64 | Empfangspuffer für Kommandos |
| `CDC_TX_BUF_SIZE` | 8192 | Blockgröße beim Senden |
| `CDC_WRITE_RETRIES` | 5 | Wiederholungen je Block |
| `XFER_TIMEOUT_SEC` | 10 | Abbruch des Gesamttransfers |
| `IMG_HEADER_MAGIC` | `"IMG:"` | Kennung im Header |

> [!IMPORTANT]
> `LCD_FB_ADDRESS` ist in `main.c` **und** in `app_threadx.c` getrennt definiert. Wenn
> ihr die Adresse ändert, müsst ihr es an beiden Stellen tun. Sonst schreibt die Kamera
> in einen anderen Puffer, als über USB gesendet wird, und ihr bekommt ein Standbild
> oder Datenmüll.

---

## 7. Troubleshooting

| Symptom | Ursache und Abhilfe |
|---|---|
| Board meldet sich nicht als COM-Port | Firmware nicht korrekt geflasht oder nicht signiert. Nochmal flashen, danach RESET drücken |
| Kein `READY` nach dem Anstecken | Board resetten. Im UART-Log über ST-Link prüfen, ob `[DBG] cmd_thread started` erscheint |
| `PING` funktioniert, `CAPTURE` nicht | Meist ein Timeout. `XFER_TIMEOUT_SEC` erhöhen oder ein anderes USB-Kabel probieren |
| Bild kommt unvollständig oder verschoben an | Irgendwo wird Text über CDC gesendet, während Binärdaten laufen. Alle Ausgaben nach dem Header müssen über UART gehen |
| Display bleibt schwarz | AXISRAM3/4-Takt nicht aktiviert, oder LTDC-Timings stimmen nicht |
| Bild auf dem Display steht, ändert sich aber nicht | Normal. Die Kamera ist nach dem Warmup eingefroren und läuft erst beim ersten `CAPTURE` wieder an |
| Bilder sind über- oder unterbelichtet | ISP-Warmup zu kurz. `ISP_WARMUP_TICKS` erhöhen |
| HardFault kurz nach dem Start | Meist der D-Cache. Er muss deaktiviert bleiben, solange kein Cache-Maintenance für Kamera und USB ergänzt ist |

---