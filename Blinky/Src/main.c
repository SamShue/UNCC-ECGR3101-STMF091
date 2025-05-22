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
    // 1. Enable clock to GPIOA
    RCC_AHBENR |= (1 << 17);  // Bit 17 = IOPAEN

    // 2. Set GPIOA pin 5 to general-purpose output mode
    GPIOA_MODER &= ~(3 << (LED_PIN * 2)); // Clear mode bits
    GPIOA_MODER |=  (1 << (LED_PIN * 2)); // Set to output mode (01)

    // 3. Toggle LED
    while (1) {
        GPIOA_ODR ^= (1 << LED_PIN); // Toggle pin
        delay(1000);
    }

    return 0;
}
