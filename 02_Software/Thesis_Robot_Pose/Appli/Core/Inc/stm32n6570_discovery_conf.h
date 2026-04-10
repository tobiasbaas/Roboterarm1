/**
  ******************************************************************************
  * @file    stm32n6570_discovery_conf.h
  * @brief   STM32N6570-DK BSP configuration file.
  ******************************************************************************
  */

#ifndef STM32N6570_DISCOVERY_CONF_H
#define STM32N6570_DISCOVERY_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32n6xx_hal.h"

/* Board revision: Rev C01 = 3 */
#define STM32N6570_DK_BSP_BOARD_REV   3U

/* COM / UART logging */
#define USE_BSP_COM_FEATURE   0U
#define USE_COM_LOG           0U

/* LCD Layer 0 framebuffer address */
#define LCD_LAYER_0_ADDRESS   0x34200000U

/* ISP defaults (not used in this basic build) */
#define ISP_ALGO_BYPASS       0U
#define ISP_FRAMERATE         30U

/* BSP Button interrupt priorities */
#define BSP_BUTTON_USER1_IT_PRIORITY   15U
#define BSP_BUTTON_TAMP_IT_PRIORITY    15U

#ifdef __cplusplus
}
#endif

#endif /* STM32N6570_DISCOVERY_CONF_H */
