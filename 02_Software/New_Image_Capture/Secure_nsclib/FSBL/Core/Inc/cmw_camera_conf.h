/**
  ******************************************************************************
  * @file    cmw_camera_conf.h
  * @brief   Camera Middleware configuration for STM32N6570-DK
  ******************************************************************************
  */

#ifndef CMW_CAMERA_CONF_H
#define CMW_CAMERA_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32n6xx_hal.h"
#include "stm32n6570_discovery_bus.h"

/* Enable the camera sensors available on STM32N6570-DK */
#define USE_IMX335_SENSOR

#ifdef __cplusplus
}
#endif

#endif /* CMW_CAMERA_CONF_H */
