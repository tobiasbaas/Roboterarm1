/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : FSBL (First Stage Bootloader) - minimal bootloader only
  * 
  * PURPOSE: 
  *   - Minimal hardware initialization required for boot
  *   - Copy RAM sections if needed
  *   - Jump to Appli vector table at 0x70100400
  *   - Do NOT initialize Camera, Display, AI threads (those are Appli responsibility)
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "stm32n657xx.h"
#include "stm32n6xx_hal_gpio.h"
#include "stm32n6570_discovery_xspi.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MPU_Config(void);
static void FSBL_NOR_PreReset(void);
static void FSBL_XSPI_Init(void);
static void JumpToApplication(void);

/* Debug-visible jump diagnostics (avoid relying on optimized locals). */
volatile uint32_t g_fsbl_jump_stage = 0U;
volatile uint32_t g_fsbl_app_msp = 0U;
volatile uint32_t g_fsbl_app_reset = 0U;


/**
  * @brief  The bootloader entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* Minimal MPU configuration for boot */
  SCB_EnableICache();
  /* USER CODE END 1 */

  /* MCU Configuration --------------------------------------------------------*/
  HAL_Init();

  /* Configure the system clock (PLL1 1200 MHz) */
  SystemClock_Config();

  /* Initialize minimal GPIO (only critical pins if needed) */
  MX_GPIO_Init();

  /* USER CODE BEGIN 2 */
  /* Enable external SRAM clocks for Appli to use */
  __HAL_RCC_AXISRAM3_MEM_CLK_ENABLE();
  __HAL_RCC_AXISRAM4_MEM_CLK_ENABLE();

  /* Configure MPU: mark XSPI2 (0x70000000) executable, XSPI1 (0x90000000) cacheable */
  MPU_Config();

  /* Initialize XSPI2 (NOR Flash) + XSPI1 (HyperRAM) in memory-mapped mode */
  FSBL_XSPI_Init();

  for (uint32_t i = 0; i < 4; i++)
  {
    HAL_GPIO_TogglePin(GPIOO, GPIO_PIN_1);
    HAL_Delay(200);
  }

  /* USER CODE END 2 */

  /* ========================================================================
     BOOTLOADER COMPLETE - JUMP TO APPLI WITH PROPER VECTOR TABLE SETUP
     ======================================================================== */

  JumpToApplication();

  /* We should never reach here */
  while (1)
  {
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the System Power Supply */
  if (HAL_PWREx_ConfigSupply(PWR_EXTERNAL_SOURCE_SUPPLY) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the main internal regulator output voltage */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE0) != HAL_OK)
  {
    Error_Handler();
  }

  /* Enable HSI as startup clock */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL1.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.PLL2.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.PLL3.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.PLL4.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes TIMPRE for TIM clock */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_TIM;
  PeriphClkInitStruct.TIMPresSelection = RCC_TIMPRES_DIV1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Switch from HSI to HSI first if needed */
  HAL_RCC_GetClockConfig(&RCC_ClkInitStruct);
  if ((RCC_ClkInitStruct.CPUCLKSource == RCC_CPUCLKSOURCE_IC1) ||
      (RCC_ClkInitStruct.SYSCLKSource == RCC_SYSCLKSOURCE_IC2_IC6_IC11))
  {
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_SYSCLK);
    RCC_ClkInitStruct.CPUCLKSource = RCC_CPUCLKSOURCE_HSI;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }
  }

  /* Configure PLL1 from HSI for 800 MHz CPU (matching ST reference)
   * PLL1 = HSI(64 MHz) × 25 / 2 = 800 MHz */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE; /* HSI already on */
  RCC_OscInitStruct.PLL1.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL1.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL1.PLLM = 2;
  RCC_OscInitStruct.PLL1.PLLN = 25;
  RCC_OscInitStruct.PLL1.PLLFractional = 0;
  RCC_OscInitStruct.PLL1.PLLP1 = 1;
  RCC_OscInitStruct.PLL1.PLLP2 = 1;
  RCC_OscInitStruct.PLL2.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.PLL3.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.PLL4.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Configure clocks: CPU/SYS from PLL1, APB from IC dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_HCLK
                                | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1
                                | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK5
                                | RCC_CLOCKTYPE_PCLK4;
  RCC_ClkInitStruct.CPUCLKSource = RCC_CPUCLKSOURCE_IC1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_IC2_IC6_IC11;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;
  RCC_ClkInitStruct.APB5CLKDivider = RCC_APB5_DIV1;
  RCC_ClkInitStruct.IC1Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
  RCC_ClkInitStruct.IC1Selection.ClockDivider = 2;
  RCC_ClkInitStruct.IC2Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
  RCC_ClkInitStruct.IC2Selection.ClockDivider = 3;
  RCC_ClkInitStruct.IC6Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
  RCC_ClkInitStruct.IC6Selection.ClockDivider = 3;
  RCC_ClkInitStruct.IC11Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
  RCC_ClkInitStruct.IC11Selection.ClockDivider = 3;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function (minimal)
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOO_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* User button pin (PC13) as output for quick FSBL tests */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* LED2 (PG10) als Output für Debug-Blink */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /* LED1 Green on STM32N6570-DK: GPIOO PIN1 */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOO, &GPIO_InitStruct);

    /* Ensure LED starts from OFF state (LED1 is high-active on DK) */
    HAL_GPIO_WritePin(GPIOO, GPIO_PIN_1, GPIO_PIN_RESET);
  /* Optional: Initialize only critical GPIOs here if needed */
  /* Appli will re-initialize all GPIO pins it needs */
  /* USER CODE END MX_GPIO_Init_2 */
}

