/**
  ******************************************************************************
  * @file    main.c
  * @author  Weili An, Niraj Menon
  * @date    Feb 7, 2024
  * @brief   ECE 362 Lab 7 student template
  ******************************************************************************
*/

/*******************************************************************************/

// Fill out your username!  Even though we're not using an autotest, 
// it should be a habit to fill out your username in this field now.
const char* username = "kanchan";

/*******************************************************************************/ 

#include "stm32f0xx.h"
#include "commands.h"
#include <stdint.h>

void internal_clock();

// Uncomment only one of the following to test each step
// #define STEP1
// #define STEP2
// #define STEP3
#define STEP4


void init_usart5() {
    // TODO
    RCC->AHBENR |= (RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN);
    RCC->APB1ENR |= RCC_APB1ENR_USART5EN;

    // AFR FOR PC12 FOR USART5
    GPIOC->MODER &= ~(GPIO_MODER_MODER12); // Clear mode bits
    GPIOC->MODER |= ( GPIO_MODER_MODER12_1); //2'b10 left shifter 12

    // Setring afr to USART5_TX (AF14 for PC12)
    GPIOC->AFR[1] |= 0x20000; //'2b10 left shifted

    // AFR FOR PD2 FOR USART5
    GPIOD->MODER &= ~(GPIO_MODER_MODER2); // Clear mode bits
    GPIOD->MODER |= ( GPIO_MODER_MODER2_1); // 2'b10 right shfited 12

    // afr to USART5_RX (AF14 for PD2)
    GPIOD->AFR[0] |= 0x200; 

    // Configure USART5
    USART5->CR1 &= ~USART_CR1_UE; // Disable USART5

    //  SetTING word size to 8 b 
    USART5->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0) ; 

    // SetTING for 1 stop bit 
    USART5->CR2 &= ~USART_CR2_STOP;

    // Set no parity control (clear PCE bit in CR1)
    USART5->CR1 &= ~USART_CR1_PCE;

    //16X OVER SAMPLING ?? 
    USART5->CR1 &= ~ USART_CR1_OVER8; 

    //  SetTING baud rate to 115200
    // Asuming system clock is 48 MHz
    USART5->BRR = 0x1A1; // 48000000 / 115200; 

    // EnablING the transmitter and the receiver
    USART5->CR1 |= (USART_CR1_TE | USART_CR1_RE);

    // EnablING the USART
    USART5->CR1 |= USART_CR1_UE;

    // Wait for TE and RE to be acknowledged
    while (!(USART5->ISR & USART_ISR_TEACK)); // Wait until TEACK is set
    while (!(USART5->ISR & USART_ISR_REACK)); // Wait until REACK is set
}

#ifdef STEP1
int main(void){
    internal_clock();
    init_usart5();
    for(;;) {
        while (!(USART5->ISR & USART_ISR_RXNE)) { }
        char c = USART5->RDR;
        while(!(USART5->ISR & USART_ISR_TXE)) { }
        USART5->TDR = c;
    }
}
#endif

#ifdef STEP2
#include <stdio.h>

// TODO Resolve the echo and carriage-return problem

