#include "stm32f0xx.h"
#include <stdint.h>
#include "neopixel.h"

/*
 * clock_init - configure SYSCLK to 48 MHz.
 * HSI (8 MHz) / PREDIV(2) = 4 MHz, PLL x12 = 48 MHz.
 * Must be called before neopixel_init().
 */
static void clock_init(void) {
    /* 1 wait state required for SYSCLK > 24 MHz */
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | FLASH_ACR_LATENCY;

    /* HSI on and ready */
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));

    /* Switch to HSI before touching PLL */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);

    /* Disable PLL */
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY);

    /* PLL source: HSI/2 = 4 MHz, multiplier x12 = 48 MHz */
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL))
              | RCC_CFGR_PLLMUL12;   /* PLLSRC=0 → HSI/2 */

    /* Enable PLL and wait for lock */
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    /* Switch SYSCLK to PLL */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}

int main(void) {
    clock_init();
    neopixel_init(16);

    neopixel_set(1, 255, 0, 0, 25); /* red  at 50% */
    neopixel_show();

    while (1) {}
}
