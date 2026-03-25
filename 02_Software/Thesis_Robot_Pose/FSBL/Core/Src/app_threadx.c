/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.c
  * @author  MCD Application Team
  * @brief   ThreadX applicative file
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "cmw_camera.h"
#include "stm32n6570_discovery.h"
#include "stm32n6570_discovery_bus.h"
#include "ux_device_cdc_acm.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAMERA_THREAD_STACK_SIZE  8192
#define CAMERA_THREAD_PRIORITY    15
#define CMD_THREAD_STACK_SIZE     4096
#define CMD_THREAD_PRIORITY       12

#define LCD_FB_ADDRESS        0x34200000U
#define LCD_WIDTH             640
#define LCD_HEIGHT            480
#define LCD_BPP               2

#define SENSOR_IMX335_WIDTH   2592
#define SENSOR_IMX335_HEIGHT  1944
#define CAMERA_FPS            30

#define ISP_WARMUP_TICKS      200

/* USB CDC */
#define CDC_RX_BUF_SIZE       64
#define CDC_TX_BUF_SIZE       256
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static TX_THREAD camera_thread;
static UCHAR camera_thread_stack[CAMERA_THREAD_STACK_SIZE];

static TX_THREAD cmd_thread;
static UCHAR cmd_thread_stack[CMD_THREAD_STACK_SIZE];

/* Placeholder counter for AI activity in the FSBL camera loop. */
static volatile int ai_inference_count = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void camera_thread_entry(ULONG thread_input);
static void cmd_thread_entry(ULONG thread_input);
/* USER CODE END PFP */

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;

  /* USER CODE BEGIN App_ThreadX_MEM_POOL */

  /* USER CODE END App_ThreadX_MEM_POOL */

  /* USER CODE BEGIN App_ThreadX_Init */

  /* Create camera thread with static stack */
  ret = tx_thread_create(&camera_thread,
                         "Camera Thread",
                         camera_thread_entry,
                         0,
                         camera_thread_stack,
                         CAMERA_THREAD_STACK_SIZE,
                         CAMERA_THREAD_PRIORITY,
                         CAMERA_THREAD_PRIORITY,
                         TX_NO_TIME_SLICE,
                         TX_AUTO_START);
  if (ret != TX_SUCCESS)
  {
    return ret;
  }

  /* Create command handler thread for USB CDC command handling */
  ret = tx_thread_create(&cmd_thread,
                         "CMD Thread",
                         cmd_thread_entry,
                         0,
                         cmd_thread_stack,
                         CMD_THREAD_STACK_SIZE,
                         CMD_THREAD_PRIORITY,
                         CMD_THREAD_PRIORITY,
                         TX_NO_TIME_SLICE,
                         TX_AUTO_START);
  if (ret != TX_SUCCESS)
  {
    return ret;
  }

  /* USER CODE END App_ThreadX_Init */

  return ret;
}

/**
  * @brief  Function that implements the kernel's initialization.
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)
{
  /* USER CODE BEGIN Before_Kernel_Start */

  /* USER CODE END Before_Kernel_Start */

  tx_kernel_enter();

  /* USER CODE BEGIN Kernel_Start_Error */

  /* USER CODE END Kernel_Start_Error */
}

/* USER CODE BEGIN 1 */

/* ------------------------------------------------------------------ */
/*  UART debug helper — sends to USART1 (ST-LINK VCP, separate COM)  */
/* ------------------------------------------------------------------ */
extern UART_HandleTypeDef huart1;

static void dbg(const char *msg)
{
  HAL_UART_Transmit(&huart1, (const uint8_t *)msg,
                    (uint16_t)strlen(msg), 50);
}

