
/**
  ******************************************************************************
  * @file    app_x-cube-ai.c
  * @author  X-CUBE-AI C code generator
  * @brief   AI program body
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

  /**
    * Description
    * Minimum template to show how to use the Neural-ART Embedded Client API
    *          Re-target of the printf function is out-of-scope.
    *
    *
    */

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/


/* System headers */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "app_x-cube-ai.h"
#include "bsp_ai.h"
#include "aiValidation.h"
#include "stai.h"
#include "npu_init.h"



/* USER CODE BEGIN includes */
/* USER CODE END includes */

/* IO buffers ----------------------------------------------------------------*/





/* Activations buffers -------------------------------------------------------*/



/* Entry points --------------------------------------------------------------*/



void STM32CubeAI_Studio_AI_Init(void)
{
    MX_UARTx_Init();
    aiPreInitialize();
    aiValidationInit();
    /* USER CODE BEGIN 5 */
    /* USER CODE END 5 */
}

void STM32CubeAI_Studio_AI_Process(void)
{
    aiValidationProcess();
} 

void STM32CubeAI_Studio_AI_Deinit(void)
{
} 


#ifdef __cplusplus
}
#endif
