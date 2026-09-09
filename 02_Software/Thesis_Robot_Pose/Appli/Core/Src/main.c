/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : APPLI Main program body - Complete hardware initialization
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
/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <assert.h>
#include "cmw_camera.h"
#include "stm32n6570_discovery.h"
#include "stm32n6570_discovery_bus.h"
#include "stm32n657xx.h"
#include "stm32n6xx_hal_gpio.h"
#include "app_x-cube-ai.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LCD_FB_ADDRESS        0x34400000U  /* AXISRAM3 — avoids overlap with .data/.bss at 0x34000000 */
#define LCD_WIDTH             640
#define LCD_HEIGHT            480
#define LCD_BPP               2  /* RGB565 = 2 bytes per pixel */

#define SENSOR_IMX335_WIDTH   2592
#define SENSOR_IMX335_HEIGHT  1944
#define CAMERA_FPS            30
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

UART_HandleTypeDef huart1;

PCD_HandleTypeDef hpcd_USB_OTG_HS1 __attribute__((section(".UsbHpcdSection")));
HCD_HandleTypeDef hhcd_USB_OTG_HS2;

XSPI_HandleTypeDef hxspi1;
XSPI_HandleTypeDef hxspi2;

DCMIPP_HandleTypeDef hdcmipp;
LTDC_HandleTypeDef hltdc;
RAMCFG_HandleTypeDef hramcfg_SRAM3;
RAMCFG_HandleTypeDef hramcfg_SRAM4;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemIsolation_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_GPDMA1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USB2_OTG_HS_HCD_Init(void);
static void MX_XSPI1_Init(void);
static void MX_XSPI2_Init(void);
static void MX_DCMIPP_Init(void);
static void MX_LTDC_Init(void);
static void MX_RAMCFG_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  DCMIPP Clock Configuration callback (called by CMW Camera Middleware)
  */
HAL_StatusTypeDef MX_DCMIPP_ClockConfig(DCMIPP_HandleTypeDef *hdcmipp_ptr)
{
  RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct = {0};
  HAL_StatusTypeDef ret;

  /* DCMIPP clock: IC17 = PLL1 / 4 = 800/4 = 200 MHz */
  RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_DCMIPP;
  RCC_PeriphCLKInitStruct.DcmippClockSelection = RCC_DCMIPPCLKSOURCE_IC17;
  RCC_PeriphCLKInitStruct.ICSelection[RCC_IC17].ClockSelection = RCC_ICCLKSOURCE_PLL1;
  RCC_PeriphCLKInitStruct.ICSelection[RCC_IC17].ClockDivider = 4;
  ret = HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
  if (ret) return ret;

  /* CSI PHY ref clock: IC18 = PLL1 / 60 = 800/60 ≈ 13.3 MHz */
  RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CSI;
  RCC_PeriphCLKInitStruct.ICSelection[RCC_IC18].ClockSelection = RCC_ICCLKSOURCE_PLL1;
  RCC_PeriphCLKInitStruct.ICSelection[RCC_IC18].ClockDivider = 60;
  ret = HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
  if (ret) return ret;

  return HAL_OK;
}

/**
  * @brief  DCMIPP error callback (called by CMW on pipe error)
  */
