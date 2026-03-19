/**
  ******************************************************************************
  * @file    usb_image_stream.c
  * @brief   USB CDC ACM image streaming implementation for ThreadX
  *
  *          Listens for a one-byte trigger ('S') from the host on CDC ACM
  *          bulk-OUT, captures a camera snapshot, and sends the RGB565 frame
  *          back on bulk-IN with a minimal header.
  ******************************************************************************
  */

#include "usb_image_stream.h"
#include "ux_device_cdc_acm.h"
#include "app_camera.h"
#include "app_camera_config.h"
#include "tx_api.h"
#include <string.h>

/* ---- Thread resources ---------------------------------------------------- */
#define USB_STREAM_THREAD_STACK_SIZE  4096
#define USB_STREAM_THREAD_PRIO        12   /* lower priority than camera */

static TX_THREAD usb_stream_thread;
static ULONG usb_stream_stack[USB_STREAM_THREAD_STACK_SIZE / sizeof(ULONG)];

/* Temporary snapshot buffer in external PSRAM (after the display buffer).
   Offset by 1 MB from PSRAM base to stay clear of the display framebuffer. */
#define SNAPSHOT_PSRAM_OFFSET  0x00100000UL
#define PSRAM_BASE             0x90000000UL
static uint8_t *snapshot_buf = (uint8_t *)(PSRAM_BASE + SNAPSHOT_PSRAM_OFFSET);

/* ---- Header format ------------------------------------------------------- */
#define FRAME_HEADER_SIZE  8

static void build_frame_header(uint8_t *hdr, uint16_t width, uint16_t height)
{
  hdr[0] = 0xAA;
  hdr[1] = 0x55;
  hdr[2] = (uint8_t)(width & 0xFF);
  hdr[3] = (uint8_t)(width >> 8);
  hdr[4] = (uint8_t)(height & 0xFF);
  hdr[5] = (uint8_t)(height >> 8);
  hdr[6] = 0x00;
  hdr[7] = 0x00;
}

/* ---- Thread entry -------------------------------------------------------- */

static void usb_stream_thread_entry(ULONG arg)
{
  (void)arg;
  UINT status;
  ULONG actual_length;
  uint8_t rx_byte;
  uint8_t hdr[FRAME_HEADER_SIZE];

  /* Wait until USB CDC ACM is connected */
  while (!USBD_CDC_ACM_IsActive())
  {
    tx_thread_sleep(100);
  }

  while (1)
  {
    UX_SLAVE_CLASS_CDC_ACM *cdc = USBD_CDC_ACM_GetInstance();
    if (cdc == UX_NULL)
    {
      tx_thread_sleep(100);
      continue;
    }

    /* Wait for a command byte from the host */
    status = ux_device_class_cdc_acm_read(cdc, &rx_byte, 1, &actual_length);
    if (status != UX_SUCCESS || actual_length == 0)
    {
      tx_thread_sleep(10);
      continue;
    }

    /* 'S' = snapshot request */
    if (rx_byte == 'S' || rx_byte == 's')
    {
      uint32_t disp_w, disp_h;
      App_Camera_GetDisplaySize(&disp_w, &disp_h);
      uint32_t frame_bytes = disp_w * disp_h * 2; /* RGB565 */

      /* Capture current camera frame */
      int ret = App_Camera_CaptureSnapshot(snapshot_buf, frame_bytes);
      if (ret != 0)
        continue;  /* capture failed, ignore */

      /* Send header */
      build_frame_header(hdr, (uint16_t)disp_w, (uint16_t)disp_h);
      status = ux_device_class_cdc_acm_write(cdc, hdr, FRAME_HEADER_SIZE, &actual_length);
      if (status != UX_SUCCESS)
        continue;

      /* Send pixel data in chunks (CDC ACM max packet is typically 512 B) */
      uint32_t offset = 0;
      const uint32_t chunk = 4096;
      while (offset < frame_bytes)
      {
        uint32_t to_send = frame_bytes - offset;
        if (to_send > chunk)
          to_send = chunk;

        status = ux_device_class_cdc_acm_write(cdc, &snapshot_buf[offset],
                                                to_send, &actual_length);
        if (status != UX_SUCCESS)
          break;
        offset += to_send;
      }
    }
  }
}

/* ---- Public API ---------------------------------------------------------- */

void USB_ImageStream_Init(void)
{
  tx_thread_create(&usb_stream_thread, "USB Image Stream",
                   usb_stream_thread_entry, 0,
                   usb_stream_stack, USB_STREAM_THREAD_STACK_SIZE,
                   USB_STREAM_THREAD_PRIO, USB_STREAM_THREAD_PRIO,
                   TX_NO_TIME_SLICE, TX_AUTO_START);
}
