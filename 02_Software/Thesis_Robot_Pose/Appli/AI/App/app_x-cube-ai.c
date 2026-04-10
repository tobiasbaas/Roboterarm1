
/**
  ******************************************************************************
  * @file    app_x-cube-ai.c
  * @author  X-CUBE-AI C code generator
  * @brief   AI program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

  /**
    * Description
    * Minimum template to show how to use the Neural-ART Embedded Client API
    *          Re-target of the printf function is out-of-scope.
    *
    *
    */

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/


/* System headers */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "app_x-cube-ai.h"
#include "bsp_ai.h"
#include "stai.h"
#include "npu_init.h"
#include "ll_aton_NN_interface.h"
#ifdef USE_THREADX_AI_RUNTIME
#include "tx_api.h"
#endif



/* USER CODE BEGIN includes */
/* USER CODE END includes */

/* IO buffers ----------------------------------------------------------------*/


/* Input defs ----------------------------------------------------------------*/

/**

// Array to store the data of the input tensor
stai_ptr data_ins[] = {
}; 
*/

/* Output defs ----------------------------------------------------------------*/

/**

// c-array to store the data of the output tensor
stai_ptr data_outs[] = {
}; 
*/




/* Activations buffers -------------------------------------------------------*/


/* Entry points --------------------------------------------------------------*/
LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(pose)
LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(seg)

uint8_t *buffer_in;
uint8_t *buffer_out;

typedef struct
{
  NN_Instance_TypeDef *instance;
  uint8_t *input;
  uint8_t *outputs[2];
  size_t input_size;
  size_t output_sizes[2];
  uint32_t output_count;
} AppAIModelCtx_t;

static AppAIModelCtx_t g_pose_ctx;
static AppAIModelCtx_t g_seg_ctx;
static volatile AppAIModel_t g_active_model = APP_AI_MODEL_POSE;
static volatile uint32_t g_ai_run_counter = 0U;
static volatile int32_t g_ai_last_run_status = 0;

__weak uint16_t *App_AI_GetDisplayFramebuffer(void)
{
  /*
   * IMPORTANT:
   * 0x34000000 is frequently used by generated AI flex memory on STM32N6.
   * Returning NULL by default avoids corrupting AI buffers when no board-specific
   * display framebuffer hook is provided.
   */
  return NULL;
}

__weak uint32_t App_AI_GetDisplayWidth(void)
{
  return 800U;
}

__weak uint32_t App_AI_GetDisplayHeight(void)
{
  return 480U;
}

static AppAIModelCtx_t *App_AI_GetModelCtx(AppAIModel_t model)
{
  if (model == APP_AI_MODEL_SEGMENTATION)
  {
    return &g_seg_ctx;
  }
  return &g_pose_ctx;
}

static void App_AI_RefreshActiveBuffers(void)
{
  AppAIModelCtx_t *ctx = App_AI_GetModelCtx((AppAIModel_t)g_active_model);
  buffer_in = ctx->input;
  buffer_out = ctx->outputs[0];
}

static void App_AI_InitModelCtx(AppAIModelCtx_t *ctx,
                                NN_Instance_TypeDef *instance,
                                uint32_t expected_outputs)
{
  const LL_Buffer_InfoTypeDef *input_infos;
  const LL_Buffer_InfoTypeDef *output_infos;
  uint32_t i;

  ctx->instance = instance;
  ctx->output_count = expected_outputs;

  input_infos = LL_ATON_Input_Buffers_Info(instance);
  output_infos = LL_ATON_Output_Buffers_Info(instance);

  ctx->input = (uint8_t *)LL_Buffer_addr_start(&input_infos[0]);
  ctx->input_size = (size_t)LL_Buffer_len(&input_infos[0]);

  for (i = 0U; i < 2U; ++i)
  {
    ctx->outputs[i] = NULL;
    ctx->output_sizes[i] = 0U;
  }

  for (i = 0U; (i < expected_outputs) && (i < 2U); ++i)
  {
    ctx->outputs[i] = (uint8_t *)LL_Buffer_addr_start(&output_infos[i]);
    ctx->output_sizes[i] = (size_t)LL_Buffer_len(&output_infos[i]);
  }
}

