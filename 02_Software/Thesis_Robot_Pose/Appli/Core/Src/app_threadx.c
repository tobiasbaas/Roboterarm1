/* USER CODE BEGIN Header */
/*
Innerhalb dieses Codeabschnittes befinden sich die unterschiedlichen Threads für die Kamera, 
die USB CDC Schnittstelle und den Edge AI Inferenzprozess. Der Aufruf der richtigen Funktionen
wird geregelt über den Scheduler. 
4
--> Muss noch entschieden werden, ob alles in einem Thread, oder ob du die Implementierung 
    in meherern ausgeführt werden soll
*/

/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmw_camera.h"
#include "stm32n6570_discovery.h"
#include "stm32n6570_discovery_bus.h"
#include "ux_device_cdc_acm.h"
#include "app_x-cube-ai.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include "main.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//USBX Stack Defines fertig
#define CAMERA_THREAD_STACK_SIZE  8192
#define CAMERA_THREAD_PRIORITY    15
#define CMD_THREAD_STACK_SIZE     4096
#define CMD_THREAD_PRIORITY       12

//Initialisierung des verbauten LCD-Displays
#define LCD_FB_ADDRESS            0x34000000U
#define LCD_WIDTH                 640
#define LCD_HEIGHT                480
#define LCD_BPP                   2
#define LCD_FB_SIZE               (LCD_WIDTH * LCD_HEIGHT * LCD_BPP)

//Notwendige Defines für die Kamera IMX335
#define SENSOR_IMX335_WIDTH       2592
#define SENSOR_IMX335_HEIGHT      1944
#define CAMERA_FPS                30

//ISP Warmup und Capture Settle Ticks (Beinhaltet Kameraeinstellungen, Belichtung, Weißabgleich etc.)
#define ISP_WARMUP_TICKS          200
#define CAPTURE_SETTLE_TICKS      50

// USB CDC Buffergrößen und Timeout
#define CDC_RX_BUF_SIZE           64
#define CDC_TX_BUF_SIZE           8192  /* match UX_SLAVE_REQUEST_DATA_MAX_LENGTH */
#define CDC_WRITE_RETRIES         5     /* retries per chunk on transient error */
#define XFER_TIMEOUT_SEC          10    /* max seconds for full image transfer */

//Bilderheader Definition for USB transfer (Eindeutiger Indikator)
#define IMG_HEADER_MAGIC          "IMG:"
#define IMG_HEADER_MAGIC_LEN      4
#define IMG_HEADER_SIZE           8

//Edge AI Thread Definierungen
#define AI_THREAD_STACK_SIZE      4096
#define AI_THREAD_PRIORITY        10
#define AI_MODEL_SWITCH_DEBOUNCE_TICKS 200

//Edge AI Display Definierungen
#define APP_AI_DISPLAY_FB_ADDR    0x34000000U
#define APP_AI_DISPLAY_WIDTH      640
#define APP_AI_DISPLAY_HEIGHT     480
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */



/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
//Gloablen Variablenfestlegungen für die einzelnen Threads, Modelle, Button und LED Status, sowie Input Pattern Counter und Inference Status
static TX_THREAD camera_thread;
static UCHAR camera_thread_stack[CAMERA_THREAD_STACK_SIZE];

static TX_THREAD cmd_thread;
static UCHAR cmd_thread_stack[CMD_THREAD_STACK_SIZE];

static TX_THREAD ai_thread;
static ULONG ai_thread_stack[AI_THREAD_STACK_SIZE / sizeof(ULONG)];
static volatile int pipe_suspended = 0;
static AppAIModel_t g_requested_model = APP_AI_MODEL_POSE;
static uint8_t g_button_init_done = 0U;
static uint8_t g_button_was_pressed = 0U;
static ULONG g_last_toggle_tick = 0U;
static uint8_t g_led_init_done = 0U;
static uint32_t g_input_pattern_counter = 0U;
static uint8_t g_inference_heartbeat = 0U;
static uint8_t g_thread_heartbeat = 0U;
static uint8_t g_last_inference_ok = 0U;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void camera_thread_entry(ULONG thread_input);
static void cmd_thread_entry(ULONG thread_input);
static VOID ai_thread_entry(ULONG thread_input);

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */

