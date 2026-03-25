/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.c
  * @brief   Minimal ThreadX runtime for AI task scheduling in Appli context.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "app_threadx.h"
#include "app_x-cube-ai.h"
#include <stddef.h>

#include "network.h"

#define AI_THREAD_STACK_SIZE  (4096U)
#define AI_THREAD_PRIORITY    (10U)

static TX_THREAD ai_thread;
static ULONG ai_thread_stack[AI_THREAD_STACK_SIZE / sizeof(ULONG)];

__weak int App_AI_PrepareInput(uint8_t *input_buffer, size_t input_size)
{
  (void)input_buffer;
  (void)input_size;
  return 0;
}

__weak void App_AI_OnResult(const uint8_t *output_buffer, size_t output_size)
{
  (void)output_buffer;
  (void)output_size;
}

static VOID ai_thread_entry(ULONG thread_input)
{
  (void)thread_input;

  for (;;)
  {
    if (App_AI_PrepareInput(buffer_in, LL_ATON_NETWORK_IN_1_SIZE_BYTES) == 0)
    {
      aiRun();
      App_AI_OnResult(buffer_out, LL_ATON_NETWORK_OUT_1_SIZE_BYTES);
    }
    tx_thread_sleep(1);
  }
}

UINT App_ThreadX_Init(VOID *memory_ptr)
{
  (void)memory_ptr;

  return tx_thread_create(&ai_thread,
                          "AI Thread",
                          ai_thread_entry,
                          0,
                          ai_thread_stack,
                          sizeof(ai_thread_stack),
                          AI_THREAD_PRIORITY,
                          AI_THREAD_PRIORITY,
                          TX_NO_TIME_SLICE,
                          TX_AUTO_START);
}

VOID tx_application_define(VOID *first_unused_memory)
{
  (void)first_unused_memory;
  (void)App_ThreadX_Init(TX_NULL);
}

void MX_ThreadX_Init(void)
{
  tx_kernel_enter();
}

void valueNotSetted(ULONG thread_input)
{
  (void)thread_input;
}