static uint16_t App_AI_ColorBlend565(uint16_t base, uint16_t over)
{
  uint32_t br = (uint32_t)((base >> 11) & 0x1FU);
  uint32_t bg = (uint32_t)((base >> 5) & 0x3FU);
  uint32_t bb = (uint32_t)(base & 0x1FU);
  uint32_t orr = (uint32_t)((over >> 11) & 0x1FU);
  uint32_t og = (uint32_t)((over >> 5) & 0x3FU);
  uint32_t ob = (uint32_t)(over & 0x1FU);

  uint32_t rr = (br + orr) >> 1;
  uint32_t rg = (bg + og) >> 1;
  uint32_t rb = (bb + ob) >> 1;

  return (uint16_t)((rr << 11) | (rg << 5) | rb);
}

typedef struct
{
  float x_center;
  float y_center;
  float width;
  float height;
  uint8_t class_index;
  int8_t confidence_raw;
  int8_t mask_coeffs[64];
} AppAISegDetection_t;

static float App_AI_Int8ToUnit(int8_t value)
{
  return (float)((int32_t)value + 128) / 255.0f;
}

static float App_AI_Clamp01(float value)
{
  if (value < 0.0f)
  {
    return 0.0f;
  }
  if (value > 1.0f)
  {
    return 1.0f;
  }
  return value;
}

static float App_AI_IoU(const AppAISegDetection_t *a, const AppAISegDetection_t *b)
{
  float ax0 = a->x_center - (0.5f * a->width);
  float ay0 = a->y_center - (0.5f * a->height);
  float ax1 = a->x_center + (0.5f * a->width);
  float ay1 = a->y_center + (0.5f * a->height);
  float bx0 = b->x_center - (0.5f * b->width);
  float by0 = b->y_center - (0.5f * b->height);
  float bx1 = b->x_center + (0.5f * b->width);
  float by1 = b->y_center + (0.5f * b->height);
  float ix0 = (ax0 > bx0) ? ax0 : bx0;
  float iy0 = (ay0 > by0) ? ay0 : by0;
  float ix1 = (ax1 < bx1) ? ax1 : bx1;
  float iy1 = (ay1 < by1) ? ay1 : by1;
  float iw = ix1 - ix0;
  float ih = iy1 - iy0;
  float inter;
  float a_area;
  float b_area;
  float uni;

  if ((iw <= 0.0f) || (ih <= 0.0f))
  {
    return 0.0f;
  }

  inter = iw * ih;
  a_area = a->width * a->height;
  b_area = b->width * b->height;
  uni = a_area + b_area - inter;
  if (uni <= 0.0f)
  {
    return 0.0f;
  }
  return inter / uni;
}

