/**
  ******************************************************************************
  * @file    isp_param_conf.h
  * @brief   Header file for IQT parameters of the ISP middleware.
  ******************************************************************************
  */
#ifndef __ISP_PARAM_CONF__H
#define __ISP_PARAM_CONF__H

#include "cmw_camera.h"
#include "imx335_isp_param_conf.h"

static const ISP_IQParamTypeDef* ISP_IQParamCacheInit[] = {
    &ISP_IQParamCacheInit_IMX335,
};

#endif /* __ISP_PARAM_CONF__H */