/* ------------------------------------------------------------------ */
/*  Camera thread — continuous mode with AI placeholder               */
/* ------------------------------------------------------------------ */
static void camera_thread_entry(ULONG thread_input)
{
  CMW_CameraInit_t cam_conf;
  CMW_Sensor_Name_t sensor;
  CMW_DCMIPP_Conf_t dcmipp_conf;
  uint32_t hw_pitch;
  int ret;
  int s_width = SENSOR_IMX335_WIDTH;
  int s_height = SENSOR_IMX335_HEIGHT;

  (void)thread_input;

  ret = CMW_CAMERA_GetSensorName(&sensor);
  assert(ret == CMW_ERROR_NONE);

  cam_conf.width  = s_width;
  cam_conf.height = s_height;
  cam_conf.fps    = CAMERA_FPS;
  cam_conf.mirror_flip = CMW_MIRRORFLIP_MIRROR;
  ret = CMW_CAMERA_Init(&cam_conf, NULL);
  assert(ret == CMW_ERROR_NONE);
  s_width  = cam_conf.width;
  s_height = cam_conf.height;

  {
    DCMIPP_IPPlugConfTypeDef ipplug_conf = { 0 };
    DCMIPP_HandleTypeDef *hdcmipp_ptr = CMW_CAMERA_GetDCMIPPHandle();
    ipplug_conf.MemoryPageSize = DCMIPP_MEMORY_PAGE_SIZE_256BYTES;
    ipplug_conf.Client = DCMIPP_CLIENT5;
    ipplug_conf.Traffic = DCMIPP_TRAFFIC_BURST_SIZE_128BYTES;
    ipplug_conf.MaxOutstandingTransactions = DCMIPP_OUTSTANDING_TRANSACTION_3;
    ipplug_conf.DPREGStart = 0;
    ipplug_conf.DPREGEnd   = 639;
    ipplug_conf.WLRURatio  = 15;
    ret = HAL_DCMIPP_SetIPPlugConfig(hdcmipp_ptr, &ipplug_conf);
    assert(ret == HAL_OK);
  }

  dcmipp_conf.output_width  = LCD_WIDTH;
  dcmipp_conf.output_height = LCD_HEIGHT;
  dcmipp_conf.output_format = DCMIPP_PIXEL_PACKER_FORMAT_RGB565_1;
  dcmipp_conf.output_bpp    = LCD_BPP;
  dcmipp_conf.mode          = CMW_Aspect_ratio_crop;
  dcmipp_conf.enable_swap   = 0;
  dcmipp_conf.enable_gamma_conversion = 0;
  ret = CMW_CAMERA_SetPipeConfig(DCMIPP_PIPE1, &dcmipp_conf, &hw_pitch);
  assert(ret == HAL_OK);

  ret = HAL_DCMIPP_PIPE_EnableLineEvent(CMW_CAMERA_GetDCMIPPHandle(),
                                         DCMIPP_PIPE1, DCMIPP_MULTILINE_128_LINES);
  assert(ret == HAL_OK);
  ret = HAL_DCMIPP_PIPE_DisableLineEvent(CMW_CAMERA_GetDCMIPPHandle(),
                                          DCMIPP_PIPE1);
  assert(ret == HAL_OK);

  ret = CMW_CAMERA_Start(DCMIPP_PIPE1, (uint8_t *)LCD_FB_ADDRESS,
                         CMW_MODE_CONTINUOUS);
  assert(ret == CMW_ERROR_NONE);

  for (int t = 0; t < ISP_WARMUP_TICKS; t++)
  {
    CMW_CAMERA_Run();
    tx_thread_sleep(1);
  }

  dbg("[AI] ISP warmup done - continuous camera active\r\n");

  while (1)
  {
    CMW_CAMERA_Run();

    /* Keep placeholder for AI activity in FSBL. */
    ai_inference_count++;

    tx_thread_sleep(1);
  }
}

/* ------------------------------------------------------------------ */
/*  Minimal CDC write helpers                                         */
/* ------------------------------------------------------------------ */
static UINT cdc_write(const void *buf, ULONG len)
{
  ULONG actual = 0;
  if (g_cdc_acm == UX_NULL) return UX_ERROR;
  return ux_device_class_cdc_acm_write(g_cdc_acm, (UCHAR *)buf, len, &actual);
}

static UINT cdc_send_str(const char *s)
{
  return cdc_write(s, (ULONG)strlen(s));
}

/* ------------------------------------------------------------------ */
/*  Command handler thread — USB active, never waits for commands     */
/* ------------------------------------------------------------------ */
static void cmd_thread_entry(ULONG thread_input)
{
  static int ready_sent = 0;

  (void)thread_input;

  dbg("\r\n[DBG] cmd_thread started\r\n");

  while (1)
  {
    if (g_cdc_acm != UX_NULL)
    {
      if (!ready_sent)
      {
        cdc_send_str("READY\r\n");
        ready_sent = 1;
        dbg("[DBG] CDC ACM active (non-blocking mode)\r\n");
      }
    }
    else
    {
      ready_sent = 0;
    }

    /* Keep USB thread alive but never block waiting for serial input. */
    tx_thread_sleep(20);
  }
}

/* USER CODE END 1 */