static int App_AI_InferYoloSegParams(size_t det_size,
                                     size_t mask_size,
                                     uint32_t *boxes,
                                     uint32_t *classes,
                                     uint32_t *masks,
                                     uint32_t *mask_side)
{
  static const uint32_t box_candidates[] = {8400U, 5376U, 25200U, 1344U, 1008U, 768U};
  static const uint32_t side_candidates[] = {160U, 128U, 96U, 80U, 64U, 48U, 40U, 32U};
  uint32_t inferred_masks = 0U;
  uint32_t inferred_side = 0U;
  uint32_t best_boxes = 0U;
  uint32_t best_classes = 0U;
  uint32_t i;

  if ((det_size == 0U) || (mask_size == 0U) || (boxes == NULL) || (classes == NULL) ||
      (masks == NULL) || (mask_side == NULL))
  {
    return 0;
  }

  for (i = 0U; i < (sizeof(side_candidates) / sizeof(side_candidates[0])); ++i)
  {
    uint32_t side = side_candidates[i];
    uint32_t pixel_count = side * side;
    if ((pixel_count != 0U) && ((mask_size % pixel_count) == 0U))
    {
      uint32_t nb_masks = (uint32_t)(mask_size / pixel_count);
      if ((nb_masks >= 4U) && (nb_masks <= 64U))
      {
        inferred_masks = nb_masks;
        inferred_side = side;
        break;
      }
    }
  }

  if (inferred_masks == 0U)
  {
    return 0;
  }

  for (i = 0U; i < (sizeof(box_candidates) / sizeof(box_candidates[0])); ++i)
  {
    uint32_t candidate_boxes = box_candidates[i];
    if ((candidate_boxes != 0U) && ((det_size % candidate_boxes) == 0U))
    {
      uint32_t attrs = (uint32_t)(det_size / candidate_boxes);
      if (attrs > (4U + inferred_masks))
      {
        uint32_t candidate_classes = attrs - 4U - inferred_masks;
        if ((candidate_classes >= 1U) && (candidate_classes <= 80U))
        {
          best_boxes = candidate_boxes;
          best_classes = candidate_classes;
          break;
        }
      }
    }
  }

  if ((best_boxes == 0U) || (best_classes == 0U))
  {
    return 0;
  }

  *boxes = best_boxes;
  *classes = best_classes;
  *masks = inferred_masks;
  *mask_side = inferred_side;
  return 1;
}

static uint32_t App_AI_DecodeYoloSegDetections(const int8_t *det_data,
                                               size_t det_size,
                                               uint32_t boxes,
                                               uint32_t classes,
                                               uint32_t masks,
                                               AppAISegDetection_t *out,
                                               uint32_t out_cap)
{
  uint32_t b;
  uint32_t count = 0U;
  const int8_t conf_threshold_raw = 40;

  (void)det_size;

  if ((det_data == NULL) || (out == NULL) || (out_cap == 0U))
  {
    return 0U;
  }

  for (b = 0U; b < boxes; ++b)
  {
    uint32_t c;
    uint8_t best_class = 0U;
    int8_t best_conf = INT8_MIN;

    for (c = 0U; c < classes; ++c)
    {
      int8_t conf = det_data[(4U + c) * boxes + b];
      if (conf > best_conf)
      {
        best_conf = conf;
        best_class = (uint8_t)c;
      }
    }

    if (best_conf < conf_threshold_raw)
    {
      continue;
    }

    if (count < out_cap)
    {
      uint32_t m;
      out[count].x_center = App_AI_Clamp01(App_AI_Int8ToUnit(det_data[(0U * boxes) + b]));
      out[count].y_center = App_AI_Clamp01(App_AI_Int8ToUnit(det_data[(1U * boxes) + b]));
      out[count].width = App_AI_Clamp01(App_AI_Int8ToUnit(det_data[(2U * boxes) + b]));
      out[count].height = App_AI_Clamp01(App_AI_Int8ToUnit(det_data[(3U * boxes) + b]));
      out[count].class_index = best_class;
      out[count].confidence_raw = best_conf;

      for (m = 0U; m < masks; ++m)
      {
        out[count].mask_coeffs[m] = det_data[(4U + classes + m) * boxes + b];
      }
      ++count;
    }
  }

  return count;
}

static uint32_t App_AI_NmsDetections(AppAISegDetection_t *detections,
                                     uint32_t count,
                                     float iou_threshold)
{
  uint32_t i;
  uint32_t kept = 0U;

  for (i = 0U; i < count; ++i)
  {
    uint32_t j;
    uint8_t keep = 1U;
    for (j = 0U; j < kept; ++j)
    {
      if (detections[i].class_index == detections[j].class_index)
      {
        if (App_AI_IoU(&detections[i], &detections[j]) > iou_threshold)
        {
          keep = 0U;
          break;
        }
      }
    }

    if (keep != 0U)
    {
      if (kept != i)
      {
        detections[kept] = detections[i];
      }
      ++kept;
    }
  }

  return kept;
}

