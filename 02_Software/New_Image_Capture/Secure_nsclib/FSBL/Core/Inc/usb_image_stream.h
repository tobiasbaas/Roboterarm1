/**
  ******************************************************************************
  * @file    usb_image_stream.h
  * @brief   USB CDC ACM image streaming interface
  ******************************************************************************
  */

#ifndef USB_IMAGE_STREAM_H
#define USB_IMAGE_STREAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
  * @brief  Initialise the image-streaming subsystem.
  *         Creates the TX ThreadX thread that listens for capture requests
  *         received on the CDC ACM bulk-OUT endpoint and replies with RGB565
  *         frames on the bulk-IN endpoint.
  *
  *         Protocol (host → device):
  *           'S' (0x53)  – request a single snapshot
  *
  *         Protocol (device → host):
  *           [HDR 8 B] [pixel data]
  *           HDR: 0xAA 0x55  width_lo width_hi  height_lo height_hi  0x00 0x00
  *           pixel data: width * height * 2 bytes (RGB565, little-endian)
  */
void USB_ImageStream_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* USB_IMAGE_STREAM_H */