void CMW_CAMERA_PIPE_ErrorCallback(uint32_t pipe)
{
  /* Ignore pipe errors for now */
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* MPU configuration must happen BEFORE enabling the I-Cache so that
     XSPI2 (0x70000000) and AXISRAM (0x34000000) have the right memory
     attributes when the cache starts line-filling instruction fetches.
     The FSBL has already set up compatible regions for the jump; the
     call to HAL_MPU_Disable() inside MPU_Config temporarily disables
     the MPU before re-programming, which is fine because the default
     memory map is executable for 0x60000000..0x9FFFFFFF (Normal WB). */
  MPU_Config();

  /* Enable I-Cache only.  D-Cache is intentionally DISABLED because the
     camera/DMA subsystem was not designed with cache maintenance in mind.
     Enabling D-Cache introduced hard faults / data corruption on earlier
     attempts. */
  SCB_EnableICache();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* LED blink test is done after MX_GPIO_Init(), when GPIOs are configured. */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* Enable AXISRAM3/4 memory clocks early for LTDC framebuffer */
  __HAL_RCC_AXISRAM3_MEM_CLK_ENABLE();
  __HAL_RCC_AXISRAM4_MEM_CLK_ENABLE();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  /* USER CODE BEGIN LED_BLINK_TEST */
  for (uint32_t i = 0; i < 4; i++)
  {
    HAL_GPIO_TogglePin(GPIOO, GPIO_PIN_1);
    HAL_Delay(200);
  }
  /* USER CODE END LED_BLINK_TEST */

  MX_GPDMA1_Init();
  MX_USART1_UART_Init();
  MX_USB2_OTG_HS_HCD_Init();
  MX_XSPI1_Init();
  MX_XSPI2_Init();
  MX_DCMIPP_Init();
  MX_LTDC_Init();
  MX_RAMCFG_Init();
  SystemIsolation_Config();
  /* USER CODE BEGIN 2 */
  /* Enable AXISRAM3/4 for framebuffer */
  __HAL_RCC_AXISRAM3_MEM_CLK_ENABLE();
  __HAL_RCC_AXISRAM4_MEM_CLK_ENABLE();
  HAL_RAMCFG_EnableAXISRAM(&hramcfg_SRAM3);
  HAL_RAMCFG_EnableAXISRAM(&hramcfg_SRAM4);

  /* Fill framebuffer with blue for initial test */
  {
    uint16_t *fb = (uint16_t *)LCD_FB_ADDRESS;
    for (uint32_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++)
      fb[i] = 0x001F; /* Blue in RGB565 */
  }
  /* USER CODE END 2 */

  STM32CubeAI_Studio_AI_Init();

  MX_ThreadX_Init();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
/* USER CODE BEGIN CLK 1 */
/* USER CODE END CLK 1 */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  /* NOTE: The Appli executes from xSPI2 (0x70100400) in XiP mode.
   * The FSBL has already configured PLL1 at 800 MHz (HSI×25/2) with
   * CPU on IC1/2 (CPU = PLL1/2 = 400 MHz).  Re-running the full PLL1
   * config is UNSAFE because switching the CPU temporarily to HSI
   * disrupts the XSPI fetch clock.
   *
   * PLL2 (NPU) and PLL3 (AXISRAM3-6) are independent of PLL1/IC1/IC2
   * and CAN be safely enabled here.  Without them the NPU runs at
   * ~400 MHz instead of 1000 MHz and AXISRAM at ~400 instead of
   * 900 MHz — a massive performance penalty.
   */
  RCC_OscInitTypeDef osc = {0};
  RCC_ClkInitTypeDef clk = {0};

  /* Keep PLL1 untouched — only add PLL2 + PLL3 */
  osc.OscillatorType = RCC_OSCILLATORTYPE_NONE;   /* HSE already on */
  osc.PLL1.PLLState  = RCC_PLL_NONE;              /* don't touch PLL1 */

  /* PLL2 = HSI(64) × 125 / 8 = 1000 MHz  →  NPU clock via IC6 */
  osc.PLL2.PLLState    = RCC_PLL_ON;
  osc.PLL2.PLLSource   = RCC_PLLSOURCE_HSI;
  osc.PLL2.PLLM        = 8;
  osc.PLL2.PLLN        = 125;
  osc.PLL2.PLLFractional = 0;
  osc.PLL2.PLLP1       = 1;
  osc.PLL2.PLLP2       = 1;

  /* PLL3 = HSI(64) × 225 / 8 / 2 = 900 MHz  →  AXISRAM3-6 via IC11 */
  osc.PLL3.PLLState    = RCC_PLL_ON;
  osc.PLL3.PLLSource   = RCC_PLLSOURCE_HSI;
  osc.PLL3.PLLM        = 8;
  osc.PLL3.PLLN        = 225;
  osc.PLL3.PLLFractional = 0;
  osc.PLL3.PLLP1       = 1;
  osc.PLL3.PLLP2       = 2;

  osc.PLL4.PLLState = RCC_PLL_NONE;

  if (HAL_RCC_OscConfig(&osc) != HAL_OK)
  {
    Error_Handler();
  }

  /* Switch IC6 (NPU) to PLL2 and IC11 (AXISRAM) to PLL3.
   * IC1 (CPU) and IC2 (AXI/XSPI) must be RE-STATED with their current
   * FSBL values, otherwise HAL_RCC_ClockConfig will reprogram them with
   * divider=0 (uninitialised struct) and kill the XiP fetch clock. */
  clk.ClockType      = RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_SYSCLK;
  clk.CPUCLKSource   = RCC_CPUCLKSOURCE_IC1;
  clk.SYSCLKSource   = RCC_SYSCLKSOURCE_IC2_IC6_IC11;

  /* Keep IC1/IC2 on PLL1 with the FSBL's dividers — identical write, safe */
  clk.IC1Selection.ClockSelection  = RCC_ICCLKSOURCE_PLL1;
  clk.IC1Selection.ClockDivider    = 2;     /* PLL1 / 2 = CPU clock */
  clk.IC2Selection.ClockSelection  = RCC_ICCLKSOURCE_PLL1;
  clk.IC2Selection.ClockDivider    = 3;     /* PLL1 / 3 = SYSCLK/AXI */

  /* Only IC6 and IC11 really change source */
  clk.IC6Selection.ClockSelection  = RCC_ICCLKSOURCE_PLL2;
  clk.IC6Selection.ClockDivider    = 1;     /* 1000 MHz for NPU */
  clk.IC11Selection.ClockSelection = RCC_ICCLKSOURCE_PLL3;
  clk.IC11Selection.ClockDivider   = 1;     /* 900 MHz for AXISRAM3-6 */

  if (HAL_RCC_ClockConfig(&clk) != HAL_OK)
  {
    Error_Handler();
  }

  SystemCoreClockUpdate();
}


