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
#include <stdint.h>
#include <stddef.h>
#include "stai.h"
#include "ai_datatypes_defines.h"
#include "ll_aton_runtime.h"

typedef enum
{
  APP_AI_MODEL_POSE = 0,
  APP_AI_MODEL_SEGMENTATION = 1
} AppAIModel_t;

/* IO buffers ----------------------------------------------------------------*/

void STM32CubeAI_Studio_AI_Init(void);
void STM32CubeAI_Studio_AI_Process(void);
int aiRun(void);

void App_AI_SetModel(AppAIModel_t model);
AppAIModel_t App_AI_GetModel(void);

uint8_t *App_AI_GetInputBuffer(void);
const uint8_t *App_AI_GetOutputBuffer(uint32_t output_index);

size_t App_AI_GetInputSize(void);
size_t App_AI_GetOutputSize(uint32_t output_index);
uint32_t App_AI_GetOutputCount(void);

uint32_t App_AI_GetRunCounter(void);
int32_t App_AI_GetLastRunStatus(void);

uint16_t *App_AI_GetDisplayFramebuffer(void);
uint32_t App_AI_GetDisplayWidth(void);
uint32_t App_AI_GetDisplayHeight(void);

void App_AI_RenderActiveModelOverlay(void);

extern uint8_t *buffer_in;
extern uint8_t *buffer_out;
/* USER CODE BEGIN includes */
/* USER CODE END includes */


#ifdef __cplusplus
}
#endif
#endif /*__STMicroelectronics_ST_EDGE_AI_3.0.0-20426 123672867_H */
