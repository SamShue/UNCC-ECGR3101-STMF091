// Simple blinky for STM32F091RC (Nucleo) - GPIOA Pin 5
#define RCC_BASE     0x40021000
#define GPIOA_BASE   0x48000000

#define RCC_AHBENR   (*(volatile unsigned int*)(RCC_BASE + 0x14))
#define GPIOA_MODER  (*(volatile unsigned int*)(GPIOA_BASE + 0x00))
#define GPIOA_ODR    (*(volatile unsigned int*)(GPIOA_BASE + 0x14))

#define LED_PIN      5

void delay(volatile unsigned int time) {
    while (time--) {
        for (volatile int i = 0; i < 1000; i++);
    }
}

int main(void) {
    // Enable clock to GPIOA
    RCC_AHBENR |= (1 << 17);  // Bit 17 = IOPAEN

    // Set GPIOA pin 5 to general-purpose output mode
    // LED is on Port A pin 5
    GPIOA_MODER |=  (01 << LED_PIN * 2); // Set to output mode (01)

    // Toggle LED
    while (1) {
        GPIOA_ODR ^= (1 << LED_PIN); // Toggle pin
        delay(1000);
    }

    return 0;
}