/**
  * @brief GPDMA1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPDMA1_Init(void)
{

  /* USER CODE BEGIN GPDMA1_Init 0 */

  /* USER CODE END GPDMA1_Init 0 */

  /* Peripheral clock enable */
  __HAL_RCC_GPDMA1_CLK_ENABLE();

  /* GPDMA1 interrupt Init */
    HAL_NVIC_SetPriority(GPDMA1_Channel0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel0_IRQn);
    HAL_NVIC_SetPriority(GPDMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel1_IRQn);

  /* USER CODE BEGIN GPDMA1_Init 1 */

  /* USER CODE END GPDMA1_Init 1 */
  /* USER CODE BEGIN GPDMA1_Init 2 */

  /* USER CODE END GPDMA1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USB1_OTG_HS Initialization Function
  * @param None
  * @retval None
  */
void MX_USB1_OTG_HS_PCD_Init(void)
{

  /* USER CODE BEGIN USB1_OTG_HS_Init 0 */
  /* PCD handle is in non-cacheable (NOLOAD) section — not zeroed by startup
     code.  Memset it before use. */
  memset(&hpcd_USB_OTG_HS1, 0, sizeof(hpcd_USB_OTG_HS1));
  /* USER CODE END USB1_OTG_HS_Init 0 */

  /* USER CODE BEGIN USB1_OTG_HS_Init 1 */

  /* USER CODE END USB1_OTG_HS_Init 1 */
  hpcd_USB_OTG_HS1.Instance = USB1_OTG_HS;
  hpcd_USB_OTG_HS1.Init.dev_endpoints = 9;
  hpcd_USB_OTG_HS1.Init.speed = PCD_SPEED_HIGH;
  hpcd_USB_OTG_HS1.Init.phy_itface = USB_OTG_HS_EMBEDDED_PHY;
  hpcd_USB_OTG_HS1.Init.Sof_enable = DISABLE;
  hpcd_USB_OTG_HS1.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_HS1.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_HS1.Init.use_dedicated_ep1 = DISABLE;
  hpcd_USB_OTG_HS1.Init.vbus_sensing_enable = DISABLE;
  hpcd_USB_OTG_HS1.Init.dma_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_HS1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB1_OTG_HS_Init 2 */

  /* USER CODE END USB1_OTG_HS_Init 2 */

}

/**
  * @brief USB2_OTG_HS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB2_OTG_HS_HCD_Init(void)
{
  /* USB2 HCD intentionally left uninitialised — not used in this build. */
}

/**
  * @brief XSPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_XSPI1_Init(void)
{
  /* FSBL already configured XSPI1 (HyperRAM) in memory-mapped mode at 0x90000000. */
}