static void App_AI_DrawYoloSegOverlay(const int8_t *det_data,
                                      size_t det_size,
                                      const int8_t *mask_data,
                                      size_t mask_size)
{
  static const uint16_t palette[10] =
  {
    0xF800U, 0x07E0U, 0x001FU, 0xFFE0U, 0xF81FU,
    0x07FFU, 0xFD20U, 0xAFE5U, 0x87FFU, 0xFFFFU
  };
  AppAISegDetection_t detections[10];
  uint16_t *fb = App_AI_GetDisplayFramebuffer();
  uint32_t fb_w = App_AI_GetDisplayWidth();
  uint32_t fb_h = App_AI_GetDisplayHeight();
  uint32_t boxes = 0U;
  uint32_t classes = 0U;
  uint32_t masks = 0U;
  uint32_t mask_side = 0U;
  uint32_t det_count;
  uint32_t d;

  if ((fb == NULL) || (det_data == NULL) || (mask_data == NULL) || (fb_w == 0U) || (fb_h == 0U))
  {
    return;
  }

  if (!App_AI_InferYoloSegParams(det_size, mask_size, &boxes, &classes, &masks, &mask_side))
  {
    return;
  }

  det_count = App_AI_DecodeYoloSegDetections(det_data,
                                             det_size,
                                             boxes,
                                             classes,
                                             masks,
                                             detections,
                                             (uint32_t)(sizeof(detections) / sizeof(detections[0])));
  det_count = App_AI_NmsDetections(detections, det_count, 0.45f);

  for (d = 0U; d < det_count; ++d)
  {
    uint32_t mx;
    uint32_t my;
    float x0f = detections[d].x_center - 0.5f * detections[d].width;
    float y0f = detections[d].y_center - 0.5f * detections[d].height;
    float x1f = detections[d].x_center + 0.5f * detections[d].width;
    float y1f = detections[d].y_center + 0.5f * detections[d].height;
    uint32_t x0;
    uint32_t y0;
    uint32_t x1;
    uint32_t y1;
    uint16_t color = palette[detections[d].class_index % 10U];

    if (x0f < 0.0f)
    {
      x0f = 0.0f;
    }
    if (y0f < 0.0f)
    {
      y0f = 0.0f;
    }
    if (x1f > 1.0f)
    {
      x1f = 1.0f;
    }
    if (y1f > 1.0f)
    {
      y1f = 1.0f;
    }

    x0 = (uint32_t)(x0f * (float)fb_w);
    y0 = (uint32_t)(y0f * (float)fb_h);
    x1 = (uint32_t)(x1f * (float)fb_w);
    y1 = (uint32_t)(y1f * (float)fb_h);

    if ((x1 <= x0) || (y1 <= y0))
    {
      continue;
    }

    for (my = 0U; my < mask_side; my += 2U)
    {
      for (mx = 0U; mx < mask_side; mx += 2U)
      {
        int32_t sum = 0;
        uint32_t k;
        uint32_t proto_index;
        uint32_t px;
        uint32_t py;

        for (k = 0U; k < masks; ++k)
        {
          proto_index = ((my * mask_side) + mx) * masks + k;
          sum += (int32_t)detections[d].mask_coeffs[k] * (int32_t)mask_data[proto_index];
        }

        if (sum <= 0)
        {
          continue;
        }

        px = x0 + ((x1 - x0) * mx) / mask_side;
        py = y0 + ((y1 - y0) * my) / mask_side;
        if ((px < fb_w) && (py < fb_h))
        {
          fb[py * fb_w + px] = App_AI_ColorBlend565(fb[py * fb_w + px], color);
        }
      }
    }
  }
}

static uint32_t App_AI_IntegerSqrt(uint32_t value)
{
  uint32_t r = 0U;
  while ((r + 1U) * (r + 1U) <= value)
  {
    ++r;
  }
  return r;
}