//-------------------------------------------THREAD Creates -------------------------------------------
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  (void)memory_ptr;

  UINT ret = TX_SUCCESS;

  //Edge AI Inferenzthread für die kontinuierliche Ausführung der KI-Modelle und Ausgabe auf das LCD Display
  ret = tx_thread_create(&ai_thread,
                         "AI Thread",
                         ai_thread_entry,
                         0,
                         ai_thread_stack,
                         sizeof(ai_thread_stack),
                         AI_THREAD_PRIORITY,
                         AI_THREAD_PRIORITY,
                         TX_NO_TIME_SLICE,
                         TX_AUTO_START);
                          
  if (ret != TX_SUCCESS)
  {
    return ret;
  }

  //Kamerathread für die kontinuierliche Bilderfassung von der Kamera und Ausgabe auf das LCD Display
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

  //Serieller Command Thread für die USB CDC Schnittstelle zum Debuggen bzw. Bilder 
  //über die USB Schnittstelle zu übertragen
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
//-------------------------------------------THREAD Creates ENDE  -------------------------------------------


VOID tx_application_define(VOID *first_unused_memory)
{
  (void)first_unused_memory;
  (void)App_ThreadX_Init(TX_NULL);
}

void MX_ThreadX_Init(void)
{
  /* USER CODE BEGIN Before_Kernel_Start */

  /* USER CODE END Before_Kernel_Start */

  tx_kernel_enter();
  
  /* USER CODE BEGIN Kernel_Start_Error */

  /* USER CODE END Kernel_Start_Error */
}

void valueNotSetted(ULONG thread_input)
{
  (void)thread_input;
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

static void dbg_val(const char *prefix, int val)
{
  char buf[64];
  int n = snprintf(buf, sizeof(buf), "%s%d\r\n", prefix, val);
  HAL_UART_Transmit(&huart1, (const uint8_t *)buf, (uint16_t)n, 50);
}

/* ------------------------------------------------------------------ */
/*  Camera thread — unchanged                                         */
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

  /* Camera stays running after warmup for live AI inference */
  while (1)
  {
    CMW_CAMERA_Run();
    tx_thread_sleep(1);
  }
}

