# Thesis_Robot_Pose

Technische Dokumentation fuer den aktuellen Integrationsstand von STM32Cube AI Studio Code in das CMake-Projekt auf STM32N6570-DK.

## 1. Status (aktuell)

### 1.0 Critical Fix: APPLI Startup Bugs behoben (April 2026) ✅

**Zusammenfassung der drei Bugs und Fixes:**

| # | Bug | Symptom | Fix | Status |
|---|-----|---------|-----|--------|
| 1 | HAL_Delay hängt vor ThreadX Start | LED-Blink steckt in Endlosschleife bei HAL_Delay(200) | ThreadX SysTick_Handler mit HAL_IncTick erweitern | ✅ Fixed |
| 2 | FSBL Sanity-Check fallthrough | Ungültige Vektoren führen zum Jump zu Müll-Adressen | Error_Handler() nach sanity check einfügen | ✅ Fixed |
| 3 | LCD Framebuffer überschreibt .data/.bss | Globale Variablen corrupted vor ThreadX/AI init | LCD_FB_ADDRESS von 0x34000000 zu 0x34400000 verschieben | ✅ Fixed |

**Betroffene Dateien:**
- `Appli/Core/Src/tx_initialize_low_level.S` — Zeile ~631: `BL HAL_IncTick` vor `BL _tx_timer_interrupt`
- `FSBL/Core/Src/main.c` — Zeile ~507: `Error_Handler();` nach `g_fsbl_jump_stage = 0xE001U;`
- `Appli/Core/Src/main.c` — Zeile 42: `#define LCD_FB_ADDRESS 0x34400000U` (statt 0x34000000U)

**Details:** Siehe Abschnitt 1.3 unten.

---

### 1.0a Wichtigste Änderung (März 2026) — Dual-xSPI (OctaSPI) Aktiviert ✅

**Problem behoben:** Das Projekt wurde auf Dual-xSPI umgestellt, um die 5,88 MB großen Modelle (Pose + Segmentation) optimal zu verteilen:

| Aspekt | Alt | Neu | Status |
|--------|-----|-----|--------|
| Linker-Skript | `STM32N657XX_ROMxspi1.ld` | `STM32N657XX_ROMxspi1xspi2_RAMxspi3.ld` | ✅ |
| Flash-Layout | 1 xSPI (2047K) | 2 xSPI: ROM (511K) + ROM2 (2047K) | ✅ |
| ROM Auslastung | 84,46% | **27,84%** | ✅ FIXED |
| ROM2 Auslastung | - | **46,76%** | ✅ NEW |
| OctaSPI | ❌ Nicht genutzt | ✅ Vollständig genutzt | ✅ |

**Memory Layout nach Fix:**
- Appli Code (.text) → xSPI1/ROM (511K, 27,84% used)
- Modellgewichte (.rodata) → xSPI2/ROM2 (2047K, 46,76% used)
- Platz für Wachstum vorhanden: ~1100K in ROM2

Die folgenden Punkte sind umgesetzt und erfolgreich gebaut:

1. Import des von STM32Cube AI Studio erzeugten Netzwerk-Codes (run-13 Export).
2. Import der ST AI Runtime und NPU Low-Level Runtime.
3. Umstellung der Appli-Linkerstrategie auf externes **Dual-xSPI** mit getrennten ROM-Baenken.
4. Nicht-blockierende AI-Ausfuehrung in eigenem ThreadX-Thread.
5. Build von CDC_ACM_Appli ist erfolgreich.
6. Zwei AI-Modelle (Pose + Segmentierung) sind parallel eingebunden und zur Laufzeit umschaltbar.

Aktuelle Memory-Auslastung (nach Linker-Fix):

- ROM: 27.84% (145.7 KB / 511 KB)
- ROM2: 46.76% (980.1 KB / 2047 KB)
- RAM: 0.53% (11.2 KB / 2048 KB)
- EXTRAM: 0.00%

### 1.1 FSBL JumpToApplication Fix (März 2026) ✅

**Problem behoben:** Die `JumpToApplication()`-Funktion in der FSBL war nach Standard-Cortex-M-Muster implementiert, hatte aber drei kritische Fehler für den STM32N6 (Cortex-M55 / ARM v8.1-M):