static void App_AI_DrawPoint(uint16_t *fb,
                             uint32_t fb_w,
                             uint32_t fb_h,
                             uint32_t cx,
                             uint32_t cy,
                             uint32_t radius,
                             uint16_t color)
{
  uint32_t y;
  uint32_t x;

  if ((fb == NULL) || (fb_w == 0U) || (fb_h == 0U))
  {
    return;
  }

  for (y = (cy > radius) ? (cy - radius) : 0U;
       (y <= cy + radius) && (y < fb_h);
       ++y)
  {
    for (x = (cx > radius) ? (cx - radius) : 0U;
         (x <= cx + radius) && (x < fb_w);
         ++x)
    {
      int32_t dx = (int32_t)x - (int32_t)cx;
      int32_t dy = (int32_t)y - (int32_t)cy;
      if ((uint32_t)(dx * dx + dy * dy) <= (radius * radius))
      {
        fb[y * fb_w + x] = color;
      }
    }
  }
}

static void App_AI_DrawStatusMarker(void)
{
  uint16_t *fb = App_AI_GetDisplayFramebuffer();
  uint32_t fb_w = App_AI_GetDisplayWidth();
  uint32_t fb_h = App_AI_GetDisplayHeight();
  uint16_t mode_color;
  uint32_t x;
  uint32_t y;
  uint32_t heartbeat_on;

  if ((fb == NULL) || (fb_w < 24U) || (fb_h < 24U))
  {
    return;
  }

  mode_color = (App_AI_GetModel() == APP_AI_MODEL_SEGMENTATION) ? 0x07E0U : 0x001FU;
  heartbeat_on = (g_ai_run_counter & 0x08U) ? 1U : 0U;

  for (y = 0U; y < 18U; ++y)
  {
    for (x = 0U; x < 18U; ++x)
    {
      uint16_t c = mode_color;
      if ((x == 0U) || (y == 0U) || (x == 17U) || (y == 17U))
      {
        c = 0xFFFFU;
      }
      if ((x >= 6U) && (x <= 11U) && (y >= 6U) && (y <= 11U) && (heartbeat_on != 0U))
      {
        c = 0xF800U;
      }
      fb[y * fb_w + x] = c;
    }
  }
}

static void App_AI_DrawSegmentationMapOverlay(const uint8_t *seg_map, size_t seg_size)
{
  static const uint16_t palette[8] =
  {
    0x0000U, 0xF800U, 0x07E0U, 0x001FU, 0xFFE0U, 0xF81FU, 0x07FFU, 0xFFFFU
  };
  uint16_t *fb = App_AI_GetDisplayFramebuffer();
  uint32_t fb_w = App_AI_GetDisplayWidth();
  uint32_t fb_h = App_AI_GetDisplayHeight();
  uint32_t seg_w;
  uint32_t seg_h;
  uint32_t y;
  uint32_t x;

  if ((fb == NULL) || (seg_map == NULL) || (seg_size == 0U) || (fb_w == 0U) || (fb_h == 0U))
  {
    return;
  }

  seg_w = 160U;
  if ((seg_size % seg_w) != 0U)
  {
    seg_w = App_AI_IntegerSqrt((uint32_t)seg_size);
    if (seg_w == 0U)
    {
      return;
    }
  }

  seg_h = (uint32_t)(seg_size / seg_w);
  if (seg_h == 0U)
  {
    return;
  }

  for (y = 0U; y < fb_h; y += 2U)
  {
    uint32_t sy = (y * seg_h) / fb_h;
    for (x = 0U; x < fb_w; x += 2U)
    {
      uint32_t sx = (x * seg_w) / fb_w;
      uint32_t idx = sy * seg_w + sx;
      uint8_t cls;
      uint16_t color;

      if (idx >= seg_size)
      {
        continue;
      }

      cls = seg_map[idx];
      if (cls == 0U)
      {
        continue;
      }

      color = palette[cls & 0x07U];
      fb[y * fb_w + x] = App_AI_ColorBlend565(fb[y * fb_w + x], color);
    }
  }
}

static int App_AI_DrawPoseKeypointsYolo(const int8_t *pose_data, size_t pose_size)
{
  uint16_t *fb = App_AI_GetDisplayFramebuffer();
  uint32_t fb_w = App_AI_GetDisplayWidth();
  uint32_t fb_h = App_AI_GetDisplayHeight();
  static const uint16_t colors[17] =
  {
    0xF800U, 0xFBE0U, 0x07E0U, 0x07FFU, 0x001FU, 0xF81FU, 0xFFE0U, 0xFFFFU,
    0xFD20U, 0xAFE5U, 0xD81FU, 0x87FFU, 0xA145U, 0x5FE0U, 0x780FU, 0xBDF7U,
    0xEF5DU
  };
  static const uint32_t box_candidates[] = {8400U, 5376U, 1344U, 1008U, 768U};
  uint32_t i;
  uint32_t boxes = 0U;
  uint32_t keypoints = 0U;
  uint32_t attrs = 0U;
  uint32_t best_box = 0U;
  int8_t best_conf = INT8_MIN;

  if ((pose_data == NULL) || (fb == NULL) || (fb_w == 0U) || (fb_h == 0U))
  {
    return 0;
  }

  for (i = 0U; i < (sizeof(box_candidates) / sizeof(box_candidates[0])); ++i)
  {
    uint32_t candidate = box_candidates[i];
    if ((candidate != 0U) && ((pose_size % candidate) == 0U))
    {
      uint32_t candidate_attrs = (uint32_t)(pose_size / candidate);
      if ((candidate_attrs > 8U) && (((candidate_attrs - 5U) % 3U) == 0U))
      {
        uint32_t candidate_kps = (candidate_attrs - 5U) / 3U;
        if ((candidate_kps >= 4U) && (candidate_kps <= 32U))
        {
          boxes = candidate;
          attrs = candidate_attrs;
          keypoints = candidate_kps;
          break;
        }
      }
    }
  }

  if ((boxes == 0U) || (keypoints == 0U) || (attrs == 0U))
  {
    return 0;
  }

  for (i = 0U; i < boxes; ++i)
  {
    int8_t conf = pose_data[(4U * boxes) + i];
    if (conf > best_conf)
    {
      best_conf = conf;
      best_box = i;
    }
  }

  if (best_conf < 40)
  {
    return 0;
  }

  for (i = 0U; i < keypoints; ++i)
  {
    uint32_t k_off = 5U + 3U * i;
    int8_t x_raw;
    int8_t y_raw;
    int8_t c_raw;
    uint32_t x;
    uint32_t y;

    if ((k_off + 2U) >= attrs)
    {
      break;
    }

    x_raw = pose_data[(k_off + 0U) * boxes + best_box];
    y_raw = pose_data[(k_off + 1U) * boxes + best_box];
    c_raw = pose_data[(k_off + 2U) * boxes + best_box];

    if (c_raw < 20)
    {
      continue;
    }

    x = (uint32_t)(App_AI_Int8ToUnit(x_raw) * (float)fb_w);
    y = (uint32_t)(App_AI_Int8ToUnit(y_raw) * (float)fb_h);
    App_AI_DrawPoint(fb, fb_w, fb_h, x, y, 4U, colors[i % 17U]);
  }

  return 1;
}