/**
  * @brief  Configure MPU for XSPI memory windows.
  *
  * The Cortex-M55 default memory map treats 0x60000000-0x7FFFFFFF as Device
  * memory (non-cacheable, implementation-defined execute permission). We must
  * add explicit Normal-Cacheable regions so that:
  *   - 0x70000000 (XSPI2 / NOR Flash) is executable for XiP
  *   - 0x90000000 (XSPI1 / HyperRAM)  is cacheable for data use
  *
  * The Appli will call its own MPU_Config() which starts with HAL_MPU_Disable(),
  * so these FSBL regions are automatically replaced on handover.
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef     MPU_InitStruct = {0};
  MPU_Attributes_InitTypeDef MPU_AttrInit   = {0};

  HAL_MPU_Disable();

  /* Attribute 0: Normal Write-Back Non-Transient Read+Write-Allocate
     Used for both NOR Flash (cached XiP reads) and HyperRAM (cached R/W). */
  MPU_AttrInit.Number     = MPU_ATTRIBUTES_NUMBER0;
  MPU_AttrInit.Attributes = INNER_OUTER(MPU_NON_TRANSIENT | MPU_WRITE_BACK | MPU_RW_ALLOCATE);
  HAL_MPU_ConfigMemoryAttributes(&MPU_AttrInit);

  /* Region 0: XSPI2 NOR Flash  0x70000000 – 0x7FFFFFFF (256 MB)
     Executable (XN=0) – required for XiP at 0x70100400. */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress      = 0x70000000UL;
  MPU_InitStruct.LimitAddress     = 0x7FFFFFFFUL;
  MPU_InitStruct.AttributesIndex  = MPU_ATTRIBUTES_NUMBER0;
  MPU_InitStruct.AccessPermission = MPU_REGION_ALL_RW;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Region 1: XSPI1 HyperRAM   0x90000000 – 0x9FFFFFFF (256 MB)
     Non-executable (XN=1) – data RAM only. */
  MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress      = 0x90000000UL;
  MPU_InitStruct.LimitAddress     = 0x9FFFFFFFUL;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable MPU; privileged accesses to unmapped regions use the default map. */
  HAL_MPU_Enable(MPU_HFNMI_PRIVDEF);
}