| Problem | Alt | Neu | Begründung |
|---------|-----|-----|------------|
| `HAL_RCC_DeInit()` vor Jump | Ja | **Entfernt** | Riskiert Verlust des xSPI2 Memory-Mapped-Modus — Appli liegt bei 0x70100400 in XSPI2 |
| `HAL_DeInit()` vor Jump | Ja | **Entfernt** | Unnötig; Appli macht eigenes Init in `SystemClock_Config()` |
| `__set_MSPLIM(0)` | Fehlend | **Hinzugefügt** | Cortex-M55 (v8.1-M) Pflicht: altes MSPLIM vor `__set_MSP()` löschen, sonst sofortiger Stack-Overflow-Fault |
| I-Cache deaktivieren | Fehlend | **Hinzugefügt** | Appli soll mit sauberem Cache-Zustand starten |
| SysTick | `CTRL/LOAD/VAL = 0` hart | `HAL_SuspendTick()` | ST-Standard, saubereres Suspend |

**Technischer Hintergrund:** Die FSBL läuft aus AXISRAM2 (0x34180400), nicht aus XSPI2. `HAL_RCC_DeInit()` hätte den XSPI2-Peripheral-Clock zurücksetzen können — danach wäre der CPU-Fetch von 0x70100400 mit einem HardFault gescheitert. Die Appli konfiguriert Clocks vollständig in ihrer eigenen `SystemClock_Config()` neu.

**Betroffene Datei:** `FSBL/Core/Src/main.c` — Funktion `JumpToApplication()` (Zeile ~205)

---

### 1.2 FSBL XSPI-Initialisierung (XiP-Vorbereitung für den Sprung) — April 2026 ✅

Damit der Sprung zur Appli gelingt, muss die FSBL XSPI2 (NOR Flash, 0x70000000) und XSPI1 (HyperRAM, 0x90000000) korrekt in den Memory-Mapped-Modus versetzen. Dabei waren vier kritische Probleme zu lösen:

#### Fix 1 — uwTick bleibt 0 (HAL_Delay/Timeout kaputt)

**Problem:** `HAL_TIM_PeriodElapsedCallback` war nicht überschrieben. HAL rief den leeren weak-Handler auf → `uwTick` wurde nie inkrementiert → alle HAL-Timeouts schlugen nach 0ms fehl.

**Lösung:** Starke Überschreibung in `FSBL/Core/Src/main.c`:
```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6) { HAL_IncTick(); }
}
```

**Betroffene Datei:** `FSBL/Core/Src/main.c`

---

#### Fix 2 — XSPIM (IO-Manager) nicht konfiguriert

**Problem:** CubeMX generiert `MX_XSPI_NOR_Init()` und `MX_XSPI_RAM_Init()` ohne `HAL_XSPIM_Config()`-Aufruf. Der XSPI IO-Manager (XSPIM) routet beide XSPI-Controller auf die GPIO-Ports. Ohne XSPIM-Konfiguration: keine Memory-Mapped-Kommunikation.

**Lösung:** Starke Überschreibungen in `FSBL/Core/Src/main.c` — beide Funktionen rufen zusätzlich `HAL_XSPIM_Config()` mit den Board-spezifischen Port-Zuweisungen auf:
- XSPI2 (NOR Flash): `IOPort = HAL_XSPIM_IOPORT_2` (GPION), `nCSOverride = NCS1`
- XSPI1 (HyperRAM): `IOPort = HAL_XSPIM_IOPORT_1` (GPIOP/GPIOO), `nCSOverride = NCS1`

**Betroffene Datei:** `FSBL/Core/Src/main.c` — `MX_XSPI_NOR_Init()` und `MX_XSPI_RAM_Init()`

---

#### Fix 3 — Boot ROM lässt XSPI2 in DOPI Memory-Mapped-Modus (BUSY=1 permanent)

**Problem:** Der Boot ROM konfiguriert XSPI2 in den OPI DTR Memory-Mapped-Modus zum Lesen des FSBL-Images. Im Memory-Mapped-Modus ist BUSY auf dem STM32N6 **permanent 1**. `HAL_XSPI_Init()` wartet auf BUSY=0 → 5-Sekunden-Timeout. Das BSP versucht dann, mit dem Flash in SPI-Modus zu kommunizieren — der Flash ist aber noch in DOPI → kein Antwort → hängt in `HAL_Receive`.

