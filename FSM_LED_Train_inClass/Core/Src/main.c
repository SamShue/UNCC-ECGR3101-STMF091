#include "stm32f0xx.h"
#include <stdint.h>
#include "neopixel.h"
#include <stdbool.h>

typedef enum {STOP, REVERSE, FORWARD} state_t;

#define CPU_HZ 8000000

void delay_ms(int ms)
{
    volatile int i, j;

    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < (CPU_HZ / 1000 / 4); j++)
        {
            ;
        }
    }
}


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

static inline bool isButtonPressed(void){
    // PC13 in IDR (active-low)
    return (GPIOC->IDR & (1U << 13)) == 0U;
}

void button_init(void){
    // Enable GPIOC (bit 19) clock
    RCC->AHBENR |= (1U << 19);   // GPIOCEN

    // ---- PC13 as input with pull-up ----
    // MODER13 bits [27:26] = 00 (input)
    GPIOC->MODER &= ~(0x3U << 26);

    // PUPDR13 bits [27:26] = 01 (pull-up)
    GPIOC->PUPDR &= ~(0x3U << 26);
    GPIOC->PUPDR |=  (0x1U << 26);
}

void drawTrain(uint32_t position){

	neopixel_set((postion - 1) % 16, 255, 0, 0, 0);
	neopixel_set(position, 255, 0, 0, 25);
	neopixel_set((position + 1) % 16, 255, 0, 0, 25);
	neopixel_set((position + 2) % 16, 255, 0, 0, 25);
	neopixel_set((position + 3) % 16, 255, 0, 0, 0);

	neopixel_show();
}

typedef struct {
	bool buttonPress;
	state_t lastMovingState;
	uint32_t trainPosition;
} inputs_t;

state_t runStopState(inputs_t inputs){
	drawTrain(position);

	if(buttonPressed && lastMovingState == REVERSE)
		nextState = FORWARD;
	else if(buttonPressed && lastMovingState == FORWARD)
		nextState = REVERSE;
	else
		nextState = STOP;
	break;
}

state_t runStopState(inputs_t inputs){

}

state_t runStopState(inputs_t inputs){

}

int main(void) {
    clock_init();
    button_init();
    neopixel_init(16);

    uint32_t position = 0;
    state_t currentState = STOP;
    state_t nextState = STOP;

    state_t lastMovingState = REVERSE;


    while (1) {

    	switch(currentState){
    	case STOP:
    		nextState = runStopState(isButtonPressed());
    	case REVERSE:
    		lastMovingState = REVERSE;

    		if(position == 0){
    			position = 15;
    		} else {
    			position--;
    		}

    		drawTrain(position);
    		delay_ms(500);

    		if(isButtonPressed()){
    			nextState = STOP;
    		} else {
    			nextState = REVERSE;
    		}
    		break;
    	case FORWARD:
    		lastMovingState = FORWARD;

			if(position == 15){
				position = 0;
			} else {
				position++;
			}

			drawTrain(position);
			delay_ms(500);

			if(isButtonPressed()){
				nextState = STOP;
			} else {
				nextState = FORWARD;
			}
			break;

    		break;
    	default:
    		nextState = STOP;
    	}

    	currentState = nextState;


    }
}
