/**
  ******************************************************************************
  * @file    app_camera.h
  * @brief   Camera pipeline header (DCMIPP + CMW) for ThreadX
  ******************************************************************************
  */

#ifndef APP_CAMERA_H
#define APP_CAMERA_H

#include <stdint.h>

/* Initialize camera sensor and DCMIPP display pipe */
int App_Camera_Init(void);

/* Start continuous camera capture to display buffer */
int App_Camera_DisplayStart(void);

/* Called periodically to run ISP (AEC/AWB) */
void App_Camera_IspUpdate(void);

/* Capture a single frame (snapshot) into the provided buffer.
   Returns 0 on success. The buffer must be large enough for
   display_width * display_height * 2 bytes (RGB565). */
int App_Camera_CaptureSnapshot(uint8_t *dst_buffer, uint32_t buf_size);

/* Get current display pipe output dimensions */
void App_Camera_GetDisplaySize(uint32_t *width, uint32_t *height);

/* Get pointer to the live display framebuffer */
uint8_t *App_Camera_GetDisplayBuffer(void);

#endif /* APP_CAMERA_H */