**Lösung:** Neue Funktion `FSBL_NOR_PreReset()` in `FSBL/Core/Src/main.c`, die **vor** `BSP_XSPI_NOR_Init()` aufgerufen wird:
1. XSPI2 per RCC zwangsweise zurücksetzen (`FORCE_RESET` + `RELEAHSE_RESET`) → löscht BUSY und alle Register
2. XSPI2 minimal initialisieren (`HAL_XSPI_Init` + `HAL_XSPIM_Config`)
3. OPI DTR Reset Enable (0x6699) + Reset Memory (0x9966) an den Flash senden
4. 15 ms warten (tRST des MX66UW1G45G)

Der Flash ist danach wieder im SPI-Modus. `BSP_XSPI_NOR_Init()` kann dann normal mit dem Flash kommunizieren.

**Betroffene Datei:** `FSBL/Core/Src/main.c` — neue Funktion `FSBL_NOR_PreReset()`, aufgerufen am Anfang von `FSBL_XSPI_Init()`

---

#### Fix 4 — XSPI2-Taktquelle HCLK löst langen Kalibrierungsvorgang aus

**Problem:** CubeMX setzt `RCC_XSPI2CLKSOURCE_HCLK` für XSPI2. Auf dem STM32N6 löst die **erste** Konfiguration von XSPI2 auf die HCLK-Quelle einen internen Clock-Kalibrierungsvorgang aus, der BUSY **erneut** für >5 Sekunden auf 1 hält — auch nach dem RCC-Reset aus Fix 3. Ergebnis: `FSBL_NOR_PreReset()`s `HAL_XSPI_Init()` lief in den Timeout, gab `HAL_TIMEOUT` zurück, die Funktion returnierte früh → OPI DTR Reset wurde **nie gesendet** → BSP schlug weiter fehl.

**Lösung:** In `FSBL/Core/Src/stm32n6xx_hal_msp.c` XSPI2-Taktquelle auf IC3 = PLL1/6 = 200 MHz umgestellt (wie beide ST-Referenzprojekte VENC_USB und FSBL_Modes XIP):

```c
PeriphClkInitStruct.PeriphClockSelection        = RCC_PERIPHCLK_XSPI2;
PeriphClkInitStruct.Xspi2ClockSelection         = RCC_XSPI2CLKSOURCE_IC3;
PeriphClkInitStruct.ICSelection[RCC_IC3].ClockSelection = RCC_ICCLKSOURCE_PLL1;
PeriphClkInitStruct.ICSelection[RCC_IC3].ClockDivider   = 6;  /* 1200 MHz / 6 = 200 MHz */
```

IC3 ist ein dedizierter PLL-Divider — kein Kalibrierungsvorgang, BUSY bleibt nach dem RCC-Reset auf 0. `HAL_XSPI_Init()` kehrt sofort zurück, `FSBL_NOR_PreReset()` sendet den Reset erfolgreich.

**Hinweis:** XSPI1 (HyperRAM) bleibt auf HCLK — der Boot ROM konfiguriert XSPI1 nicht, daher tritt der Kalibrierungseffekt dort nicht auf.

**Betroffene Datei:** `FSBL/Core/Src/stm32n6xx_hal_msp.c` — `HAL_XSPI_MspInit()`, XSPI2-Zweig

---

### 1.3 Appli Startup Fixes (April 2026) ✅

#### Fix 1 — HAL_Delay hängt mit ThreadX SysTick_Handler

**Problem:** Mit `USE_THREADX_AI_RUNTIME` definiert, wird `stm32n6xx_it.c`s `SysTick_Handler` (der `HAL_IncTick()` aufruft) durch `#ifndef USE_THREADX_AI_RUNTIME` ausgeschlossen. Stattdessen gewinnt `tx_initialize_low_level.S`s ThreadX `SysTick_Handler` die Link-Auflösung. Dieser ThreadX-Handler ruft nur `_tx_timer_interrupt` auf, **nicht** `HAL_IncTick()`. Resultat: `uwTick` bleibt bei 0. Im LED-Blink-Test (Zeile 174-179 von `Appli/Core/Src/main.c`) schlägt `HAL_Delay(200)` sofort fehl:
```c
for (uint32_t i = 0; i < 4; i++) {
  HAL_GPIO_TogglePin(GPIOO, GPIO_PIN_1);
  HAL_Delay(200);  // ← ckeckt: (0 - 0) >= 200 ? Nein → Endlosschleife!
}
```