/**
  * @brief  Initialize XSPI2 (MX66UW1G45G NOR Flash) and XSPI1 (APS256XX HyperRAM)
  *         in memory-mapped mode so the Appli can execute from 0x70100400 (XiP)
  *         and use HyperRAM at 0x90000000 without re-initializing.
  *
  * Why this is needed:
  *   The STM32N6 Boot ROM reads the FSBL from XSPI2 in indirect/command mode and
  *   copies it to AXISRAM2. After that jump, the FSBL reconfigures the system clock
  *   (SystemClock_Config), which changes HCLK — invalidating the Boot ROM's timing
  *   calibration for XSPI2. Without re-initializing the XSPI2 controller here,
  *   any read from 0x70xxxxxx causes a bus error (HardFault).
  *
  * The NOR Flash chip (MX66UW1G45G) is already in OctoSPI DTR mode (DOPI,
  * configured by the Boot ROM). BSP_XSPI_NOR_Init's reset sequence starts with
  * ReadStatusRegister in SPI mode — but the flash is in DOPI and won't respond,
  * causing HAL_XSPI_Receive to time out after 5s and leave XSPI2 hardware with
  * BUSY=1. The next BSP command (SPI ResetEnable) then also times out waiting
  * for BUSY=0, causing Error_Handler.
  *
  * Fix: FSBL_NOR_PreReset() sends OPI DTR Reset Enable + Reset Memory to the
  * flash first, returning it to SPI mode. After that, BSP_XSPI_NOR_Init's SPI
  * ReadStatusRegister succeeds, and the full BSP reset+configure sequence works.
  *
  * HyperRAM (APS256XX) is not used by the Boot ROM, so it requires full init.
  * The Appli bypasses MX_XSPI1_Init (return; guard) and expects the FSBL to
  * have already placed XSPI1 in memory-mapped mode at 0x90000000.
  */

/**
  * @brief  Pre-reset the NOR Flash from OPI DTR → SPI mode.
  *
  * Boot ROM leaves MX66UW1G45G in DOPI (OPI DTR) mode. The BSP_XSPI_NOR_Init
  * reset sequence opens with ReadStatusRegister in SPI mode (1-wire). Since the
  * flash is in DOPI it ignores the SPI command; HAL_XSPI_Receive times out after
  * 5 s and leaves XSPI2 hardware BUSY=1, causing every subsequent HAL call to
  * also time out. This function sends OPI DTR Reset Enable + Reset Memory (both
  * command-only, no data phase → no FT/TC wait, no hang) to put the flash back
  * in SPI mode BEFORE BSP_XSPI_NOR_Init is called.
  */