int __io_putchar(int c) {
    // TODO

    if (c == '\n'){
        while(!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
        // return c;

    }

    while(!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = c;
    return c;
    
}

int __io_getchar(void) {
    while (!(USART5->ISR & USART_ISR_RXNE));
    char c = USART5->RDR;
    // TODO

    if (c == '\r') {
        c = '\n';
    }
    __io_putchar(c); 
    return c;
}

int main() {
    internal_clock();
    init_usart5();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

#ifdef STEP3
#include <stdio.h>
#include "fifo.h"
#include "tty.h"
int __io_putchar(int c) {
    // TODO Copy from your STEP2
    if (c == '\n'){
        while(!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    

    }

    while(!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    // TODO
    return line_buffer_getchar(); 
}

int main() {
    internal_clock();
    init_usart5();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

#ifdef STEP4

#include <stdio.h>
#include "fifo.h"
#include "tty.h"

// TODO DMA data structures
#define FIFOSIZE 16
char serfifo[FIFOSIZE];
int seroffset = 0;

void enable_tty_interrupt(void) {
    // TODO
    USART5->CR1 |= USART_CR1_RXNEIE;
    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
     NVIC->ISER[0] |= (1<<USART3_8_IRQn) ;  

    DMA2->CSELR |= DMA2_CSELR_CH2_USART5_RX;
    DMA2_Channel2->CCR &= ~DMA_CCR_EN;

    USART5->CR3 |= USART_CR3_DMAR;
    DMA2_Channel2->CMAR = serfifo;
    DMA2_Channel2->CPAR = &USART5->RDR;  //address of the register 
    DMA2_Channel2->CNDTR = FIFOSIZE;  
    // DMA2_Channel2->CCR &= ~(DMA_CCR_DIR);  // Rst the CCR register
    DMA2_Channel2->CCR |= (DMA_CCR_CIRC |  DMA_CCR_MINC | DMA_CCR_PL);

    // DMA2_Channel2->CCR &= ~(DMA_CCR_MEM2MEM);                  
                   

    
    DMA2_Channel2->CCR |= DMA_CCR_EN;


   
}

// Works like line_buffer_getchar(), but does not check or clear ORE nor wait on new characters in USART
char interrupt_getchar() {
    // TODO
      USART_TypeDef *u = USART5;
    // If we missed reading some characters, clear the overrun flag.

    // Wait for a newline to complete the buffer.
    while(fifo_newline(&input_fifo) == 0) {
        // while (!(u->ISR & USART_ISR_RXNE))
        //     ;
        // insert_echo_char(u->RDR);
        asm volatile ("wfi"); // wait for an interrupt
    }
    // Return a character from the line buffer.
    char ch = fifo_remove(&input_fifo);
    return ch;
}

int __io_putchar(int c) {
    // TODO copy from STEP2
    if (c == '\n'){
        while(!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
        // return c;

    }

    while(!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    // TODO Use interrupt_getchar() instead of line_buffer_getchar()
    return interrupt_getchar(); 
}

// TODO Copy the content for the USART5 ISR here
// TODO Remember to look up for the proper name of the ISR function
void USART3_8_IRQHandler(void) {
    while(DMA2_Channel2->CNDTR != sizeof serfifo - seroffset) {
        if (!fifo_full(&input_fifo))
            insert_echo_char(serfifo[seroffset]);
        seroffset = (seroffset + 1) % sizeof serfifo;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void init_spi1_slow(void) {
    // Enable the clock for GPIOB and SPI1
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;   // Enable GPIOB clock
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;    // Enable SPI1 clock

    // Configure GPIOB pins: PB3 (SCK), PB4 (MISO), PB5 (MOSI)
    GPIOB->MODER &= ~(GPIO_MODER_MODER0_Msk | GPIO_MODER_MODER4_Msk | GPIO_MODER_MODER5_Msk);  // Clear MODER bits //FIX THIS 
    GPIOB->MODER |= (GPIO_MODER_MODER3_1 | GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1);   // Set PB3, PB4, PB5 to alternate function mode

    // Configure GPIOB alternate functions for SPI1
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL3_Msk | GPIO_AFRL_AFSEL4_Msk | GPIO_AFRL_AFSEL5_Msk);  // Clear AFR bits
    GPIOB->AFR[0] |= (5 << GPIO_AFRL_AFSEL3_Pos) | (5 << GPIO_AFRL_AFSEL4_Pos) | (5 << GPIO_AFRL_AFSEL5_Pos);  // Set AF5 (SPI1) for SCK, MISO, MOSI

    // Configure SPI1 settings
    SPI1->CR1 = 0;   // Reset all SPI1 settings

    // Set SPI1 in master mode, with 8-bit data size
    SPI1->CR1 |= SPI_CR1_MSTR | (7<<8);    // Master mode, 8-bit data size
    SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;     // Enable software slave management and internal slave select

    // Set the baud rate divisor to the maximum value for the slowest baud rate
    SPI1->CR1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0;  // Baud rate divisor = 256

    // Configure the reception threshold to immediately release a received 8-bit value
    SPI1->CR2 |= SPI_CR2_FRXTH;  // Set the FIFO threshold to 8-bit

    // Enable SPI1
    SPI1->CR1 |= SPI_CR1_SPE;  // Enable SPI1
}

void enable_sdcard(void) {
    GPIOB->ODR &= ~ GPIO_ODR_2;  // Set PB2 low to enable SD card
}

void disable_sdcard(void) {
    GPIOB->ODR |= GPIO_ODR_2;  // Set PB2 high to disable SD card
}

void init_sdcard_io(void) {
    // Initialize SPI1 with slow baud rate
    init_spi1_slow();

    // Configure PB2 as an output (for SD card control)
    GPIOB->MODER &= ~GPIO_MODER_MODER2_Msk;  // Clear MODER bits for PB2
    GPIOB->MODER |= GPIO_MODER_MODER2_0;     // Set PB2 as output

    // Disable SD card (set PB2 high)
    disable_sdcard();
}

void sdcard_io_high_speed(void) {
    // Disable SPI1
    SPI1->CR1 &= ~SPI_CR1_SPE;  // Disable SPI1

    // Set SPI1 baud rate to 12 MHz (assuming a system clock of 84 MHz)
    // Baud rate is determined by: BaudRate = f_PCLK / (2^(BR[2:0]))
    // For 12 MHz, BR[2:0] = 010 (divisor of 7 -> 84 MHz / 7 = 12 MHz)
    SPI1->CR1 &= ~SPI_CR1_BR;  // Clear the BR bits
    SPI1->CR1 |= (SPI_CR1_BR_2); // Set BR[2:0] = 010 for 12 MHz

    // Re-enable SPI1
    SPI1->CR1 |= SPI_CR1_SPE;  // Enable SPI1
}


void init_lcd_spi(void) {
    // Enable the clock for GPIOB
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;   // Enable GPIOB clock

    // Configure PB8, PB11, PB14 as GPIO outputs
    GPIOB->MODER &= ~(GPIO_MODER_MODER8_Msk | GPIO_MODER_MODER11_Msk | GPIO_MODER_MODER14_Msk);  // Clear MODER bits for PB8, PB11, PB14
    GPIOB->MODER |= (GPIO_MODER_MODER8_0 | GPIO_MODER_MODER11_0 | GPIO_MODER_MODER14_0);        // Set PB8, PB11, PB14 as output

    // Initialize SPI1 with slow settings
    init_spi1_slow();

    // Make SPI1 faster (set baud rate to 24 MHz)
    sdcard_io_high_speed();  // This sets the baud rate to 12 MHz, but we will modify the baud rate for LCD

    // BaudRate = f_PCLK / (2^BR) -> For 24 MHz, BR = 3 (84 MHz / 24 MHz = 3)
    SPI1->CR1 &= ~SPI_CR1_BR;  // Clear the BR bits
    SPI1->CR1 |= (SPI_CR1_BR_2 | SPI_CR1_BR_1);  // Set BR[2:0] = 011 for 24 MHz baud rate

    SPI1->CR1 &= ~(SPI_CR1_LSBFIRST_Pos); //MSB First
    SPI1->CR2 |=  (7<< SPI_CR2_DS_Pos);  // 8-bit data size

    // Enable SPI1 for communication with LCD
    SPI1->CR1 |= SPI_CR1_SPE;  // Enable SPI1
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    internal_clock();
    init_usart5();
    enable_tty_interrupt();
    
    setbuf(stdin,0); // These turn off buffering; more efficient, but makes it hard to explain why first 1023 characters not dispalyed
    setbuf(stdout,0);
    setbuf(stderr,0);
    command_shell();
    // printf("Enter your name: "); // Types name but shouldn't echo the characters; USE CTRL-J to finish
    // char name[80];
    // fgets(name, 80, stdin);
    // printf("Your name is %s", name);
    // printf("Type any characters.\n"); // After, will type TWO instead of ONE
    // for(;;) {
    //     char c = getchar();
    //     putchar(c);
    // }
}
#endif