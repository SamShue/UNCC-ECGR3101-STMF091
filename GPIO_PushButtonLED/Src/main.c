#define RCC_BASE      0x40021000
#define GPIOA_BASE    0x48000000
#define GPIOC_BASE    0x48000800

#define RCC_AHBENR    (*(volatile unsigned int*)(RCC_BASE + 0x14))
#define GPIOA_MODER   (*(volatile unsigned int*)(GPIOA_BASE + 0x00))
#define GPIOA_ODR     (*(volatile unsigned int*)(GPIOA_BASE + 0x14))

#define GPIOC_MODER   (*(volatile unsigned int*)(GPIOC_BASE + 0x00))
#define GPIOC_IDR     (*(volatile unsigned int*)(GPIOC_BASE + 0x10))

#define LED_PIN       5     // GPIOA5
#define BUTTON_PIN    13    // GPIOC13

void delay(volatile unsigned int time) {
    while (time--) {
        for (volatile int i = 0; i < 1000; i++);
    }
}

int main(void) {
    // Enable clock for GPIOA and GPIOC
    RCC_AHBENR |= (1 << 17); // IOPAEN
    RCC_AHBENR |= (1 << 19); // IOPCEN

    // Configure PA5 as output (LED)
    GPIOA_MODER &= ~(3 << (LED_PIN * 2)); // Clear mode
    GPIOA_MODER |=  (1 << (LED_PIN * 2)); // Set to output (01)

    // Configure PC13 as input (Button)
    GPIOC_MODER &= ~(3 << (BUTTON_PIN * 2)); // Set to input (00)

    int button_prev = 1;  // Not pressed (initially high)

    while (1) {
        int button_curr = (GPIOC_IDR >> BUTTON_PIN) & 0x1;

        if (button_prev == 1 && button_curr == 0) {
            // Detected falling edge (button press)
            GPIOA_ODR ^= (1 << LED_PIN); // Toggle LED
            delay(300); // Debounce
        }

        button_prev = button_curr;
    }

    return 0;
}
