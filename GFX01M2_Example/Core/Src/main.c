// main.c
#include "stm32f0xx.h"
#include "gfx01m2.h"
#include <string.h>


#define WHITE 0xFFFF
#define BLUE  0x001F
#define RED   0xF800

int main(void) {
    LCD_Init();
    LCD_Clear(BLUE);
    LCD_DrawString("STM32F091RC LCD Test", 10, 10, WHITE, BLUE);

    uint16_t x = 60, y = 60;
    LCD_DrawString("Joystick!", x, y, RED, BLUE);

    while (1) {
        uint8_t joy = Joystick_Read();
        LCD_DrawString("Joystick!", x, y, BLUE, BLUE); // Erase
        if (joy & JOY_UP) y -= 8;
        if (joy & JOY_DOWN) y += 8;
        if (joy & JOY_LEFT) x -= 8;
        if (joy & JOY_RIGHT) x += 8;
        LCD_DrawString("Joystick!", x, y, RED, BLUE);
        for (volatile int i = 0; i < 100000; i++); // crude delay
    }
}

/*
int main(void) {
    LCD_Init();
    LCD_ClearScreen(0x0000);  // Black background

    LCD_DrawText(10, 10, "Hello, World!", 0xFFFF, 0x0000);  // White text

    while (1) {
        uint8_t joy = Joystick_GetState();
        if (joy & JOY_UP) {
            LCD_DrawText(10, 30, "UP pressed  ", 0xF800, 0x0000);
        } else if (joy & JOY_DOWN) {
            LCD_DrawText(10, 30, "DOWN pressed", 0x07E0, 0x0000);
        } else if (joy & JOY_LEFT) {
            LCD_DrawText(10, 30, "LEFT pressed", 0x001F, 0x0000);
        } else if (joy & JOY_RIGHT) {
            LCD_DrawText(10, 30, "RIGHT pressed", 0xFFE0, 0x0000);
        } else if (joy & JOY_CENTER) {
            LCD_DrawText(10, 30, "CENTER pressed", 0xF81F, 0x0000);
        } else {
            LCD_DrawText(10, 30, "              ", 0x0000, 0x0000);  // Clear line
        }
    }
}
*/
