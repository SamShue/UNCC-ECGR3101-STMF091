#ifndef NEOPIXEL_H
#define NEOPIXEL_H

/*
 * neopixel.h - WS2812B NeoPixel driver for STM32F091RC
 *
 * Uses TIM2 PWM + DMA1 (no CPU bit-banging):
 *   - TIM2_CH1 (PA0, AF2) generates the PWM waveform at 800 kHz
 *   - DMA1_Channel2 (TIM2_UP request) feeds a pre-built CCR table
 *     into TIM2->CCR1 each period, setting the duty cycle per bit
 *   - neopixel_show() blocks until the DMA transfer completes
 *
 * Pin Connections (STM32F091RC Nucleo Morpho Connector):
 *
 *  NeoPixel Ring | Nucleo Morpho | STM32
 *  ------------- | ------------- | ------
 *  GND           | CN7-20        | GND
 *  5V            | CN7-18        | 5V
 *  DIN           | CN10-3        | PA0  (TIM2_CH1, AF2)
 *
 * Requires 48 MHz SYSCLK — call clock_init() in main.c before neopixel_init().
 *
 * At 48 MHz, TIM2 period = 60 counts = 1.25 µs (800 kHz):
 *   T1H = 38 counts = 792 ns   T1L = 22 counts = 458 ns
 *   T0H = 19 counts = 396 ns   T0L = 41 counts = 854 ns
 *   RESET: CCR = 0 (0% duty) held for >= 40 periods > 50 µs
 *
 * Usage:
 *   clock_init();
 *   neopixel_init(16);
 *   neopixel_set(0, 255, 0, 0, 128);  // LED 0 = red, 50% brightness
 *   neopixel_clear(3);                // LED 3 off
 *   neopixel_show();                  // transmit (blocks until done)
 */

#include "stm32f0xx.h"
#include <stdint.h>
#include <string.h>

#define NEOPIXEL_MAX_LEDS  64

/* Timer constants at 48 MHz → 800 kHz */
#define _NP_ARR     59   /* ARR: 60 counts = 1.25 µs period */
#define _NP_T1      38   /* CCR for a '1' bit: 792 ns high  */
#define _NP_T0      19   /* CCR for a '0' bit: 396 ns high  */
#define _NP_RESET   50   /* Number of zero-CCR periods for reset (> 50 µs) */

/* ------------------------------------------------------------------ */
/*  Private state                                                       */
/* ------------------------------------------------------------------ */

static uint8_t _np_count;
static uint8_t _np_buf[NEOPIXEL_MAX_LEDS][3];                         /* [R,G,B] frame buffer  */
static uint8_t _np_dma[NEOPIXEL_MAX_LEDS * 24 + _NP_RESET];           /* CCR value per bit     */

/* Build the DMA transfer table from the frame buffer.
 * Each entry is the CCR1 value (_NP_T1 or _NP_T0) for one WS2812B bit.
 * Trailing zeros produce 0% duty (line low) for the reset pulse. */
static inline void _np_build(void) {
    uint32_t idx = 0;
    for (int i = 0; i < _np_count; i++) {
        /* WS2812B wire order: G, R, B */
        uint8_t bytes[3] = { _np_buf[i][1], _np_buf[i][0], _np_buf[i][2] };
        for (int b = 0; b < 3; b++) {
            for (int bit = 7; bit >= 0; bit--) {
                _np_dma[idx++] = (bytes[b] >> bit) & 1 ? _NP_T1 : _NP_T0;
            }
        }
    }
    memset(&_np_dma[idx], 0, _NP_RESET);
}

/* ------------------------------------------------------------------ */
/*  Public API                                                          */
/* ------------------------------------------------------------------ */

/*
 * neopixel_init - configure TIM2_CH1 (PA0) and DMA1_Channel2.
 *   count : number of LEDs (1 .. NEOPIXEL_MAX_LEDS)
 * Call clock_init() first to ensure 48 MHz SYSCLK.
 */