Dies geschieht **vor** `MX_ThreadX_Init()`, daher ist ThreadX noch nicht initialisiert. Appli sieht aus wie nie gestartet.

**Lösung:** `BL HAL_IncTick` vor `BL _tx_timer_interrupt` in `tx_initialize_low_level.S` hinzufügen (beide ARMCC und GCC Abschnitte):
```asm
BL      HAL_IncTick                         // Keep HAL tick alive
BL      _tx_timer_interrupt
```

**Betroffene Datei:** `Appli/Core/Src/tx_initialize_low_level.S` — SysTick_Handler (beide Abschnitte)

---

#### Fix 2 — FSBL JumpToApplication Sanity Check fehlt Error_Handler()

**Problem:** Die Sanity Check (Zeile 502-507 in `FSBL/Core/Src/main.c`) detectiert ungültige Vektor-Tabelle-Werte (siehe unten) und setzt `g_fsbl_jump_stage = 0xE001U`, aber es folgt **kein `return`** oder `Error_Handler()` — fällt durch in Jump mit Müll-Adressen.

```c
if ((g_fsbl_app_msp < 0x34000000UL) || (g_fsbl_app_msp > 0x34200000UL) ||
    (g_fsbl_app_reset < 0x70000001UL) || (g_fsbl_app_reset > 0x70400001UL) ||
    ((g_fsbl_app_reset & 1UL) == 0UL)) {
  g_fsbl_jump_stage = 0xE001U;
  // ← KEINE RETURN! BUG
}
```

**Lösung:** `Error_Handler()` Aufruf hinzufügen:
```c
g_fsbl_jump_stage = 0xE001U;
Error_Handler(); /* APPLI vector table is invalid — halt, do not jump */
```

**Betroffene Datei:** `FSBL/Core/Src/main.c` — `JumpToApplication()`, Zeile ~507

---

#### Fix 3 — LCD Framebuffer überschreibt .data/.bss Globals

**Problem:** `LCD_FB_ADDRESS` war auf `0x34000000U` definiert — die gleiche Adresse wie `.data` im Linker-Skript. Der Framebuffer-Fill (Zeile 199-202 von `Appli/Core/Src/main.c`) schreibt `640×480×2 = 614,400 Byte`:
```c
uint16_t *fb = (uint16_t *)LCD_FB_ADDRESS;
for (uint32_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++)
  fb[i] = 0x001F; /* Blue */
```

Dies überschreibt alle globalen Variablen (`.data` = 808 Byte, `.bss` folgt direkt nach). ThreadX und AI runtime state sind corrupted, bevor sie initialisiert sind.

**Lösung:** `LCD_FB_ADDRESS` nach `0x34400000U` (AXISRAM3) verschieben:
```c
#define LCD_FB_ADDRESS        0x34400000U  /* AXISRAM3 — avoids overlap with .data/.bss at 0x34000000 */
```

AXISRAM3 ist bereits aktiviert durch `HAL_RAMCFG_EnableAXISRAM(&hramcfg_SRAM3)` (Zeile 194) und `MX_LTDC_Init` benutzt `LCD_FB_ADDRESS` direkt für `pLayerCfg.FBStartAdress`, also ist kein weiterer Change nötig.

**Betroffene Datei:** `Appli/Core/Src/main.c` — `#define LCD_FB_ADDRESS`

---

#### Sanity Check Debugging

Falls der Code weiterhin bei der Sanity Check stehen bleibt (und `Error_Handler()` auslöst), liegt ein ungültiger Vektor-Tabelle vor. Die Check prüft:

| Bedingung | Sollte sein | Wenn fehl | Ursache |
|-----------|------------|-----------|---------|
| `g_fsbl_app_msp` | `[0x34000000, 0x34200000]` | MSP außerhalb RAM | .isr_vector[0] nicht gelesen |
| `g_fsbl_app_reset` | `[0x70000001, 0x70400001]` | Reset-Handler außerhalb XiP | .isr_vector[1] nicht gelesen |
| `g_fsbl_app_reset & 1` | `!= 0` (Thumb-Bit) | Reset-Handler ist ARM32, nicht Thumb | Linker-Fehler |

**Debugging-Schritte:**
1. **ELF-Vektor-Tabelle prüfen:**
   ```bash
   arm-none-eabi-objdump -h build/Appli/CDC_ACM_Appli.elf | grep -A5 isr_vector
   arm-none-eabi-readelf -x .isr_vector build/Appli/CDC_ACM_Appli.elf
   ```
   Sollte zeigen:
   - Offset 0: MSP = 0x34200000 oder etwas näher 0x34000000
   - Offset 4: Reset_Handler = 0x70xxxxxx mit Thumb-Bit (ungerade Adresse)

2. **Nach dem Flash — Laufzeit-Debugging:**
   FSBL setzt `g_fsbl_app_msp` und `g_fsbl_app_reset` auf Werte aus der geflashten Binärdatei (nicht dem ELF). Mit einem Debugger oder Terminalausgabe vor dem Jump prüfen:
   ```c
   // In JumpToApplication(), vor dem Jump
   printf("app_msp = 0x%08lX (expect [0x34000000..0x34200000])\n", app_msp_primary);
   printf("app_reset = 0x%08lX (expect [0x70000001..0x70400001], Thumb=%d)\n", 
          app_reset_primary, app_reset_primary & 1);
   ```

3. **Häufige Fehler:**
   - Flash wurde nicht programmiert → alle Bytes sind 0xFF → Vector-Tabelle ungültig
   - Signing-Tool hat Header versatz falsch berechnet → Payload startet nicht bei 0x70100400
   - Linker-Skript platziert .isr_vector nicht richtig in ROM
   - Endianness-Fehler (sollte nicht vorkommen, aber: MSP/Reset als uint32_t lesen, nicht als bytes)

---

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

### 2.3 FSBL XSPI-Init-Kette — Übersicht aller Änderungen

Die vollständige Init-Kette in `FSBL_XSPI_Init()` (aufgerufen aus FSBL `main()`):

```
SystemInit()
  └─ RCC-Reset von XSPI2 + XSPIM (system_stm32n6xx_fsbl.c, bereits korrekt)

FSBL_XSPI_Init()
  ├─ FSBL_NOR_PreReset()            [NEU]
  │    ├─ __HAL_RCC_XSPI2_FORCE_RESET / RELEASE_RESET
  │    ├─ HAL_XSPI_Init (hpre, ClkPrescaler=3 → 37.5 MHz)
  │    ├─ HAL_XSPIM_Config (IOPort=2, NCS1)
  │    ├─ HAL_XSPI_Command: 0x6699 (OPI DTR Reset Enable)
  │    ├─ HAL_XSPI_Command: 0x9966 (OPI DTR Reset Memory)
  │    └─ HAL_Delay(15)             [Flash: tRST max. 15 ms]
  │
  ├─ BSP_XSPI_NOR_Init(OPI_STR)    [Flash jetzt in SPI → BSP kann kommunizieren]
  ├─ BSP_XSPI_NOR_EnableMemoryMappedMode()
  ├─ BSP_XSPI_RAM_Init()
  └─ BSP_XSPI_RAM_EnableMemoryMappedMode()

MX_XSPI_NOR_Init() [starke Überschreibung, NEU]
  └─ HAL_XSPIM_Config für XSPI2 (GPION, IOPort_2, NCS1)

MX_XSPI_RAM_Init() [starke Überschreibung, NEU]
  └─ HAL_XSPIM_Config für XSPI1 (GPIOP/O, IOPort_1, NCS1)
```

**stm32n6xx_hal_msp.c — XSPI2 Clock [GEÄNDERT]:**
- Alt: `RCC_XSPI2CLKSOURCE_HCLK` → löst langen Kalibrierungsvorgang aus (BUSY=1 für >5 s)
- Neu: `RCC_XSPI2CLKSOURCE_IC3`, IC3 = PLL1/6 = 200 MHz → kein Kalibrierungsvorgang

