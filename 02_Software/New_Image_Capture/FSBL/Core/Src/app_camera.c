/**
  ******************************************************************************
  * @file    app_camera.c
  * @brief   Camera pipeline implementation using CMW (DCMIPP) for ThreadX
  *
  *          Adapted from Lars Beispiel/Application/STM32N6570-DK for use with
  *          ThreadX RTOS and USBX CDC image streaming.
  ******************************************************************************
  */

#include <assert.h>
#include <string.h>
#include "cmw_camera.h"
#include "app_camera.h"
#include "app_camera_config.h"

/* ---- Frame event counters (set by DCMIPP IRQ callback) ---- */
volatile int32_t displayFrameReceived = 0;

/* ---- Display pipe state ---- */
static uint32_t display_width  = 0;
static uint32_t display_height = 0;

/* Display framebuffer - placed in external PSRAM (memory-mapped XSPI1 region).
   BSP_XSPI_RAM_Init() + EnableMemoryMappedMode() must be called before use. */
#define PSRAM_BASE_ADDRESS  0x90000000UL
static uint8_t *lcd_bg_buffer = (uint8_t *)PSRAM_BASE_ADDRESS;

/* ------------------------------------------------------------------ */

/**
  * @brief Configure the display pipe (PIPE1) of the DCMIPP
  */
static void DCMIPP_PipeInitDisplay(CMW_CameraInit_t *camConf)
{
  CMW_DCMIPP_Conf_t dcmipp_conf = {0};
  int ret;
  CMW_Aspect_Ratio_Mode_t aspect_ratio;

  if (ASPECT_RATIO_MODE == ASPECT_RATIO_CROP)
    aspect_ratio = CMW_Aspect_ratio_crop;
  else if (ASPECT_RATIO_MODE == ASPECT_RATIO_FIT)
    aspect_ratio = CMW_Aspect_ratio_fit;
  else
    aspect_ratio = CMW_Aspect_ratio_fullscreen;

  uint32_t cam_w = (camConf->width  == 0U) ? DISPLAY_PIPE_MAX_WIDTH : camConf->width;
  uint32_t cam_h = (camConf->height == 0U) ? SCREEN_HEIGHT          : camConf->height;

  int lcd_bg_height = (cam_h <= SCREEN_HEIGHT) ? (int)cam_h : SCREEN_HEIGHT;
  int lcd_bg_width;

#if ASPECT_RATIO_MODE == ASPECT_RATIO_FULLSCREEN
  lcd_bg_width = (int)(((cam_w * (uint32_t)lcd_bg_height) / cam_h));
  lcd_bg_width -= lcd_bg_width % 16; /* DCMIPP needs multiple-of-16 width */
#else
  lcd_bg_width = lcd_bg_height;
#endif

  if (lcd_bg_width > (int)DISPLAY_PIPE_MAX_WIDTH)
    lcd_bg_width = (int)DISPLAY_PIPE_MAX_WIDTH;
  lcd_bg_width = (lcd_bg_width / 16) * 16;
  if (lcd_bg_width == 0)
    lcd_bg_width = 16;

  display_width  = (uint32_t)lcd_bg_width;
  display_height = (uint32_t)lcd_bg_height;

  dcmipp_conf.output_width  = display_width;
  dcmipp_conf.output_height = display_height;
  dcmipp_conf.output_format = DCMIPP_PIXEL_PACKER_FORMAT_RGB565_1;
  dcmipp_conf.output_bpp    = 2;
  dcmipp_conf.mode          = aspect_ratio;
  dcmipp_conf.enable_gamma_conversion = 0;

  uint32_t pitch;
  ret = CMW_CAMERA_SetPipeConfig(DCMIPP_PIPE1, &dcmipp_conf, &pitch);
  assert(ret == HAL_OK);
  (void)ret;
}

/* ------------------------------------------------------------------ */

int App_Camera_Init(void)
{
  int ret;
  CMW_CameraInit_t cam_conf;

  cam_conf.width       = CAMERA_WIDTH;
  cam_conf.height      = CAMERA_HEIGHT;
  cam_conf.fps         = CAMERA_FPS;
  cam_conf.mirror_flip = CAMERA_FLIP;

  ret = CMW_CAMERA_Init(&cam_conf, NULL);
  if (ret != CMW_ERROR_NONE)
    return ret;

  DCMIPP_PipeInitDisplay(&cam_conf);
  return 0;
}

int App_Camera_DisplayStart(void)
{
  int ret;
  ret = CMW_CAMERA_Start(DCMIPP_PIPE1, lcd_bg_buffer, CMW_MODE_CONTINUOUS);
  return ret;
}

void App_Camera_IspUpdate(void)
{
  CMW_CAMERA_Run();
}

int App_Camera_CaptureSnapshot(uint8_t *dst_buffer, uint32_t buf_size)
{
  uint32_t frame_size = display_width * display_height * 2; /* RGB565 */
  if (buf_size < frame_size)
    return -1;

  /* Wait for next frame to arrive */
  int32_t prev = displayFrameReceived;
  /* Timeout: ~500ms at 30fps is ~15 frames */
  for (volatile uint32_t timeout = 0; timeout < 5000000; timeout++)
  {
    if (displayFrameReceived != prev)
      break;
  }
  if (displayFrameReceived == prev)
    return -2; /* timeout */

  /* Copy from the live display buffer */
  memcpy(dst_buffer, lcd_bg_buffer, frame_size);
  return 0;
}

void App_Camera_GetDisplaySize(uint32_t *width, uint32_t *height)
{
  if (width)  *width  = display_width;
  if (height) *height = display_height;
}

uint8_t *App_Camera_GetDisplayBuffer(void)
{
  return lcd_bg_buffer;
}

/* ---- DCMIPP frame event callback (called from IRQ context) ---- */
int CMW_CAMERA_PIPE_FrameEventCallback(uint32_t pipe)
{
  if (pipe == DCMIPP_PIPE1)
    displayFrameReceived++;
  return 0;
}

void CMW_CAMERA_PIPE_ErrorCallback(uint32_t pipe)
{
  (void)pipe;
}

/* ---- DCMIPP clock config (called by CMW internally) ---- */
HAL_StatusTypeDef MX_DCMIPP_ClockConfig(DCMIPP_HandleTypeDef *hdcmipp)
{
  (void)hdcmipp;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  HAL_StatusTypeDef ret;

  /* DCMIPP kernel clock: IC17 from PLL2 (1000 MHz) / 3 ≈ 333 MHz */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_DCMIPP;
  PeriphClkInitStruct.DcmippClockSelection = RCC_DCMIPPCLKSOURCE_IC17;
  PeriphClkInitStruct.ICSelection[RCC_IC17].ClockSelection = RCC_ICCLKSOURCE_PLL2;
  PeriphClkInitStruct.ICSelection[RCC_IC17].ClockDivider = 3;
  ret = HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
  if (ret != HAL_OK)
    return ret;

  /* CSI PHY reference clock: IC18 from PLL1 / 60 = 20 MHz */
  memset(&PeriphClkInitStruct, 0, sizeof(PeriphClkInitStruct));
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CSI;
  PeriphClkInitStruct.ICSelection[RCC_IC18].ClockSelection = RCC_ICCLKSOURCE_PLL1;
  PeriphClkInitStruct.ICSelection[RCC_IC18].ClockDivider = 60;
  ret = HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  return ret;
}
