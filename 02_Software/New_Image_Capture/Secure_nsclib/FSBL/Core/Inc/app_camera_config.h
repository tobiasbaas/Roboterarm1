/**
  ******************************************************************************
  * @file    app_camera_config.h
  * @brief   Camera + Display configuration for New_Image_Capture
  ******************************************************************************
  */

#ifndef APP_CAMERA_CONFIG_H
#define APP_CAMERA_CONFIG_H

/* ---- Display ---- */
#define SCREEN_WIDTH   800
#define SCREEN_HEIGHT  480

/* ---- Camera ---- */
/* Mirror/Flip: CMW_MIRRORFLIP_NONE / CMW_MIRRORFLIP_FLIP / CMW_MIRRORFLIP_MIRROR / CMW_MIRRORFLIP_FLIP_MIRROR */
#define CAMERA_FLIP       CMW_MIRRORFLIP_NONE

/* Aspect ratio mode for display pipe */
#define ASPECT_RATIO_CROP       1
#define ASPECT_RATIO_FIT        2
#define ASPECT_RATIO_FULLSCREEN 3
#define ASPECT_RATIO_MODE       ASPECT_RATIO_FULLSCREEN

/* Max width for display pipe (full screen width since we don't have side panel) */
#define DISPLAY_PIPE_MAX_WIDTH  SCREEN_WIDTH

/* Camera capture resolution (0 = use sensor default) */
#define CAMERA_WIDTH   0
#define CAMERA_HEIGHT  0
#define CAMERA_FPS     30

/* ---- USB Image Streaming ---- */
/* Image is captured as RGB565 and sent over USBX CDC ACM */
#define USB_STREAM_ENABLE       1

/* Capture resolution for USB transfer (display pipe output) */
/* This will be the same as the display pipe output resolution */

#endif /* APP_CAMERA_CONFIG_H */