/* ------------------------------------------------------------------ */
/*  USB CDC Funktionen, welche verantwortlich für die das Senden und
  Empfangen von Daten über die USB Schnittstelle. Es sendet bei Erfolg
  ein SUCCESS zurück oder ein Error
*/
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
/*  Command handler thread — UART debug at every step                 */
/* ------------------------------------------------------------------ */
static void cmd_thread_entry(ULONG thread_input)
{
  UCHAR rx_buf[CDC_RX_BUF_SIZE];
  ULONG actual_length;
  UINT  status;
  static UCHAR __attribute__((aligned(32))) usb_tx_buf[CDC_TX_BUF_SIZE];
  int write_count = 0;  /* total USB writes since boot */

  (void)thread_input;

  dbg("\r\n[DBG] cmd_thread started\r\n");

  /* Wait for USB enumeration */
  while (g_cdc_acm == UX_NULL)
    tx_thread_sleep(10);

  dbg("[DBG] CDC ACM activated\r\n");

  /* Send 3 × READY with debug */
  for (int i = 0; i < 3; i++)
  {
    dbg_val("[DBG] READY write #", i);
    status = cdc_send_str("READY\r\n");
    dbg_val("[DBG] READY status=", (int)status);
    write_count++;
    tx_thread_sleep(20);  /* 200ms spacing */
  }

  dbg("[DBG] entering cmd loop\r\n");

  /* ---- Main command loop ---- */
  while (1)
  {
    if (g_cdc_acm == UX_NULL) { tx_thread_sleep(50); continue; }

    dbg("[DBG] read...\r\n");
    status = ux_device_class_cdc_acm_read(g_cdc_acm, rx_buf,
                                          CDC_RX_BUF_SIZE, &actual_length);
    if (status != UX_SUCCESS || actual_length == 0)
    {
      dbg_val("[DBG] read fail/empty st=", (int)status);
      tx_thread_sleep(10);
      continue;
    }

    /* Null-terminate for easy string compare */
    if (actual_length < CDC_RX_BUF_SIZE) rx_buf[actual_length] = 0;
    dbg("[DBG] rx: ");
    dbg((const char *)rx_buf);
    dbg("\r\n");

    /* ---- PING ---- */
    if (actual_length >= 4 && memcmp(rx_buf, "PING", 4) == 0)
    {
      write_count++;
      dbg_val("[DBG] PONG write #", write_count);
      status = cdc_send_str("PONG\r\n");
      dbg_val("[DBG] PONG status=", (int)status);
      continue;
    }

    /* ---- TEST: send ONE short string, then one slightly larger ---- */
    if (actual_length >= 4 && memcmp(rx_buf, "TEST", 4) == 0)
    {
      /* Step 1: small write (like PONG) */
      write_count++;
      dbg_val("[DBG] TEST_A write #", write_count);
      status = cdc_send_str("TEST_A_OK\r\n");
      dbg_val("[DBG] TEST_A status=", (int)status);

      if (status != UX_SUCCESS) { dbg("[DBG] TEST_A FAILED!\r\n"); continue; }

      /* 100ms gap */
      tx_thread_sleep(10);

      /* Step 2: another small write */
      write_count++;
      dbg_val("[DBG] TEST_B write #", write_count);
      status = cdc_send_str("TEST_B_OK\r\n");
      dbg_val("[DBG] TEST_B status=", (int)status);

      if (status != UX_SUCCESS) { dbg("[DBG] TEST_B FAILED!\r\n"); continue; }

      tx_thread_sleep(10);

      /* Step 3: slightly larger write (64 bytes) */
      memset(usb_tx_buf, 'X', 60);
      memcpy(usb_tx_buf + 60, "\r\n", 2);
      write_count++;
      dbg_val("[DBG] TEST_C write #", write_count);
      status = cdc_write(usb_tx_buf, 62);
      dbg_val("[DBG] TEST_C status=", (int)status);

      if (status != UX_SUCCESS) { dbg("[DBG] TEST_C FAILED!\r\n"); continue; }

      cdc_send_str("TEST_DONE\r\n");
      dbg("[DBG] TEST complete\r\n");
      continue;
    }

    /* ---- CAPTURE ---- */
    if (actual_length >= 7 && memcmp(rx_buf, "CAPTURE", 7) == 0)
    {
      dbg("[DBG] CAPTURE start\r\n");

      /* Resume camera — status only via UART debug, NOT via CDC
         (sending text on CDC before IMG: header desynchronises the stream) */
      if (pipe_suspended) {
        CMW_CAMERA_Resume(DCMIPP_PIPE1);
        pipe_suspended = 0;
        dbg("[DBG] cam resumed\r\n");
      }
      tx_thread_sleep(CAPTURE_SETTLE_TICKS);

      CMW_CAMERA_Suspend(DCMIPP_PIPE1);
      pipe_suspended = 1;
      tx_thread_sleep(2);
      dbg("[DBG] cam frozen\r\n");

      /* Build header */
      uint32_t payload_size = LCD_FB_SIZE;
      UCHAR header[IMG_HEADER_SIZE];
      memcpy(header, IMG_HEADER_MAGIC, IMG_HEADER_MAGIC_LEN);
      header[4] = (UCHAR)((payload_size >>  0) & 0xFF);
      header[5] = (UCHAR)((payload_size >>  8) & 0xFF);
      header[6] = (UCHAR)((payload_size >> 16) & 0xFF);
      header[7] = (UCHAR)((payload_size >> 24) & 0xFF);

      dbg("[DBG] sending header\r\n");
      status = cdc_write(header, IMG_HEADER_SIZE);
      dbg_val("[DBG] header status=", (int)status);
      if (status != UX_SUCCESS) { dbg("[DBG] HDR_FAIL\r\n"); continue; }

      /* Send framebuffer chunk by chunk */
      const uint8_t *fb = (const uint8_t *)LCD_FB_ADDRESS;
      uint32_t remaining = payload_size;
      uint32_t offset = 0;
      ULONG timeout_ticks = (ULONG)TX_TIMER_TICKS_PER_SECOND * XFER_TIMEOUT_SEC;
      ULONG t_start = tx_time_get();
      int chunks = 0;
      int xfer_ok = 1;

      while (remaining > 0)
      {
        if ((tx_time_get() - t_start) > timeout_ticks)
        {
          dbg("[DBG] XFER timeout!\r\n");
          xfer_ok = 0;
          break;
        }

        uint32_t to_send = (remaining > CDC_TX_BUF_SIZE)
                           ? CDC_TX_BUF_SIZE : remaining;
        memcpy(usb_tx_buf, fb + offset, to_send);

        /* Retry loop for transient USB errors */
        ULONG sent = 0;
        int retries = CDC_WRITE_RETRIES;
        do {
          status = ux_device_class_cdc_acm_write(g_cdc_acm, usb_tx_buf,
                                                 to_send, &sent);
          if (status == UX_SUCCESS && sent > 0) break;
          retries--;
          tx_thread_sleep(1); /* 1 tick pause before retry */
        } while (retries > 0);

        if (status != UX_SUCCESS || sent == 0)
        {
          dbg_val("[DBG] write fail st=", (int)status);
          xfer_ok = 0;
          break;
        }

        offset    += sent;
        remaining -= sent;
        chunks++;

        if ((chunks % 50) == 0)
          dbg_val("[DBG] chunks=", chunks);
      }

      /* Only log result via UART — do NOT send text on CDC after binary data */
      char done[64];
      snprintf(done, sizeof(done), "[DBG] XFER_%s: %d chunks, %lu/%lu bytes\r\n",
               xfer_ok ? "OK" : "FAIL",
               chunks, (unsigned long)offset, (unsigned long)payload_size);
      dbg(done);
    }
  }
}

