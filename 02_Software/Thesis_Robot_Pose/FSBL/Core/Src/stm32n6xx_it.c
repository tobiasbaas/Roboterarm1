/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32n6xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "main.h"
#include "stm32n6xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim6;

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
/* direct UART1 debug for fault handlers */
#define _FLT_ISR (*(volatile unsigned long *)(0x52001000UL + 0x1CUL))
#define _FLT_TDR (*(volatile unsigned long *)(0x52001000UL + 0x28UL))
static void _flt_msg(const char *s){while(*s){while(!(_FLT_ISR&(1UL<<7))){}; _FLT_TDR=(unsigned char)*s++;}}

void HardFault_Handler(void)
{
  _flt_msg("[HARDFAULT]\r\n");
  while (1) {}
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  _flt_msg("[MEMMANAGE]\r\n");
  while (1) {}
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  _flt_msg("[BUSFAULT]\r\n");
  while (1) {}
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  _flt_msg("[USAGEFAULT]\r\n");
  while (1) {}
}

/**
  * @brief This function handles Secure fault.
  */
void SecureFault_Handler(void)
{
  /* USER CODE BEGIN SecureFault_IRQn 0 */

  /* USER CODE END SecureFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_SecureFault_IRQn 0 */
    /* USER CODE END W1_SecureFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32N6xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32n6xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles GPDMA1 Channel 0 global interrupt.
  */
void GPDMA1_Channel0_IRQHandler(void)
{
  /* USER CODE BEGIN GPDMA1_Channel0_IRQn 0 */

  /* USER CODE END GPDMA1_Channel0_IRQn 0 */
  /* USER CODE BEGIN GPDMA1_Channel0_IRQn 1 */

  /* USER CODE END GPDMA1_Channel0_IRQn 1 */
}

/**
  * @brief This function handles GPDMA1 Channel 1 global interrupt.
  */
void GPDMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN GPDMA1_Channel1_IRQn 0 */

  /* USER CODE END GPDMA1_Channel1_IRQn 0 */
  /* USER CODE BEGIN GPDMA1_Channel1_IRQn 1 */

  /* USER CODE END GPDMA1_Channel1_IRQn 1 */
}

/**
  * @brief This function handles TIM6 global interrupt.
  */
void TIM6_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_IRQn 0 */

  /* USER CODE END TIM6_IRQn 0 */
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_IRQn 1 */

  /* USER CODE END TIM6_IRQn 1 */
}

/**
  * @brief This function handles XSPI1 global interrupt.
  */
void XSPI1_IRQHandler(void)
{
  /* FSBL bootloader - no XSPI operations needed */
}

/**
  * @brief This function handles UCPD1 global interrupt.
  */
void UCPD1_IRQHandler(void)
{
  /* FSBL bootloader - no UCPD operations needed */
}

/**
  * @brief This function handles USB1 OTG HS interrupt.
  */
void USB1_OTG_HS_IRQHandler(void)
{
  /* FSBL bootloader - no USB operations needed */
}

/* USER CODE BEGIN 1 */

/**
  * @brief This function handles DCMIPP global interrupt.
  */
void DCMIPP_IRQHandler(void)
{
  /* FSBL bootloader - no camera operations needed */
}

/**
  * @brief This function handles CSI global interrupt.
  */
void CSI_IRQHandler(void)
{
  /* FSBL bootloader - no camera operations needed */
}

/**
  * @brief This function handles LTDC global interrupt.
  */
void LTDC_UP_IRQHandler(void)
{
  /* FSBL bootloader - nno display operations needed */
}

/**
  * @brief This function handles LTDC error interrupt.
  */
void LTDC_UP_ERR_IRQHandler(void)
{
  /* FSBL bootloader - no display operations needed */
}

/* USER CODE END 1 */