---

### 2.4 FSBL JumpToApplication — Korrekte Implementierung für STM32N6

Der Jump von FSBL zur Appli erfordert auf dem Cortex-M55 (ARM v8.1-M) eine spezifische Sequenz:

```c
/* 1. SysTick suspendieren */
HAL_SuspendTick();

/* 2. I-Cache deaktivieren (Appli konfiguriert Cache neu) */
SCB_DisableICache();

/* 3. Interrupts sperren */
primask_bit = __get_PRIMASK();
__disable_irq();

/* 4. VTOR auf Appli-Adresse */
SCB->VTOR = APP_START_ADDRESS;  // 0x70100400

/* 5. Reset_Handler aus Vektor-Tabelle lesen */
JumpToApp = (pFunction)(*(__IO uint32_t *)(APP_START_ADDRESS + 4U));

/* 6. MSPLIM löschen VOR MSP-Änderung (ARM v8.1-M Pflicht!) */
__set_MSPLIM(0x00000000);

/* 7. MSP aus Appli-Vektor-Tabelle */
__set_MSP(*(__IO uint32_t *)APP_START_ADDRESS);

/* 8. Memory Barriers */
__DSB(); __ISB();

/* 9. Interrupts freigeben und springen */
__set_PRIMASK(primask_bit);
JumpToApp();
```

**Wichtig:** `HAL_RCC_DeInit()` darf **nicht** vor dem Jump aufgerufen werden, da die Appli aus XSPI2 ausgeführt wird und der Memory-Mapped-Modus erhalten bleiben muss.

### 2.5 Linker / externes Memory

Aufgrund ROM-Ueberlauf wurde Appli auf ein duales externes ROM-Layout umgestellt:

- Linker-Skript: STM32N657XX_ROMxspi1xspi2_RAMxspi3.ld
- Code (.text) bleibt in ROM
- grosse Konstanten (.rodata) liegen in ROM2

Damit passt die Gesamtauslastung wieder in den verfuegbaren Addressraum.

Fuer den Dual-Model-Betrieb wurde ROM2 im Linker auf 2047K erweitert
(`STM32N657XX_ROMxspi1xspi2_RAMxspi3.ld`), damit beide Netzwerke gleichzeitig
gelinkt werden koennen.

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
- App_AI_OnOutputs(const uint8_t* const* outputs, const size_t* output_sizes, uint32_t output_count)
- App_AI_GetRequestedModel(void) fuer die Laufzeitwahl zwischen Pose und Segmentierung

Zusatz zur Modellauswahl:

- `App_AI_SetModel(APP_AI_MODEL_POSE)` und `App_AI_SetModel(APP_AI_MODEL_SEGMENTATION)` schalten das aktive Modell.
- Der AI-Thread liest in jedem Zyklus `App_AI_GetRequestedModel()` und setzt das aktive Modell entsprechend.
- Ohne Override bleibt standardmaessig Pose aktiv.
- Neu: Auf STM32N6570-DK toggelt ein Druck auf USER1 (B2) im Runtime-Betrieb zwischen Pose und Segmentierung (mit Entprellung im ThreadX-Loop).

Standard-Overlay (bereits integriert):

- Bei Segmentierung wird bevorzugt ein YOLOv8-Instance-Segmentation-Overlay aus zwei Modellausgaengen gezeichnet (Detections + Mask-Prototypes, inklusive einfacher NMS).
- Falls nur ein Segmentierungs-Output verfuegbar ist, wird auf den Map-Overlay-Fallback gewechselt.
- Bei Pose wird zuerst ein YOLOv8-Pose-Output erkannt und gezeichnet; falls das Format nicht passt, bleibt der Heatmap-Keypoint-Fallback aktiv.
- Standard-Framebuffer: `0x34000000`, Standard-Aufloesung: `800x480` (STM32N6570-DK; ueber weak Funktionen anpassbar).
- Zusaetzlicher Statusmarker links oben: Blau = Pose, Gruen = Segmentierung; rotes Blinkfeld = laufende Inferenz.
- Sicherheits-Hinweis: Standardmaessig ist kein Display-Framebuffer gesetzt (`App_AI_GetDisplayFramebuffer()` liefert NULL), damit keine AI-FlexMEM-Adressen ueberschrieben werden.
- Fuer sichtbare Overlays muss `App_AI_GetDisplayFramebuffer()` projektspezifisch auf den echten LTDC/Display-Buffer ueberschrieben werden.

