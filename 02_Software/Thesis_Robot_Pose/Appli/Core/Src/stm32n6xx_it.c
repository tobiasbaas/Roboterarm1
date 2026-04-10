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
volatile uint32_t g_appli_fault_signature = 0U;
volatile uint32_t g_appli_fault_hfsr = 0U;
volatile uint32_t g_appli_fault_cfsr = 0U;
volatile uint32_t g_appli_fault_bfar = 0U;
volatile uint32_t g_appli_fault_mmfar = 0U;
volatile uint32_t g_appli_fault_shcsr = 0U;
volatile uint32_t g_appli_fault_vtor = 0U;
volatile uint32_t g_appli_fault_msp = 0U;
volatile uint32_t g_appli_fault_ipsr = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void Appli_FaultCapture(uint32_t signature)
{
  g_appli_fault_signature = signature;
  g_appli_fault_hfsr = SCB->HFSR;
  g_appli_fault_cfsr = SCB->CFSR;
  g_appli_fault_bfar = SCB->BFAR;
  g_appli_fault_mmfar = SCB->MMFAR;
  g_appli_fault_shcsr = SCB->SHCSR;
  g_appli_fault_vtor = SCB->VTOR;
  g_appli_fault_msp = __get_MSP();
  g_appli_fault_ipsr = __get_IPSR();
}

static void Appli_FaultBlinkLoop(void)
{
  for (;;)
  {
    HAL_GPIO_TogglePin(GPIOO, GPIO_PIN_1);
    for (volatile uint32_t i = 0; i < 800000U; ++i) { }
  }
}

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

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
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
  Appli_FaultCapture(0x48445201U);

  /* USER CODE END HardFault_IRQn 0 */
  Appli_FaultBlinkLoop();
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */
  Appli_FaultCapture(0x4D454D01U);

  /* USER CODE END MemoryManagement_IRQn 0 */
  Appli_FaultBlinkLoop();
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */
  Appli_FaultCapture(0x42555301U);

  /* USER CODE END BusFault_IRQn 0 */
  Appli_FaultBlinkLoop();
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */
  Appli_FaultCapture(0x55534101U);

  /* USER CODE END UsageFault_IRQn 0 */
  Appli_FaultBlinkLoop();
}

/**
  * @brief This function handles Secure fault.
  */
void SecureFault_Handler(void)
{
  /* USER CODE BEGIN SecureFault_IRQn 0 */
  Appli_FaultCapture(0x53454301U);

  /* USER CODE END SecureFault_IRQn 0 */
  Appli_FaultBlinkLoop();
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
#ifndef USE_THREADX_AI_RUNTIME
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}
#endif

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

/**
  * @brief This function handles Pendable request for system service.
  */
#ifndef USE_THREADX_AI_RUNTIME
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}
#endif

/**
  * @brief This function handles System tick timer.
  */
#ifndef USE_THREADX_AI_RUNTIME
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}
#endif

/******************************************************************************/
/* STM32N6xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32n6xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
