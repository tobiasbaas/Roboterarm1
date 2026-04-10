/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_AI_H
#define __APP_AI_H
#ifdef __cplusplus
extern "C" {
#endif
/**
  ******************************************************************************
  * @file    app_x-cube-ai.h
  * @author  X-CUBE-AI C code generator
  * @brief   AI entry function definitions
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
/* Includes ------------------------------------------------------------------*/
#include "stai.h"
#include "ai_datatypes_defines.h"
#include "ll_aton_runtime.h"
#include "network.h"

#define MIN_HEAP_SIZE 0x800
#define MIN_STACK_SIZE 0x2000

/* Input defs ----------------------------------------------------------------*/


#define STAI_MNETWORK_IN_NUM 0

#define DEF_DATA_IN \
stai_ptr data_ins[] = { \
}; 


/* Output defs ----------------------------------------------------------------*/

#define STAI_MNETWORK_OUT_NUM 0

#define DEF_DATA_OUT \
stai_ptr data_outs[] = { \
}; 

/* IO buffers ----------------------------------------------------------------*/

extern stai_ptr data_ins[];
extern stai_ptr data_outs[];

extern stai_ptr data_activations[];
extern stai_ptr data_states[];
void STM32CubeAI_Studio_AI_Init(void);
void STM32CubeAI_Studio_AI_Process(void);
/* USER CODE BEGIN includes */
/* USER CODE END includes */


#ifdef __cplusplus
}
#endif
#endif /*__STMicroelectronics_ST_EDGE_AI_3.0.0-20426 123672867_H */
