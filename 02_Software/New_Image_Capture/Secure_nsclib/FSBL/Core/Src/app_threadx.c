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

#include "app_camera.h"
#include "stm32n6570_discovery_lcd.h"
#include "app_camera_config.h"
#include "usb_image_stream.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAMERA_THREAD_STACK_SIZE  4096
#define CAMERA_THREAD_PRIO        10
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static TX_THREAD camera_thread;
static ULONG camera_thread_stack[CAMERA_THREAD_STACK_SIZE / sizeof(ULONG)];
/* USER CODE END PV */


/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

static void camera_thread_entry(ULONG arg);
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
  /* Create camera pipeline thread */
  tx_thread_create(&camera_thread, "Camera Thread", camera_thread_entry, 0,
                   camera_thread_stack, CAMERA_THREAD_STACK_SIZE,
                   CAMERA_THREAD_PRIO, CAMERA_THREAD_PRIO,
                   TX_NO_TIME_SLICE, TX_AUTO_START);

  /* Create USB image streaming thread */
  USB_ImageStream_Init();
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

/**
  * @brief  Camera pipeline thread.
  *         Initializes camera (DCMIPP/CMW), configures the LTDC layer for the
  *         camera preview, starts continuous display capture, and runs the ISP
  *         processing loop.
  */
static void camera_thread_entry(ULONG arg)
{
  (void)arg;
  int ret;

  /* Initialize camera sensor + DCMIPP pipe */
  ret = App_Camera_Init();
  if (ret != 0)
  {
    /* Camera init failed – halt this thread */
    while (1)
      tx_thread_sleep(1000);
  }

  /* Configure LTDC Layer 1 (background) to show the camera framebuffer */
  {
    uint32_t disp_w, disp_h;
    App_Camera_GetDisplaySize(&disp_w, &disp_h);

    BSP_LCD_LayerConfig_t layer_cfg = {0};
    layer_cfg.X0          = 0;
    layer_cfg.Y0          = 0;
    layer_cfg.X1          = disp_w;
    layer_cfg.Y1          = disp_h;
    layer_cfg.PixelFormat = LCD_PIXEL_FORMAT_RGB565;
    layer_cfg.Address     = (uint32_t)App_Camera_GetDisplayBuffer();
    BSP_LCD_ConfigLayer(0, LTDC_LAYER_1, &layer_cfg);
  }

  /* Start continuous camera-to-display streaming */
  ret = App_Camera_DisplayStart();
  if (ret != 0)
  {
    while (1)
      tx_thread_sleep(1000);
  }

  /* ISP processing loop – runs forever */
  while (1)
  {
    App_Camera_IspUpdate();
    tx_thread_sleep(10); /* ~100 Hz ISP update rate */
  }
}

/* USER CODE END 1 */
