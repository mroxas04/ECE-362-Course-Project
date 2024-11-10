//HELLO BOIS, GOD BLESS AMERICA 
/**
  ******************************************************************************
  * @file    main.c
  * @author  Matthew Roxas, Armaan Kanchan, Muhammad Zohaib Ali, Aditya Hebbani 
  * @date    Nov 10, 2024
  * @brief   ECE 362 Course project 
  ******************************************************************************
*/


#include "stm32f0xx.h"
#include <stdint.h>

void internal_clock();


#define COMPLETED_STEP

void init_usart5() {
    // TODO
<<<<<<< HEAD
    RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN);
=======
    RCC->AHBENR |= (RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN);
>>>>>>> 88ee94089fb7a0d46e654a4c5563b05a84276f55
    RCC->APB1ENR |= RCC_APB1ENR_USART5EN;

    // AFR FOR PC12 FOR USART5
    GPIOC->MODER &= ~(GPIO_MODER_MODER12); // Clear mode bits
<<<<<<< HEAD
    GPIOC->MODER |= ( GPIO_MODER_MODER12_0); //2'b10 right shifter 12

    // Setring afr to USART5_TX (AF14 for PC12)
    GPIOC->AFR[1] |= (14 << (4 * (12 - 8))); // IS THERE A BETTER WAY ? 

    // AFR FOR PD2 FOR USART5
    GPIOD->MODER &= ~(GPIO_MODER_MODER2); // Clear mode bits
    GPIOD->MODER |= ( GPIO_MODER_MODER2_0); // 2'b10 right shfited 12

    // afr to USART5_RX (AF14 for PD2)
    GPIOD->AFR[0] |= (14 << (4 * 2)); // Set AF14 IS THERE A BETTER WAY ?? ////////////
=======
    GPIOC->MODER |= ( GPIO_MODER_MODER12_1); //2'b10 left shifter 12

    // Setring afr to USART5_TX (AF14 for PC12)
    GPIOC->AFR[1] |= 0x20000; //'2b10 left shifted

    // AFR FOR PD2 FOR USART5
    GPIOD->MODER &= ~(GPIO_MODER_MODER2); // Clear mode bits
    GPIOD->MODER |= ( GPIO_MODER_MODER2_1); // 2'b10 right shfited 12

    // afr to USART5_RX (AF14 for PD2)
    GPIOD->AFR[0] |= 0x200; 
>>>>>>> 88ee94089fb7a0d46e654a4c5563b05a84276f55

    // Configure USART5
    USART5->CR1 &= ~USART_CR1_UE; // Disable USART5

<<<<<<< HEAD
    //  SetTING word size to 8 bits 
    USART5->CR1 &= ~USART_CR1_M; // 0 = 8 data bits
=======
    //  SetTING word size to 8 b 
    USART5->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0) ; 
>>>>>>> 88ee94089fb7a0d46e654a4c5563b05a84276f55

    // SetTING for 1 stop bit 
    USART5->CR2 &= ~USART_CR2_STOP;

    // Set no parity control (clear PCE bit in CR1)
    USART5->CR1 &= ~USART_CR1_PCE;

    //16X OVER SAMPLING ?? 
<<<<<<< HEAD

    //  SetTING baud rate to 115200
    // Assuming system clock is 84 MHz
    USART5->BRR = 84000000 / 115200; 
=======
    USART5->CR1 &= ~ USART_CR1_OVER8; 

    //  SetTING baud rate to 115200
    // Asuming system clock is 48 MHz
    USART5->BRR = 0x1A1; // 48000000 / 115200; 
>>>>>>> 88ee94089fb7a0d46e654a4c5563b05a84276f55

    // EnablING the transmitter and the receiver
    USART5->CR1 |= (USART_CR1_TE | USART_CR1_RE);

    // EnablING the USART
    USART5->CR1 |= USART_CR1_UE;

    // Wait for TE and RE to be acknowledged
    while (!(USART5->ISR & USART_ISR_TEACK)); // Wait until TEACK is set
    while (!(USART5->ISR & USART_ISR_REACK)); // Wait until REACK is set
}



#ifdef COMPLETED_STEP

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
    while (!fifo_full(&input_fifo)) {
        asm volatile ("wfi"); //wait for an interrupt 
         
    }
    // If we missed reading some characters, clear the overrun flag.
   
    // Return a character
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


int main() {
    internal_clock();
    init_usart5();
    enable_tty_interrupt();

    setbuf(stdin,0); // These turn off buffering; more efficient, but makes it hard to explain why first 1023 characters not dispalyed
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: "); // Types name but shouldn't echo the characters; USE CTRL-J to finish
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n"); // After, will type TWO instead of ONE
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif