#include "stm32f0xx.h"

void init_usart1_rx() {
    // Enable GPIOA and USART1 clocks
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // Configure PA10 as USART1_RX (Alternate Function)
    GPIOA->MODER &= ~(GPIO_MODER_MODER10);       // Clear mode bits
    GPIOA->MODER |= (GPIO_MODER_MODER10_1);      // Set to Alternate Function
    GPIOA->AFR[1] |= (1 << (4 * (10 - 8)));      // Set AF1 for PA10

    // Configure USART1
    USART1->CR1 &= ~USART_CR1_UE;                // Disable USART for configuration
    USART1->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0); // 8 data bits
    USART1->CR2 &= ~USART_CR2_STOP;              // 1 stop bit
    USART1->CR1 &= ~USART_CR1_PCE;               // No parity
    USART1->CR1 &= ~USART_CR1_OVER8;             // 16x oversampling

    // Set baud rate (assuming 48 MHz clock)
    USART1->BRR = 48000000 / 115200;             // Set baud rate to 115200

    // Enable receiver
    USART1->CR1 |= USART_CR1_RE;

    // Enable USART1
    USART1->CR1 |= USART_CR1_UE;

    // Wait for receiver readiness
    while (!(USART1->ISR & USART_ISR_REACK));
}

char usart1_receive_char() {
    while (!(USART1->ISR & USART_ISR_RXNE));  // Wait until RX buffer is not empty
    return USART1->RDR;                      // Read received character
}

void usart1_receive_string(char *buffer, int max_length) {
    int i = 0;
    while (i < max_length - 1) {
        char c = usart1_receive_char();
        if (c == '\r' || c == '\n') break;  // Stop at newline or carriage return
        buffer[i++] = c;
    }
    buffer[i] = '\0';  // Null-terminate the string
}

int main(void) {
    init_usart1_rx();
    char buffer[100];

    while (1) {
        usart1_receive_string(buffer, sizeof(buffer));
        //NOW WRITE LOGIC TO PUT IT ON TFT 
    }
}