static void App_AI_DrawPoseKeypointsHeatmap(const uint8_t *pose_data, size_t pose_size)
{
  static const uint16_t colors[17] =
  {
    0xF800U, 0xFBE0U, 0x07E0U, 0x07FFU, 0x001FU, 0xF81FU, 0xFFE0U, 0xFFFFU,
    0xFD20U, 0xAFE5U, 0xD81FU, 0x87FFU, 0xA145U, 0x5FE0U, 0x780FU, 0xBDF7U,
    0xEF5DU
  };
  const uint32_t keypoint_count = 17U;
  uint16_t *fb = App_AI_GetDisplayFramebuffer();
  uint32_t fb_w = App_AI_GetDisplayWidth();
  uint32_t fb_h = App_AI_GetDisplayHeight();
  size_t chunk_size;
  uint32_t k;

  if ((fb == NULL) || (pose_data == NULL) || (pose_size < keypoint_count) || (fb_w == 0U) || (fb_h == 0U))
  {
    return;
  }

  chunk_size = pose_size / keypoint_count;
  if (chunk_size == 0U)
  {
    return;
  }

  for (k = 0U; k < keypoint_count; ++k)
  {
    size_t start = (size_t)k * chunk_size;
    size_t end = (k == (keypoint_count - 1U)) ? pose_size : (start + chunk_size);
    size_t i;
    size_t best_idx = start;
    uint8_t best_val = 0U;
    uint32_t gw;
    uint32_t gh;
    uint32_t gx;
    uint32_t gy;
    uint32_t x;
    uint32_t y;

    for (i = start; i < end; ++i)
    {
      if (pose_data[i] >= best_val)
      {
        best_val = pose_data[i];
        best_idx = i;
      }
    }

    if (best_val < 64U)
    {
      continue;
    }

    gw = App_AI_IntegerSqrt((uint32_t)(end - start));
    if (gw == 0U)
    {
      continue;
    }
    gh = (uint32_t)((end - start) / gw);
    if (gh == 0U)
    {
      continue;
    }

    gx = (uint32_t)((best_idx - start) % gw);
    gy = (uint32_t)((best_idx - start) / gw);

    x = (gx * fb_w) / gw;
    y = (gy * fb_h) / gh;
    App_AI_DrawPoint(fb, fb_w, fb_h, x, y, 4U, colors[k]);
  }
}

/* 
 * Bootstrap
 */
int aiInit(void) {
    LL_ATON_RT_RuntimeInit();
    LL_ATON_RT_Init_Network(&NN_Instance_pose);
    LL_ATON_RT_Init_Network(&NN_Instance_seg);

    App_AI_InitModelCtx(&g_pose_ctx, &NN_Instance_pose, 1U);
    App_AI_InitModelCtx(&g_seg_ctx, &NN_Instance_seg, 2U);
    App_AI_RefreshActiveBuffers();

  return 0;
}

int aiDeinit(void) {
  LL_ATON_RT_DeInit_Network(&NN_Instance_pose);
  LL_ATON_RT_DeInit_Network(&NN_Instance_seg);
  LL_ATON_RT_RuntimeDeInit();
  return 0;
}

/* 
 * Run inference
 */
int aiRun() {
    AppAIModelCtx_t *ctx = App_AI_GetModelCtx((AppAIModel_t)g_active_model);
  uint32_t guard = 0U;
  const uint32_t guard_max = 2000000U;
  uint32_t wfe_guard = 0U;
  const uint32_t wfe_guard_max = 4000U;

    LL_ATON_RT_RetValues_t ll_aton_rt_ret = LL_ATON_RT_DONE;
    LL_ATON_RT_Reset_Network(ctx->instance);
    
    do {
      /* Execute first/next step */
      ll_aton_rt_ret = LL_ATON_RT_RunEpochBlock(ctx->instance);
      /* Wait for next event */
      if (ll_aton_rt_ret == LL_ATON_RT_WFE)
      {
#ifdef USE_THREADX_AI_RUNTIME
        /*
         * Prevent permanent thread lock if NPU IRQ is missing.
         * Yield 1 tick and keep polling until timeout.
         */
        tx_thread_sleep(1);
        ++wfe_guard;
        if (wfe_guard > wfe_guard_max)
        {
          g_ai_last_run_status = -2;
          return -2;
        }
#else
        LL_ATON_OSAL_WFE();
#endif
      }

      ++guard;
      if (guard > guard_max)
      {
        g_ai_last_run_status = -1;
        return -1;
      }
    } while (ll_aton_rt_ret != LL_ATON_RT_DONE);

    ++g_ai_run_counter;
    g_ai_last_run_status = 0;

    App_AI_RefreshActiveBuffers();
  return 0;
}

void App_AI_SetModel(AppAIModel_t model)
{
  if ((model == APP_AI_MODEL_POSE) || (model == APP_AI_MODEL_SEGMENTATION))
  {
    g_active_model = model;
    App_AI_RefreshActiveBuffers();
  }
}

AppAIModel_t App_AI_GetModel(void)
{
  return (AppAIModel_t)g_active_model;
}

