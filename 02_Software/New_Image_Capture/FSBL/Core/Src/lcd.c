
#include "lcd.h"
#include <string.h>

static uint32_t * const framebuffer = (uint32_t *)LCD_FRAMEBUFFER_ADDR;

// Simple 8x8 font (ASCII 32-127)
static const uint8_t font8x8_basic[96][8] = {
    // Only a few characters for demo, fill out as needed
    // 'H' (72)
    {0x00,0x66,0x66,0x7E,0x66,0x66,0x66,0x00},
    // 'a' (97)
    {0x00,0x00,0x3C,0x06,0x3E,0x66,0x3E,0x00},
    // 'l' (108)
    {0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x00},
    // 'o' (111)
    {0x00,0x00,0x3C,0x66,0x66,0x66,0x3C,0x00},
    // '!' (33)
    {0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00},
    // ' ' (32)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
};

void LCD_Init(void) {
    // Framebuffer is set in LTDC layer config, nothing to do here for now
}

void LCD_Clear(uint32_t color) {
    for (uint32_t y = 0; y < LCD_HEIGHT; ++y) {
        for (uint32_t x = 0; x < LCD_WIDTH; ++x) {
            framebuffer[y * LCD_WIDTH + x] = color;
        }
    }
}

static void LCD_DrawChar(uint16_t x, uint16_t y, char c, uint32_t color) {
    int idx = 0;
    if (c == 'H') idx = 0;
    else if (c == 'a') idx = 1;
    else if (c == 'l') idx = 2;
    else if (c == 'o') idx = 3;
    else if (c == '!') idx = 4;
    else idx = 5; // space or unsupported
    for (uint8_t row = 0; row < 8; ++row) {
        uint8_t bits = font8x8_basic[idx][row];
        for (uint8_t col = 0; col < 8; ++col) {
            if (bits & (1 << (7 - col))) {
                uint32_t px = x + col;
                uint32_t py = y + row;
                if (px < LCD_WIDTH && py < LCD_HEIGHT)
                    framebuffer[py * LCD_WIDTH + px] = color;
            }
        }
    }
}

void LCD_DrawString(uint16_t x, uint16_t y, const char *str, uint32_t color) {
    while (*str) {
        LCD_DrawChar(x, y, *str, color);
        x += 8;
        ++str;
    }
}