Sichtbare Runtime-Diagnose (neu):

- LED1 (Gruen) blinkt bei jeder Inferenz.
- LED2 (Rot) zeigt aktives Modell: aus = Pose, an = Segmentierung.

Diese Hooks sind der vorgesehene Punkt fuer Kamera-Preprocessing und LTDC-Overlay-Ausgabe.

## 4. Kamera/LTDC/ThreadX-Schutz

Wichtiges Ziel war, bestehende Laufzeitpfade nicht zu blockieren:

1. AI Prozess ist nicht-blockierend.
2. ISR-Konflikte wurden fuer ThreadX-Runtime abgefangen (SVC/PendSV/SysTick Guards).
3. Kamera/LTDC-spezifische Verarbeitung ist nicht hart verdrahtet im AI-Code, sondern ueber Hooks vorgesehen.

Hinweis: Die konkrete Pose-Visualisierung auf dem Display ist erst dann aktiv, wenn App_AI_OnResult mit deiner Overlay-Logik befuellt ist.

## 5. Relevante Dateien

**FSBL (Sprung-relevante Dateien):**
- FSBL/Core/Src/main.c — `JumpToApplication()`, `FSBL_NOR_PreReset()`, `FSBL_XSPI_Init()`, `MX_XSPI_NOR_Init()` (stark), `MX_XSPI_RAM_Init()` (stark), `HAL_TIM_PeriodElapsedCallback()`
- FSBL/Core/Src/stm32n6xx_hal_msp.c — `HAL_XSPI_MspInit()`: XSPI2 Taktquelle IC3=PLL1/6=200MHz
- FSBL/Core/Src/system_stm32n6xx_fsbl.c — `SystemInit()`: RCC-Reset von XSPI2+XSPIM