/* ------------------------------------------------------------------ */
/*  Edge AI Funktionen. In diesem FUnktionen wird aufgerufen, welches KI Modell
  ausgeführt werden soll per Knopfdruch auf den User Button, da dieser auch
  vorhanden ist auf der selbstgebauten Platine. Desweiteren werden nach den
  Inferenzen die Linien gezeichnet zwischen den Keypoints und die Bounding
  Boxen bzw. Segmentflächen um das auf das Display zu zeichnen.
*/
/* ------------------------------------------------------------------ */
static uint32_t App_AI_IntegerSqrtLocal(uint32_t v)
{
  uint32_t r = 0U;
  while ((r + 1U) * (r + 1U) <= v)
  {
    ++r;
  }
  return r;
}

__weak GPIO_PinState App_AI_GetModelSwitchActiveState(void)
{
  /* USER1 (B2 / PC13) is typically active-low on STM32 discovery boards. */
  return GPIO_PIN_RESET;
}

static void App_AI_UpdateRequestedModelFromButton(void)
{
  uint8_t is_pressed;
  ULONG now;

  if (g_button_init_done == 0U)
  {
    GPIO_InitTypeDef gpio_init = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();
    gpio_init.Pin = GPIO_PIN_13;
    gpio_init.Mode = GPIO_MODE_INPUT;
    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &gpio_init);
    g_button_init_done = 1U;
  }

  is_pressed = (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == App_AI_GetModelSwitchActiveState()) ? 1U : 0U;
  now = tx_time_get();

  if ((is_pressed != 0U) && (g_button_was_pressed == 0U))
  {
    if ((g_last_toggle_tick == 0U) || ((now - g_last_toggle_tick) >= AI_MODEL_SWITCH_DEBOUNCE_TICKS))
    {
      g_requested_model = (g_requested_model == APP_AI_MODEL_POSE)
        ? APP_AI_MODEL_SEGMENTATION
        : APP_AI_MODEL_POSE;
      g_last_toggle_tick = now;
    }
  }

  g_button_was_pressed = is_pressed;
}

