/**
  ******************************************************************************
  * @file    network.h
  * @author  STEdgeAI
  * @date    2026-03-24 20:45:55
  * @brief   Minimal description of the generated c-implemention of the network
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */
#ifndef LL_ATON_NETWORK_H
#define LL_ATON_NETWORK_H

/******************************************************************************/
#define LL_ATON_NETWORK_C_MODEL_NAME        "network"
#define LL_ATON_NETWORK_ORIGIN_MODEL_NAME   "roboterarm_real_opt_v004_20260324_int8"

/************************** USER ALLOCATED IOs ********************************/
// No user allocated inputs
// No user allocated outputs

/************************** INPUTS ********************************************/
#define LL_ATON_NETWORK_IN_NUM        (1)    // Total number of input buffers
// Input buffer 1 -- Input_4_out_0
#define LL_ATON_NETWORK_IN_1_ALIGNMENT   (32)
#define LL_ATON_NETWORK_IN_1_SIZE_BYTES  (1228800)

/************************** OUTPUTS *******************************************/
#define LL_ATON_NETWORK_OUT_NUM        (2)    // Total number of output buffers
// Output buffer 1 -- Quantize_820_out_0
#define LL_ATON_NETWORK_OUT_1_ALIGNMENT   (32)
#define LL_ATON_NETWORK_OUT_1_SIZE_BYTES  (344400)
// Output buffer 2 -- Quantize_807_out_0
#define LL_ATON_NETWORK_OUT_2_ALIGNMENT   (32)
#define LL_ATON_NETWORK_OUT_2_SIZE_BYTES  (819200)

#endif /* LL_ATON_NETWORK_H */