uint8_t *App_AI_GetInputBuffer(void)
{
  AppAIModelCtx_t *ctx = App_AI_GetModelCtx((AppAIModel_t)g_active_model);
  return ctx->input;
}

const uint8_t *App_AI_GetOutputBuffer(uint32_t output_index)
{
  AppAIModelCtx_t *ctx = App_AI_GetModelCtx((AppAIModel_t)g_active_model);
  if (output_index >= ctx->output_count)
  {
    return NULL;
  }
  return ctx->outputs[output_index];
}

size_t App_AI_GetInputSize(void)
{
  AppAIModelCtx_t *ctx = App_AI_GetModelCtx((AppAIModel_t)g_active_model);
  return ctx->input_size;
}

size_t App_AI_GetOutputSize(uint32_t output_index)
{
  AppAIModelCtx_t *ctx = App_AI_GetModelCtx((AppAIModel_t)g_active_model);
  if (output_index >= ctx->output_count)
  {
    return 0U;
  }
  return ctx->output_sizes[output_index];
}

uint32_t App_AI_GetOutputCount(void)
{
  AppAIModelCtx_t *ctx = App_AI_GetModelCtx((AppAIModel_t)g_active_model);
  return ctx->output_count;
}

uint32_t App_AI_GetRunCounter(void)
{
  return g_ai_run_counter;
}

int32_t App_AI_GetLastRunStatus(void)
{
  return g_ai_last_run_status;
}

void App_AI_RenderActiveModelOverlay(void)
{
  App_AI_DrawStatusMarker();

  if (App_AI_GetModel() == APP_AI_MODEL_SEGMENTATION)
  {
    const int8_t *det_output = (const int8_t *)App_AI_GetOutputBuffer(0U);
    size_t det_size = App_AI_GetOutputSize(0U);
    const int8_t *mask_output = (const int8_t *)App_AI_GetOutputBuffer(1U);
    size_t mask_size = App_AI_GetOutputSize(1U);

    if ((det_output != NULL) && (mask_output != NULL) && (det_size != 0U) && (mask_size != 0U))
    {
      App_AI_DrawYoloSegOverlay(det_output, det_size, mask_output, mask_size);
      return;
    }

    App_AI_DrawSegmentationMapOverlay((const uint8_t *)App_AI_GetOutputBuffer(0U),
                                      App_AI_GetOutputSize(0U));
  }
  else
  {
    const int8_t *pose_out = (const int8_t *)App_AI_GetOutputBuffer(0U);
    size_t pose_size = App_AI_GetOutputSize(0U);
    if (!App_AI_DrawPoseKeypointsYolo(pose_out, pose_size))
    {
      App_AI_DrawPoseKeypointsHeatmap((const uint8_t *)pose_out, pose_size);
    }
  }
}


int acquire_and_process_data()
{
  /* fill the inputs of the c-model 
  for (int idx=0; idx < STAI_NETWORK_IN_NUM; idx++ )
  {
      stai_input[idx] = ....
  }

  */
  return 0;
}

int post_process()
{
  /* process the predictions
  for (int idx=0; idx < STAI_NETWORK_OUT_NUM; idx++ )
  {
      stai_output[idx] = ....
  }

  */
  return 0;
}



/*
 * Run one inference step per call.
 * Keeping this non-blocking avoids hijacking existing ThreadX/camera/display loops.
 */
void main_loop() {
  /* 1 - Acquire, pre-process and fill the input buffers */
  acquire_and_process_data();

  /* 2 - Call inference engine */
  aiRun();

  /* 3 - Post-process the predictions */
  post_process();
}


/* Entry points --------------------------------------------------------------*/



void STM32CubeAI_Studio_AI_Init(void)
{
    aiPreInitialize();
    /* USER CODE BEGIN 5 */
    aiInit();
    /* USER CODE END 5 */
}

void STM32CubeAI_Studio_AI_Process(void)
{
    main_loop();
} 

void STM32CubeAI_Studio_AI_Deinit(void)
{
    aiDeinit();
} 


#ifdef __cplusplus
}
#endif