static inline void neopixel_init(uint8_t count) {
    _np_count = (count > NEOPIXEL_MAX_LEDS) ? NEOPIXEL_MAX_LEDS : count;
    memset(_np_buf, 0, sizeof(_np_buf));

    /* Enable peripheral clocks */
    RCC->AHBENR  |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_DMA1EN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* PA0: AF2 (TIM2_CH1), push-pull, high-speed, no pull */
    GPIOA->MODER   = (GPIOA->MODER  & ~(3U << 0)) | (2U << 0);
    GPIOA->OTYPER &= ~(1U << 0);
    GPIOA->OSPEEDR |=  (3U << 0);
    GPIOA->PUPDR   &= ~(3U << 0);
    GPIOA->AFR[0]  = (GPIOA->AFR[0] & ~(0xFU << 0)) | (2U << 0);  /* AF2 */

    /* TIM2: up-counting PWM at 800 kHz, no prescaler */
    TIM2->CR1   = 0;
    TIM2->PSC   = 0;
    TIM2->ARR   = _NP_ARR;
    TIM2->CCR1  = 0;
    TIM2->CCMR1 = (6U << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;  /* PWM mode 1, preload */
    TIM2->CCER  = TIM_CCER_CC1E;
    TIM2->CR1  |= TIM_CR1_ARPE;
    TIM2->DIER  = TIM_DIER_UDE;   /* DMA request on update event */
    TIM2->EGR   = TIM_EGR_UG;     /* Load PSC/ARR immediately */

    /* DMA1_Channel2: TIM2_UP → TIM2->CCR1
     * Memory: 8-bit (CCR table bytes)  Peripheral: 32-bit (CCR1 register) */
    DMA1_Channel2->CCR  = 0;
    DMA1_Channel2->CPAR = (uint32_t)&TIM2->CCR1;
    DMA1_Channel2->CCR  =  DMA_CCR_DIR               /* mem → periph */
                         | DMA_CCR_MINC               /* increment memory */
                         | (0U << DMA_CCR_MSIZE_Pos)  /* memory  8-bit  */
                         | (2U << DMA_CCR_PSIZE_Pos); /* periph 32-bit  */
}

/*
 * neopixel_set - write a color into the frame buffer.
 *   index      : 0-based LED position
 *   r, g, b    : 8-bit color components
 *   brightness : 0 (off) .. 255 (full)
 */
static inline void neopixel_set(uint8_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    if (index >= _np_count) return;
    _np_buf[index][0] = (uint8_t)(((uint16_t)r * brightness) / 255);
    _np_buf[index][1] = (uint8_t)(((uint16_t)g * brightness) / 255);
    _np_buf[index][2] = (uint8_t)(((uint16_t)b * brightness) / 255);
}

/*
 * neopixel_clear - turn off a single LED in the frame buffer.
 *   index : 0-based LED position
 */
static inline void neopixel_clear(uint8_t index) {
    if (index >= _np_count) return;
    _np_buf[index][0] = 0;
    _np_buf[index][1] = 0;
    _np_buf[index][2] = 0;
}

/*
 * neopixel_show - transmit the frame buffer to the ring.
 * Blocks until all bits (including the reset pulse) have been sent.
 */
static inline void neopixel_show(void) {
    _np_build();

    uint32_t total = (uint32_t)(_np_count * 24 + _NP_RESET);

    /* Reset and re-arm DMA */
    DMA1_Channel2->CCR  &= ~DMA_CCR_EN;
    DMA1->IFCR           =  DMA_IFCR_CGIF2;  /* clear all channel-2 flags */
    DMA1_Channel2->CMAR  =  (uint32_t)_np_dma;
    DMA1_Channel2->CNDTR =  total;
    DMA1_Channel2->CCR  |=  DMA_CCR_EN;

    /* Start timer */
    TIM2->CR1 |= TIM_CR1_CEN;

    /* Poll for transfer complete */
    while (!(DMA1->ISR & DMA_ISR_TCIF2));

    /* Stop */
    DMA1_Channel2->CCR &= ~DMA_CCR_EN;
    TIM2->CR1          &= ~TIM_CR1_CEN;
    TIM2->CCR1          =  0;
    GPIOA->BRR          =  (1U << 0);  /* hold line low */
}

#endif /* NEOPIXEL_H */