/**
  * @brief XSPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_XSPI2_Init(void)
{
  /* FSBL already configured XSPI2 (NOR Flash) in memory-mapped XiP mode at 0x70000000. */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOP_CLK_ENABLE();
  __HAL_RCC_GPIOO_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPION_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pins : LCD_B4_Pin LCD_B5_Pin LCD_R4_Pin */
  GPIO_InitStruct.Pin = LCD_B4_Pin|LCD_B5_Pin|LCD_R4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LCD;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_R2_Pin LCD_R7_Pin LCD_R1_Pin */
  GPIO_InitStruct.Pin = LCD_R2_Pin|LCD_R7_Pin|LCD_R1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LCD;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_HSYNC_Pin LCD_B2_Pin LCD_G4_Pin LCD_G6_Pin
                           LCD_G5_Pin LCD_R3_Pin */
  GPIO_InitStruct.Pin = LCD_HSYNC_Pin|LCD_B2_Pin|LCD_G4_Pin|LCD_G6_Pin
                          |LCD_G5_Pin|LCD_R3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LCD;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_VSYNC_Pin */
  GPIO_InitStruct.Pin = LCD_VSYNC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LCD;
  HAL_GPIO_Init(LCD_VSYNC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : User_Pin */
  GPIO_InitStruct.Pin = User_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(User_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_B3_Pin LCD_B0_Pin LCD_G1_Pin LCD_R0_Pin
                           LCD_G0_Pin LCd_G7_Pin LCD_DE_Pin LCD_R6_Pin */
  GPIO_InitStruct.Pin = LCD_B3_Pin|LCD_B0_Pin|LCD_G1_Pin|LCD_R0_Pin
                          |LCD_G0_Pin|LCd_G7_Pin|LCD_DE_Pin|LCD_R6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LCD;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_G2_Pin LCD_R5_Pin LCD_B1_Pin LCD_B7_Pin
                           LCD_B6_Pin LCD_G3_Pin */
  GPIO_InitStruct.Pin = LCD_G2_Pin|LCD_R5_Pin|LCD_B1_Pin|LCD_B7_Pin
                          |LCD_B6_Pin|LCD_G3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LCD;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure the EXTI line attribute */
  HAL_EXTI_ConfigLineAttributes(EXTI_LINE_13, EXTI_LINE_SEC);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* LED1 Green on STM32N6570-DK: GPIOO PIN1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOO, &GPIO_InitStruct);

  /* LED1 is high-active on DK, start with OFF state */
  HAL_GPIO_WritePin(GPIOO, GPIO_PIN_1, GPIO_PIN_RESET);

  /* USER CODE END MX_GPIO_Init_2 */
}

/**
  * @brief RIF Initialization Function
  * @param None
  * @retval None
  */
  static void SystemIsolation_Config(void)
{

/* USER CODE BEGIN RIF_Init 0 */

/* USER CODE END RIF_Init 0 */

  /* set all required IPs as secure privileged */
  __HAL_RCC_RIFSC_CLK_ENABLE();

  /* RIMC configuration for DCMIPP and LTDC */
  RIMC_MasterConfig_t RIMC_master = {0};
  RIMC_master.MasterCID = RIF_CID_1;
  RIMC_master.SecPriv = RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV;
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_DCMIPP, &RIMC_master);
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_LTDC1, &RIMC_master);

  /* RISUP for DCMIPP and LTDC */
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_DCMIPP, RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_LTDCL1, RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);

  /* RIF-Aware IPs Config */

  /* set up GPDMA configuration */
  /* set GPDMA1 channel 0 used by UCPD1 */
  LL_DMA_EnableChannelSecure(GPDMA1, LL_DMA_CHANNEL_0);
  LL_DMA_EnableChannelPrivilege(GPDMA1, LL_DMA_CHANNEL_0);
  LL_DMA_EnableChannelSrcSecure(GPDMA1, LL_DMA_CHANNEL_0);
  LL_DMA_EnableChannelDestSecure(GPDMA1, LL_DMA_CHANNEL_0);
  /* set GPDMA1 channel 1 used by UCPD1 */
  LL_DMA_EnableChannelSecure(GPDMA1, LL_DMA_CHANNEL_1);
  LL_DMA_EnableChannelPrivilege(GPDMA1, LL_DMA_CHANNEL_1);
  LL_DMA_EnableChannelSrcSecure(GPDMA1, LL_DMA_CHANNEL_1);
  LL_DMA_EnableChannelDestSecure(GPDMA1, LL_DMA_CHANNEL_1);

/* USER CODE BEGIN RIF_Init 1 */

/* USER CODE END RIF_Init 1 */
/* USER CODE BEGIN RIF_Init 2 */

/* USER CODE END RIF_Init 2 */

}

/* USER CODE BEGIN 4 */

/**
  * @brief DCMIPP Initialization Function (CMW handles actual init)
  */
static void MX_DCMIPP_Init(void)
{
  return; /* DCMIPP is initialized by CMW Camera Middleware */
}

/**
  * @brief LTDC Initialization Function with correct RK050HR18 timings
  */