**Appli:**
- Appli/CMakeLists.txt
- Appli/ai-integration.cmake
- Appli/STM32N657XX_ROMxspi1xspi2_RAMxspi3.ld
- Appli/AI/App/app_x-cube-ai.c
- Appli/AI/App/app_x-cube-ai.h
- Appli/AI/App/network_pose_wrap.c
- Appli/AI/App/network_seg_wrap.c
- Appli/AI/models/pose/generated/*
- Appli/AI/models/seg/generated/*
- Appli/Core/Src/main.c
- Appli/Core/Src/app_threadx.c
- Appli/Core/Src/tx_initialize_low_level.S
- Appli/Core/Src/stm32n6xx_it.c

## 6. Build

Getesteter Build-Target:

- CDC_ACM_Appli

### 6.0 Dual-xSPI Memory Layout (FIXED)

**WICHTIG:** Das Projekt wurde auf ein Dual-xSPI-Layout migriert, um beide AI-Modelle optimal zu unterstützen.

**Alte Konfiguration (fehlerhaft):**
- Linker-Skript: `STM32N657XX_ROMxspi1.ld`
- ROM: 2047K (single xSPI1)
- Modelle: 5,88 MB (2,70 MB Pose + 3,18 MB Segmentation)
- ROM-Auslastung: **84,46%** ❌ (nur auf xSPI1)

**Neue Konfiguration (korrekt):**
- Linker-Skript: `STM32N657XX_ROMxspi1xspi2_RAMxspi3.ld`
- Code (ROM): 511K auf xSPI1 (OctaSPI)
- Models/Constants (ROM2): 2047K auf xSPI2 (OctaSPI)
- ROM Auslastung: **27,84%** ✅
- ROM2 Auslastung: **46,76%** ✅
- **Nur OctaSPI wird verwendet** (keine HexaSPI)

**Memory Distribution nach Build:**
```
Memory region         Used Size  Region Size  %age Used
         ROM:      145668 B       511 KB     27.84%
        ROM2:      980120 B      2047 KB     46,76%
         RAM:       11208 B         2 MB      0.53%
      EXTRAM:           0 B        64 MB      0.00%
```

Ergebnis:

- Build erfolgreich
- Warning vorhanden: LOAD segment with RWX permissions
  - diese Warning kommt vom aktuellen Linker-Layout und ist fuer den funktionalen Build nicht blockierend

## 6.1 Flash-Layout und Ladevorgang (optimiert für Dual-Model mit OctaSPI)

**ÄNDERUNG ab März 2026:** Das Projekt verwendet jetzt Dual-xSPI (OctaSPI) für optionale Modellplatzierung.

Im aktuellen Stand werden drei Programmabschnitte in den externen Flash geschrieben:

1. FSBL signiertes Image → 0x70000000 (xSPI1)
2. APPLI signiertes Image (Code + ROM2 Daten) → 0x70100000 (xSPI1 für Code, xSPI2 für ROM2)
3. APPLI ROM2 Daten (falls separate Programmierung nötig) → 0x90200400 (xSPI2 optional)

### Adress-Mapping

**xSPI1 (OctaSPI Interface 1) — 0x70000000–0x71FFFFFF:**
- FSBL: 0x70000000
- APPLI Core (Code): 0x70100000

**xSPI2 (OctaSPI Interface 2) — 0x90000000–0x91FFFFFF:**
- APPLI ROM2 (Modellgewichte): 0x90200400

### Warum Dual-xSPI?

Die Appli nutzt zwei ROM-Bereiche (ROM und ROM2), die auf unterschiedliche xSPI-Interfaces gemappt sind:

**ROM (xSPI1, 511K):**
- Linker-Skript Bereich für `.text`, `.ARM`, `.init_array`, etc.
- Beinhaltet: Application Code, Runtime, ThreadX

**ROM2 (xSPI2, 2047K):**
- Linker-Skript Bereich für `.rodata`
- Beinhaltet: **Netzwerk-Gewichte** (beide AI-Modelle), statische Konstanten
- Automatic Platzierung: Alle `static const` Daten (incl. `network_ecblobs.h` Blobs) landen hier

### Warum kein separates Binary für ROM2?

Im Gegensatz zur alten Konfiguration müssen die Modellgewichte **nicht als separates Hex-File programmiert** werden. Die `network_ecblobs.h` Blobs werden als `static const uint64_t` Arrays definiert und landen mit dem Linker-Skript automatisch in der `.rodata` Section, die nach ROM2 (xSPI2) geht.

**Technischer Ablauf im Skript**

Das Skript in sign_binaries.bat führt diese Schritte aus:

1. FSBL .bin signieren zu CDC_ACM_FSBL-trusted.bin.
2. APPLI aus CDC_ACM_Appli.elf aufteilen:
  - CDC_ACM_Appli-core.bin (enthält alle Code + Daten, die der Linker in ROM und ROM2 platziert)
  - Falls nötig: Separate ROM2-Extraktion (für Laufzeitdynamik)
3. APPLI signieren zu CDC_ACM_Appli-trusted.bin.
4. Mit STM32_Programmer_CLI programmieren:
  - FSBL-trusted nach 0x70000000
  - APPLI-trusted nach 0x70100000
  - (ROM2 wird teil von APPLI-trusted oder separat nach 0x90200400, falls implementiert)

**Verwendete Optionen für robustes Flashing**

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

Der Upload gilt als erfolgreich, wenn fuer alle programmierten Files jeweils File download complete gemeldet wird.

Hinweis:

- Ein optionaler MCU-Reset am Ende kann fehlschlagen, ohne den Programmiervorgang ungueltig zu machen.
- Falls die Anwendung nicht automatisch startet, RESET-Taste am Board druecken.

### Migration von altem Linker-Skript

Falls `sign_binaries.bat` aktualisiert werden muss für explizite ROM2-Extraktion:

```bat
REM === Extract ROM2 from ELF only (new dual-xSPI mode) ===
REM The Linker now automatically places .rodata in ROM2 (xSPI2 @ 0x90200400)
REM If needed for validation, extract with:
arm-none-eabi-objcopy -S --change-section-address .rodata=0x90200400 ^
  CDC_ACM_Appli.elf CDC_ACM_Appli-rom2.bin
```

Dies ist **optional** — der Standard ist, APPLI komplett programmieren zu lassen.

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
