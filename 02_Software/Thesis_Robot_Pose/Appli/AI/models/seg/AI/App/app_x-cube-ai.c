
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
#include "stai.h"
#include "npu_init.h"



/* USER CODE BEGIN includes */
/* USER CODE END includes */

/* IO buffers ----------------------------------------------------------------*/


/* Input defs ----------------------------------------------------------------*/

/**

// Array to store the data of the input tensor
stai_ptr data_ins[] = {
}; 
*/

/* Output defs ----------------------------------------------------------------*/

/**

// c-array to store the data of the output tensor
stai_ptr data_outs[] = {
}; 
*/




/* Activations buffers -------------------------------------------------------*/


/* Entry points --------------------------------------------------------------*/
LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(network)
uint8_t *buffer_in;
uint8_t *buffer_out;

/* 
 * Bootstrap
 */
int aiInit(void) {
    /* Retreive the start address of the input and output 
     * buffers (reserved in the activation buffer) 
     */
    const LL_Buffer_InfoTypeDef * inputBuffersInfos = LL_ATON_Input_Buffers_Info(&NN_Instance_network);
    const LL_Buffer_InfoTypeDef * outputBuffersInfos = LL_ATON_Output_Buffers_Info(&NN_Instance_network);
    buffer_in = (uint8_t *)LL_Buffer_addr_start(&inputBuffersInfos[0]);
    buffer_out = (uint8_t *)LL_Buffer_addr_start(&outputBuffersInfos[0]);
    LL_ATON_RT_RuntimeInit();
    LL_ATON_RT_Init_Network(&NN_Instance_network);  
  return 0;
}

int aiDeinit(void) {
  /* Deinitialize Neural-ART network instance */
  LL_ATON_RT_DeInit_Network(&NN_Instance_network);
  LL_ATON_RT_RuntimeDeInit();
  return 0;
}

/* 
 * Run inference
 */
int aiRun() {

    LL_ATON_RT_RetValues_t ll_aton_rt_ret = LL_ATON_RT_DONE;
    LL_ATON_RT_Reset_Network(&NN_Instance_network);
    
    do {
      /* Execute first/next step */
      ll_aton_rt_ret = LL_ATON_RT_RunEpochBlock(&NN_Instance_network);
      /* Wait for next event */
      if (ll_aton_rt_ret == LL_ATON_RT_WFE)
        LL_ATON_OSAL_WFE();
    } while (ll_aton_rt_ret != LL_ATON_RT_DONE);
  return 0;
}


int acquire_and_process_data()
{
  /* fill the inputs of the c-model 
  for (int idx=0; idx < STAI_NETWORK_IN_NUM; idx++ )
  {
      stai_input[idx] = ....
  }

  */
  return 0;
}

int post_process()
{
  /* process the predictions
  for (int idx=0; idx < STAI_NETWORK_OUT_NUM; idx++ )
  {
      stai_output[idx] = ....
  }

  */
  return 0;
}



/* 
 * Example of main loop function
 */
void main_loop() {
  while (1) {
    /* 1 - Acquire, pre-process and fill the input buffers */
    acquire_and_process_data();

    /* 2 - Call inference engine */
    aiRun();

    /* 3 - Post-process the predictions */
    post_process();
  }
}


/* Entry points --------------------------------------------------------------*/



void STM32CubeAI_Studio_AI_Init(void)
{
    aiPreInitialize();
    /* USER CODE BEGIN 5 */
    aiInit();
    /* USER CODE END 5 */
}

void STM32CubeAI_Studio_AI_Process(void)
{
    main_loop();
} 

void STM32CubeAI_Studio_AI_Deinit(void)
{
    aiDeinit();
} 


#ifdef __cplusplus
}
#endif
