#ifndef LCD_H
#define LCD_H

#include <stdint.h>

// --- Framebuffer-Konfiguration für 800x480 RGB565 ---
#define LCD_WIDTH 800
#define LCD_HEIGHT 480
#define LCD_PIXEL_FORMAT_RGB565 2 // Bytes pro Pixel

// Framebuffer-Adresse im externen RAM (OctaSPI), nach APPLI-Bereich
#define LCD_FRAMEBUFFER_ADDR 0x70180000

// Beispiel: Framebuffer als Zeiger (wird von LTDC verwendet)
// (Buffer wird im externen RAM angelegt, keine Variable im internen RAM!)
static inline uint16_t *LCD_GetFramebuffer(void) {
	return (uint16_t *)LCD_FRAMEBUFFER_ADDR;
}

// Beispiel für Layer-Konfiguration (in main.c oder lcd.c):
//
// LTDC_LayerCfgTypeDef pLayerCfg = {0};
// pLayerCfg.WindowX0 = 0;
// pLayerCfg.WindowX1 = LCD_WIDTH;
// pLayerCfg.WindowY0 = 0;
// pLayerCfg.WindowY1 = LCD_HEIGHT;
// pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
// pLayerCfg.FBStartAdress = LCD_FRAMEBUFFER_ADDR;
// pLayerCfg.ImageWidth = LCD_WIDTH;
// pLayerCfg.ImageHeight = LCD_HEIGHT;
// ...
// HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0);

void LCD_Init(void);
void LCD_Clear(uint32_t color);
void LCD_DrawString(uint16_t x, uint16_t y, const char *str, uint32_t color);

#endif // LCD_H
