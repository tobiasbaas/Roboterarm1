/**
  ******************************************************************************
  * @file    isp_param_conf.h
  * @brief   ISP IQ parameter configuration - includes per-sensor ISP params
  ******************************************************************************
  */

#ifndef __ISP_PARAM_CONF__H
#define __ISP_PARAM_CONF__H

#include "app_camera_config.h"
#include "cmw_camera.h"

#include "imx335_isp_param_conf.h"

static const ISP_IQParamTypeDef* ISP_IQParamCacheInit[] = {
    &ISP_IQParamCacheInit_IMX335,
};

#endif /* __ISP_PARAM_CONF__H */