static void FSBL_NOR_PreReset(void)
{
  XSPI_HandleTypeDef     hpre  = {0};
  XSPIM_CfgTypeDef       xmgr  = {0};
  XSPI_RegularCmdTypeDef cmd   = {0};

  /* Boot ROM leaves XSPI2 in memory-mapped mode. On STM32N6, the BUSY flag is
   * asserted permanently while memory-mapped mode is active. HAL_XSPI_Init
   * waits for BUSY=0 before configuring the prescaler — it would hang for the
   * full 5 s timeout. Force-reset via RCC first to clear BUSY and all XSPI2
   * registers, just as BSP_XSPI_NOR_Init's XSPI_NOR_MspInit does.
   * (HAL_XSPI_MspInit does NOT do this reset; only the BSP MspInit does.) */
  __HAL_RCC_XSPI2_CLK_ENABLE();
  __HAL_RCC_XSPI2_FORCE_RESET();
  __HAL_RCC_XSPI2_RELEASE_RESET();

  /* Minimal XSPI2 init — enough to send two command-only OPI DTR frames.
   * HAL_XSPI_Init calls HAL_XSPI_MspInit which enables XSPIM/XSPI2 clocks,
   * sets HCLK as XSPI2 source, and configures GPION alternate functions. */
  hpre.Instance                     = XSPI2;
  hpre.Init.FifoThresholdByte       = 4;
  hpre.Init.MemoryMode              = HAL_XSPI_SINGLE_MEM;
  hpre.Init.MemoryType              = HAL_XSPI_MEMTYPE_MACRONIX;
  hpre.Init.MemorySize              = HAL_XSPI_SIZE_1GB;
  hpre.Init.ChipSelectHighTimeCycle = 2;
  hpre.Init.FreeRunningClock        = HAL_XSPI_FREERUNCLK_DISABLE;
  hpre.Init.ClockMode               = HAL_XSPI_CLOCK_MODE_0;
  hpre.Init.WrapSize                = HAL_XSPI_WRAP_NOT_SUPPORTED;
  hpre.Init.ClockPrescaler          = 3;   /* 150 MHz HCLK / 4 = 37.5 MHz */
  hpre.Init.SampleShifting          = HAL_XSPI_SAMPLE_SHIFT_NONE;
  hpre.Init.DelayHoldQuarterCycle   = HAL_XSPI_DHQC_DISABLE; /* not needed for cmd-only */
  hpre.Init.ChipSelectBoundary      = HAL_XSPI_BONDARYOF_NONE;
  hpre.Init.MaxTran                 = 0;
  hpre.Init.Refresh                 = 0;
  hpre.Init.MemorySelect            = HAL_XSPI_CSSEL_NCS1;
  if (HAL_XSPI_Init(&hpre) != HAL_OK) { return; }

  /* Route XSPI2 → Port P2 (NOR flash on GPION, XSPIM_P2) */
  xmgr.nCSOverride = HAL_XSPI_CSSEL_OVR_NCS1;
  xmgr.IOPort      = HAL_XSPIM_IOPORT_2;
  xmgr.Req2AckTime = 1;
  if (HAL_XSPIM_Config(&hpre, &xmgr, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return; }

  /* Shared command descriptor: 16-bit OPI DTR instruction, no address/data.
   * HAL_XSPI_Command only waits for BUSY=0 (no FT/TC), so it cannot hang. */
  cmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  cmd.IOSelect           = HAL_XSPI_SELECT_IO_3_0;
  cmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  cmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  cmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  cmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  cmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  cmd.DataMode           = HAL_XSPI_DATA_NONE;
  cmd.DummyCycles        = 0;
  cmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  /* Reset Enable (0x6699) then Reset Memory (0x9966) in OPI DTR mode */
  cmd.Instruction = 0x6699U;  /* MX66UW1G45G_OCTA_RESET_ENABLE_CMD */
  (void)HAL_XSPI_Command(&hpre, &cmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);

  cmd.Instruction = 0x9966U;  /* MX66UW1G45G_OCTA_RESET_MEMORY_CMD */
  (void)HAL_XSPI_Command(&hpre, &cmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);

  /* tRST: MX66UW1G45G needs up to 15 ms to complete the reset to SPI mode */
  HAL_Delay(15);

  /* Flash is now in SPI mode. XSPI2 will be force-reset by BSP's
   * XSPI_NOR_MspInit (XSPI_NOR_FORCE_RESET) when BSP_XSPI_NOR_Init runs. */
}

static void FSBL_XSPI_Init(void)
{
  BSP_XSPI_NOR_Init_t nor_init;

  /* Pre-reset: send OPI DTR Reset to flash so BSP's SPI ReadStatusRegister
   * (first call in XSPI_NOR_ResetMemory) can complete without hanging. */
  FSBL_NOR_PreReset();

  /* --- XSPI2: MX66UW1G45G NOR Flash → OctoSPI STR memory-mapped (XiP) --- */
  nor_init.InterfaceMode = BSP_XSPI_NOR_OPI_MODE;
  nor_init.TransferRate  = BSP_XSPI_NOR_STR_TRANSFER;
  if (BSP_XSPI_NOR_Init(0, &nor_init) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }
  if (BSP_XSPI_NOR_EnableMemoryMappedMode(0) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* --- XSPI1: APS256XX HyperRAM → memory-mapped for Appli use ----------- */
  if (BSP_XSPI_RAM_Init(0) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }
  if (BSP_XSPI_RAM_EnableMemoryMappedMode(0) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }
}

/**
  * @brief  Jump to Application vector table at 0x70100400
  * @retval None (does not return)
  *
  * Note: HAL_RCC_DeInit() is intentionally NOT called here.
  * The FSBL runs from AXISRAM2, but the Appli runs from xSPI1 (0x70100400).
  * Resetting RCC would risk losing the xSPI1 memory-mapped mode needed for
  * the CPU to fetch instructions after the jump. The Appli reconfigures
  * clocks itself in its own SystemClock_Config().
  */
static void JumpToApplication(void)
{
  /*
  * Appli vector table is linked at 0x70100400 (xSPI1 ROM, after FSBL at 0x70000000).
  * For trusted images, the binary must be flashed 0x400 bytes earlier
  * (at 0x70100000), so payload still starts at 0x70100400.
  * FSBL Memory Layout:
  *   0x70000000 - 0x7003FFFF: FSBL (255 KB, from linker script)
  *
  * Appli Memory Layout:
  *   0x70100000 - 0x701003FF: Signing header (trusted image)
  *   0x70100400 - 0x7025FFFF: Appli ROM payload and vector table
  *   0x70200400 - 0x7040FFFF: AI Models in ROM2 (2047 KB xSPI2)
  */

  #define APP_VECTOR_ADDRESS      0x70100400UL
  #define APP_VECTOR_ALT_ADDRESS  (APP_VECTOR_ADDRESS + 0x400UL)
  typedef void (*pFunction)(void);
  static pFunction JumpToApp;
  uint32_t app_vector = APP_VECTOR_ADDRESS;
  uint32_t app_msp_primary;
  uint32_t app_reset_primary;
  uint32_t app_msp_alt;
  uint32_t app_reset_alt;

  g_fsbl_jump_stage = 1U;
  app_msp_primary = *(__IO uint32_t *)APP_VECTOR_ADDRESS;
  app_reset_primary = *(__IO uint32_t *)(APP_VECTOR_ADDRESS + 4U);
  app_msp_alt = *(__IO uint32_t *)APP_VECTOR_ALT_ADDRESS;
  app_reset_alt = *(__IO uint32_t *)(APP_VECTOR_ALT_ADDRESS + 4U);

  /* Prefer linked vector base (0x70100400). If invalid, fallback to +0x400. */
  if ((app_msp_primary >= 0x34000000UL) && (app_msp_primary <= 0x34200000UL) &&
      (app_reset_primary >= 0x70000001UL) && (app_reset_primary <= 0x70400001UL) &&
      ((app_reset_primary & 1UL) != 0UL))
  {
    app_vector = APP_VECTOR_ADDRESS;
    g_fsbl_app_msp = app_msp_primary;
    g_fsbl_app_reset = app_reset_primary;
  }
  else if ((app_msp_alt >= 0x34000000UL) && (app_msp_alt <= 0x34200000UL) &&
           (app_reset_alt >= 0x70000001UL) && (app_reset_alt <= 0x70400001UL) &&
           ((app_reset_alt & 1UL) != 0UL))
  {
    app_vector = APP_VECTOR_ALT_ADDRESS;
    g_fsbl_app_msp = app_msp_alt;
    g_fsbl_app_reset = app_reset_alt;
  }
  else
  {
    g_fsbl_app_msp = app_msp_primary;
    g_fsbl_app_reset = app_reset_primary;
  }

  /* Basic vector sanity checks: valid RAM MSP + Thumb reset handler in XiP window. */
  if ((g_fsbl_app_msp < 0x34000000UL) || (g_fsbl_app_msp > 0x34200000UL) ||
      (g_fsbl_app_reset < 0x70000001UL) || (g_fsbl_app_reset > 0x70400001UL) ||
      ((g_fsbl_app_reset & 1UL) == 0UL))
  {
    g_fsbl_jump_stage = 0xE001U;
    Error_Handler(); /* APPLI vector table is invalid — halt, do not jump */
  }

  /* 1. Suspend SysTick — Appli will reconfigure it */
  HAL_SuspendTick();
  g_fsbl_jump_stage = 2U;

  /* 2. Disable I-Cache so Appli starts with a clean cache state */
  if (SCB->CCR & SCB_CCR_IC_Msk)
  {
    SCB_DisableICache();
  }

  /* 3. Disable the MPU so APPLI's SystemInit and startup run against the
   *    default memory map; APPLI will call its own MPU_Config() early. */
  HAL_MPU_Disable();

  /* 4. Disable and clear all pending interrupts. Interrupts stay disabled
   *    across the jump; APPLI's Reset_Handler and HAL_Init() will re-enable
   *    them at the right time. */
  __disable_irq();
  for (uint32_t i = 0U; i < 8U; i++)
  {
    NVIC->ICER[i] = 0xFFFFFFFFUL;
    NVIC->ICPR[i] = 0xFFFFFFFFUL;
  }
  g_fsbl_jump_stage = 3U;

  /* 5. Point VTOR at the APPLI vector table. */
  SCB->VTOR = app_vector;
  g_fsbl_jump_stage = 4U;

  /* 6. Load the APPLI Reset_Handler into a local before we touch MSP —
   *    after this point the compiler must not spill locals to the (old)
   *    stack. */
  JumpToApp = (pFunction)g_fsbl_app_reset;
  g_fsbl_jump_stage = 5U;

  /* 7. Cortex-M55: clear MSPLIM before changing MSP. */
  __set_MSPLIM(0x00000000UL);
  __set_MSP(g_fsbl_app_msp);
  g_fsbl_jump_stage = 6U;

  /* 8. Barriers: ensure SCB->VTOR and MSP writes are visible before jump. */
  __DSB();
  __ISB();

  /* 9. Jump. APPLI Reset_Handler re-sets MSP/MSPLIM from its own vector
   *    table and then calls SystemInit/main. Interrupts remain masked. */
  g_fsbl_jump_stage = 8U;
  JumpToApp();
  g_fsbl_jump_stage = 0xE002U;

  /* THIS SHOULD NEVER BE REACHED */
  while (1)
  {
    /* Halt forever if jump fails */
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

/**
  * @brief  Period elapsed callback for the HAL timebase timer (TIM6).
  *
  * The FSBL uses TIM6 as the HAL timebase (configured in
  * stm32n6xx_hal_timebase_tim.c).  TIM6_IRQHandler calls
  * HAL_TIM_IRQHandler which in turn calls this weak override.
  * Without this override HAL_IncTick() is never called → uwTick
  * stays at 0 → every XSPI_WaitFlagStateUntilTimeout check
  * evaluates (0-0) > 5000 = false → infinite loop.
  *
  * @param  htim  TIM handle fired by the IRQ
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
}

/**
  * @brief  Override of BSP __weak MX_XSPI_NOR_Init — adds mandatory
  *         HAL_XSPIM_Config for XSPI2 → Port P2 (NOR Flash, GPION).
  *
  * The BSP weak version only calls HAL_XSPI_Init and returns. It never calls
  * HAL_XSPIM_Config, which means the XSPI IO Manager (XSPIM) is left in
  * whatever state the Boot ROM configured it for (memory-mapped OPI mode).
  * Without re-configuring XSPIM for indirect mode, any attempt to do an
  * indirect read/write on XSPI2 hangs forever (FT/TC flags never set).
  *
  * Parameters mirror the BSP __weak implementation exactly (same Init fields,
  * same MemoryType=MACRONIX, same ChipSelectHighTimeCycle=2). The only
  * addition is the HAL_XSPIM_Config call that assigns XSPI2 to IOPORT_2.
  */
HAL_StatusTypeDef MX_XSPI_NOR_Init(XSPI_HandleTypeDef *hxspi, MX_XSPI_InitTypeDef *Init)
{
  XSPIM_CfgTypeDef sXspiManagerCfg = {0};

  hxspi->Instance                     = XSPI2;
  hxspi->Init.FifoThresholdByte       = 1;
  hxspi->Init.MemorySize              = Init->MemorySize;
  hxspi->Init.ChipSelectHighTimeCycle = 2;
  hxspi->Init.FreeRunningClock        = HAL_XSPI_FREERUNCLK_DISABLE;
  hxspi->Init.ClockMode               = HAL_XSPI_CLOCK_MODE_0;
  hxspi->Init.DelayHoldQuarterCycle   = (Init->TransferRate == (uint32_t)BSP_XSPI_NOR_DTR_TRANSFER)
                                         ? HAL_XSPI_DHQC_ENABLE : HAL_XSPI_DHQC_DISABLE;
  hxspi->Init.ClockPrescaler          = Init->ClockPrescaler;
  hxspi->Init.SampleShifting          = Init->SampleShifting;
  hxspi->Init.ChipSelectBoundary      = HAL_XSPI_BONDARYOF_NONE;
  hxspi->Init.MemoryMode              = HAL_XSPI_SINGLE_MEM;
  hxspi->Init.WrapSize                = HAL_XSPI_WRAP_NOT_SUPPORTED;
  hxspi->Init.MemoryType              = HAL_XSPI_MEMTYPE_MACRONIX;
  hxspi->Init.MemorySelect            = HAL_XSPI_CSSEL_NCS1;
  hxspi->Init.MaxTran                 = 0;
  hxspi->Init.Refresh                 = 0;

  if (HAL_XSPI_Init(hxspi) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* Route XSPI2 to Port P2 (NOR Flash, GPION pins via XSPIM_P2).
   * HAL_XSPIM_Config clears XSPIM->CR and reconfigures it; without this the
   * XSPIM keeps the Boot ROM's memory-mapped OPI configuration which is
   * incompatible with the BSP indirect-mode reset/configure sequence. */
  sXspiManagerCfg.nCSOverride = HAL_XSPI_CSSEL_OVR_NCS1;
  sXspiManagerCfg.IOPort      = HAL_XSPIM_IOPORT_2;
  sXspiManagerCfg.Req2AckTime = 1;
  if (HAL_XSPIM_Config(hxspi, &sXspiManagerCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @brief  Override of BSP __weak MX_XSPI_RAM_Init — adds mandatory
  *         HAL_XSPIM_Config for XSPI1 → Port P1 (HyperRAM, GPIOP/GPIOO).
  *
  * Same rationale as MX_XSPI_NOR_Init above. Parameters mirror the BSP
  * weak implementation exactly (MemoryType=APMEM_16BITS, DHQC_ENABLE,
  * ChipSelectBoundary=16KB, Refresh formula). The only addition is the
  * HAL_XSPIM_Config call that assigns XSPI1 to IOPORT_1.
  */
HAL_StatusTypeDef MX_XSPI_RAM_Init(XSPI_HandleTypeDef *hxspi, MX_XSPI_InitTypeDef *Init)
{
  XSPIM_CfgTypeDef sXspiManagerCfg = {0};
  uint32_t hspi_clk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_XSPI1);

  hxspi->Instance                     = XSPI1;
  hxspi->Init.FifoThresholdByte       = 8;
  hxspi->Init.MemoryType              = HAL_XSPI_MEMTYPE_APMEM_16BITS;
  hxspi->Init.MemoryMode              = HAL_XSPI_SINGLE_MEM;
  hxspi->Init.MemorySize              = Init->MemorySize;
  hxspi->Init.MemorySelect            = HAL_XSPI_CSSEL_NCS1;
  hxspi->Init.ChipSelectHighTimeCycle = 5;
  hxspi->Init.ClockMode               = HAL_XSPI_CLOCK_MODE_0;
  hxspi->Init.ClockPrescaler          = Init->ClockPrescaler;
  hxspi->Init.SampleShifting          = Init->SampleShifting;
  hxspi->Init.DelayHoldQuarterCycle   = HAL_XSPI_DHQC_ENABLE;
  hxspi->Init.ChipSelectBoundary      = HAL_XSPI_BONDARYOF_16KB;
  hxspi->Init.FreeRunningClock        = HAL_XSPI_FREERUNCLK_DISABLE;
  hxspi->Init.Refresh                 = ((2U * (hspi_clk / hxspi->Init.ClockPrescaler)) / 1000000U) - 4U;
#if defined (OCTOSPI_DCR1_DLYBYP)
  hxspi->Init.DelayBlockBypass        = HAL_XSPI_DELAY_BLOCK_BYPASS;
#endif
  hxspi->Init.WrapSize                = HAL_XSPI_WRAP_NOT_SUPPORTED;

  if (HAL_XSPI_Init(hxspi) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* Route XSPI1 to Port P1 (HyperRAM, GPIOP/GPIOO pins via XSPIM_P1). */
  sXspiManagerCfg.nCSOverride = HAL_XSPI_CSSEL_OVR_NCS1;
  sXspiManagerCfg.IOPort      = HAL_XSPIM_IOPORT_1;
  sXspiManagerCfg.Req2AckTime = 1;
  if (HAL_XSPIM_Config(hxspi, &sXspiManagerCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