static void MX_LTDC_Init(void)
{
  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 4;
  hltdc.Init.VerticalSync = 4;
  hltdc.Init.AccumulatedHBP = 12;
  hltdc.Init.AccumulatedVBP = 12;
  hltdc.Init.AccumulatedActiveW = 812;
  hltdc.Init.AccumulatedActiveH = 492;
  hltdc.Init.TotalWidth = 820;
  hltdc.Init.TotalHeigh = 500;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }

  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = LCD_WIDTH;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = LCD_HEIGHT;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = LCD_FB_ADDRESS;
  pLayerCfg.ImageWidth = LCD_WIDTH;
  pLayerCfg.ImageHeight = LCD_HEIGHT;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /* RIMC for LTDC */
  RIMC_MasterConfig_t RIMC_master = {0};
  RIMC_master.MasterCID = RIF_CID_1;
  RIMC_master.SecPriv = RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV;
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_LTDC1, &RIMC_master);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_LTDCL1, RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
}

/**
  * @brief RAMCFG Initialization Function for SRAM3/SRAM4
  */
static void MX_RAMCFG_Init(void)
{
  hramcfg_SRAM3.Instance = RAMCFG_SRAM3_AXI;
  if (HAL_RAMCFG_Init(&hramcfg_SRAM3) != HAL_OK)
  {
    Error_Handler();
  }

  hramcfg_SRAM4.Instance = RAMCFG_SRAM4_AXI;
  if (HAL_RAMCFG_Init(&hramcfg_SRAM4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/* ---------------------------------------------------------------------------
   MPU_Config  –  Configure MPU regions BEFORE enabling the MPU:
     Region 0 : XSPI2 NOR Flash   (0x70000000..0x7FFFFFFF) Normal WB, executable
                – mandatory for XiP instruction fetch after FSBL hand-over.
     Region 1 : XSPI1 HyperRAM    (0x90000000..0x9FFFFFFF) Normal WB, XN
     Region 2 : AXISRAM           (0x34000000..0x345FFFFF) Normal WB
                – covers .data/.bss (0x34000000..0x34200000) and the LCD
                  framebuffer in AXISRAM3/4 at 0x34400000.
     Region 3 : .noncacheable     (linker symbols)         Non-cacheable, XN
                – USB PCD handle + USBX pools: D-Cache coherency safety.
   ------------------------------------------------------------------------ */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef     MPU_InitStruct = {0};
  MPU_Attributes_InitTypeDef MPU_AttrInit   = {0};

  HAL_MPU_Disable();

  /* Attribute 0: Normal Write-Back Read/Write-Allocate (cached) */
  MPU_AttrInit.Number     = MPU_ATTRIBUTES_NUMBER0;
  MPU_AttrInit.Attributes = INNER_OUTER(MPU_NON_TRANSIENT | MPU_WRITE_BACK | MPU_RW_ALLOCATE);
  HAL_MPU_ConfigMemoryAttributes(&MPU_AttrInit);

  /* Attribute 1: Non-cacheable */
  MPU_AttrInit.Number     = MPU_ATTRIBUTES_NUMBER1;
  MPU_AttrInit.Attributes = INNER_OUTER(MPU_NOT_CACHEABLE);
  HAL_MPU_ConfigMemoryAttributes(&MPU_AttrInit);

  /* Region 0: XSPI2 NOR Flash — executable, cached (XiP) */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress      = 0x70000000UL;
  MPU_InitStruct.LimitAddress     = 0x7FFFFFFFUL;
  MPU_InitStruct.AttributesIndex  = MPU_ATTRIBUTES_NUMBER0;
  MPU_InitStruct.AccessPermission = MPU_REGION_ALL_RW;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Region 1: XSPI1 HyperRAM — data only, cached */
  MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress      = 0x90000000UL;
  MPU_InitStruct.LimitAddress     = 0x9FFFFFFFUL;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Region 2: AXISRAM (covers .data/.bss and LCD framebuffer) — cached */
  MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress      = 0x34000000UL;
  MPU_InitStruct.LimitAddress     = 0x345FFFFFUL;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Region 3: .noncacheable section (USB PCD handle + USBX pools) */
  extern uint32_t __snoncacheable;
  extern uint32_t __enoncacheable;
  uint32_t nc_base  = (uint32_t)&__snoncacheable & ~0x1FU;
  uint32_t nc_limit = ((uint32_t)&__enoncacheable + 0x1FU) & ~0x1FU;
  if (nc_limit <= nc_base) nc_limit = nc_base + 32U;

  MPU_InitStruct.Number           = MPU_REGION_NUMBER3;
  MPU_InitStruct.BaseAddress      = nc_base;
  MPU_InitStruct.LimitAddress     = nc_limit - 1U;
  MPU_InitStruct.AttributesIndex  = MPU_ATTRIBUTES_NUMBER1;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  HAL_MPU_Enable(MPU_HFNMI_PRIVDEF);
}