__weak int App_AI_PrepareInput(uint8_t *input_buffer, size_t input_size)
{
  uint16_t *fb = App_AI_GetDisplayFramebuffer();
  uint32_t fb_w = App_AI_GetDisplayWidth();
  uint32_t fb_h = App_AI_GetDisplayHeight();
  size_t i;

  if ((input_buffer == NULL) || (input_size == 0U))
  {
    return -1;
  }

  if ((fb != NULL) && (fb_w != 0U) && (fb_h != 0U))
  {
    uint32_t channels = 1U;
    uint32_t pixels = (uint32_t)input_size;
    uint32_t side;
    uint32_t y;
    uint32_t x;

    if ((input_size % 3U) == 0U)
    {
      channels = 3U;
      pixels = (uint32_t)(input_size / 3U);
    }

    side = App_AI_IntegerSqrtLocal(pixels);
    if ((side != 0U) && ((side * side) == pixels))
    {
      for (y = 0U; y < side; ++y)
      {
        uint32_t sy = (y * fb_h) / side;
        for (x = 0U; x < side; ++x)
        {
          uint32_t sx = (x * fb_w) / side;
          uint16_t px = fb[sy * fb_w + sx];
          uint8_t r = (uint8_t)(((px >> 11) & 0x1FU) << 3);
          uint8_t g = (uint8_t)(((px >> 5) & 0x3FU) << 2);
          uint8_t b = (uint8_t)((px & 0x1FU) << 3);
          uint32_t idx = y * side + x;

          if (channels == 3U)
          {
            uint32_t o = idx * 3U;
            input_buffer[o + 0U] = r;
            input_buffer[o + 1U] = g;
            input_buffer[o + 2U] = b;
          }
          else
          {
            input_buffer[idx] = (uint8_t)(((uint32_t)r + (uint32_t)g + (uint32_t)b) / 3U);
          }
        }
      }
      return 0;
    }
  }

  /* Fallback pattern: allows visible model activity even without camera hook override. */
  for (i = 0U; i < input_size; ++i)
  {
    input_buffer[i] = (uint8_t)((i + g_input_pattern_counter) & 0xFFU);
  }
  g_input_pattern_counter += 3U;
  return 0;
}

__weak void App_AI_OnResult(const uint8_t *output_buffer, size_t output_size)
{
  (void)output_buffer;
  (void)output_size;
}

__weak void App_AI_OnOutputs(const uint8_t *const *outputs,
                             const size_t *output_sizes,
                             uint32_t output_count)
{
  (void)outputs;
  (void)output_sizes;
  (void)output_count;
}

__weak AppAIModel_t App_AI_GetRequestedModel(void)
{
  App_AI_UpdateRequestedModelFromButton();
  return g_requested_model;
}

static VOID ai_thread_entry(ULONG thread_input)
{
  (void)thread_input;

  for (;;)
  {
    g_thread_heartbeat ^= 1U;

    App_AI_SetModel(App_AI_GetRequestedModel());

    if (App_AI_PrepareInput(App_AI_GetInputBuffer(), App_AI_GetInputSize()) == 0)
    {
      const uint8_t *outputs[2] = {0};
      size_t output_sizes[2] = {0};
      uint32_t output_count;
      uint32_t i;

      if (aiRun() == 0)
      {
        g_last_inference_ok = 1U;
        App_AI_RenderActiveModelOverlay();
      }
      else
      {
        g_last_inference_ok = 0U;
      }

      output_count = App_AI_GetOutputCount();
      if (output_count > 2U)
      {
        output_count = 2U;
      }

      for (i = 0U; i < output_count; ++i)
      {
        outputs[i] = App_AI_GetOutputBuffer(i);
        output_sizes[i] = App_AI_GetOutputSize(i);
      }

      App_AI_OnOutputs(outputs, output_sizes, output_count);
      App_AI_OnResult(outputs[0], output_sizes[0]);

      g_inference_heartbeat ^= 1U;
    }
    else
    {
      g_last_inference_ok = 0U;
    }
    tx_thread_sleep(1);
  }
}

/* Board-specific overrides for display framebuffer hooks */
uint16_t *App_AI_GetDisplayFramebuffer(void)
{
  return (uint16_t *)APP_AI_DISPLAY_FB_ADDR;
}

uint32_t App_AI_GetDisplayWidth(void)
{
  return (uint32_t)APP_AI_DISPLAY_WIDTH;
}

uint32_t App_AI_GetDisplayHeight(void)
{
  return (uint32_t)APP_AI_DISPLAY_HEIGHT;
}

/* USER CODE END 1 */
