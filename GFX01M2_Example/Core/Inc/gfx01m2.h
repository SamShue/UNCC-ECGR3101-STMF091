#ifndef GFX01M2_H
#define GFX01M2_H

#include <stdint.h>

#define LCD_WIDTH  240
#define LCD_HEIGHT 320

#define JOY_RIGHT  0x01
#define JOY_LEFT   0x02
#define JOY_UP     0x04
#define JOY_DOWN   0x08
#define JOY_CENTER 0x10

void LCD_Init(void);
void LCD_Clear(uint16_t color);
void LCD_DrawChar(char c, uint16_t x, uint16_t y, uint16_t fg, uint16_t bg);
void LCD_DrawString(const char *str, uint16_t x, uint16_t y, uint16_t fg, uint16_t bg);
void LCD_DrawImage(const uint16_t *img, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
uint8_t Joystick_Read(void);

#endif
