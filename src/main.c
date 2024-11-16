/**
  ******************************************************************************
  * @file    main.c
  * @author  Matthew Roxas, Armaan Kanchan, Muhammad Zohaib Ali, Aditya Hebbani
  * @date    Nov 12, 2024
  * @brief   ECE 362 Course Project
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
#include <string.h>

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
    GPIOB->MODER &= ~(GPIO_MODER_MODER3_Msk | GPIO_MODER_MODER4_Msk | GPIO_MODER_MODER5_Msk);  // Clear MODER bits //FIX THIS 
    GPIOB->MODER |= (GPIO_MODER_MODER3_1 | GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1);   // Set PB3, PB4, PB5 to alternate function mode

    // Configure GPIOB alternate functions for SPI1
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL3_Msk | GPIO_AFRL_AFSEL4_Msk | GPIO_AFRL_AFSEL5_Msk);  // Clear AFR bits
    // GPIOB->AFR[0] |= (5 << GPIO_AFRL_AFSEL3_Pos) | (5 << GPIO_AFRL_AFSEL4_Pos) | (5 << GPIO_AFRL_AFSEL5_Pos);  // Set AF5 (SPI1) for SCK, MISO, MOSI //Why AF5?
    // GPIOB->AFR[0] |= (0 << GPIO_AFRL_AFSEL3_Pos) | (0 << GPIO_AFRL_AFSEL4_Pos) | (0 << GPIO_AFRL_AFSEL5_Pos);  // Set AF5 (SPI1) for SCK, MISO, MOSI


    // Configure SPI1 settings
    // SPI1->CR1 = 0;   // Reset all SPI1 settings          // Please do not do this. --Armaan notes : 
    SPI1->CR1 &= ~SPI_CR1_SPE;  // Disable SPI1

    //BAUDRATE 
    // SPI1->CR1 &= ~SPI_CR1_BR;  // Clear the BR bits
    SPI1->CR1 |= SPI_CR1_BR;  //111, highest divisor 256 


    // Set SPI1 in master mode, with 8-bit data size
    SPI1->CR1 |= SPI_CR1_MSTR;    // Master mode, 
    SPI1->CR2 |= (SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0);  //8-bit data size IS 0111
    SPI1->CR2 &= ~(SPI_CR2_DS_3);// << 8);
    SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;     // Enable software slave management and internal slave select

    // Set the baud rate divisor to the maximum value for the slowest baud rate
    // SPI1->CR1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0;  // Baud rate divisor = 256

    // Configure the reception threshold to immediately release a received 8-bit value
    SPI1->CR2 |= SPI_CR2_FRXTH;  // Set the FIFO threshold to 8-bit

    // Enable SPI1
    SPI1->CR1 |= SPI_CR1_SPE;  // Enable SPI1
}

void enable_sdcard(void) {
    GPIOB->ODR &= ~GPIO_ODR_2;  // Set PB2 low to enable SD card
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
    // For 12 MHz, BR[2:0] = 001 (divisor of 4 -> 48 MHz / 4 = 12 MHz)
    SPI1->CR1 &= ~SPI_CR1_BR;  // Clear the BR bits
    SPI1->CR1 |= (SPI_CR1_BR_0); // Set BR[2:0] = 001 for 12 MHz

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

    // // BaudRate = f_PCLK / (2^BR) -> For 24 MHz, BR = 3 (84 MHz / 24 MHz = 3)
    // SPI1->CR1 &= ~SPI_CR1_BR;  // Clear the BR bits
    // SPI1->CR1 |= (SPI_CR1_BR_2 | SPI_CR1_BR_1);  // Set BR[2:0] = 011 for 24 MHz baud rate

    // SPI1->CR1 &= ~(SPI_CR1_LSBFIRST_Pos); //MSB First
    // SPI1->CR2 |=  (7<< SPI_CR2_DS_Pos);  // 8-bit data size

    // // Enable SPI1 for communication with LCD
    // SPI1->CR1 |= SPI_CR1_SPE;  // Enable SPI1
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/// GPIO CODE/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
//   Init GPIO port B
 *        Pin 0: input
 *        Pin 4: input
 *        Pin 8-11: output
 *
 */
// void initb() {
  
//   RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
//   GPIOB->MODER |= 0x550000;//00000000010101010000000000000000 //OUTPUT 
//   GPIOB->MODER &= ~0x303;//0000000000000000000000001100000011  //INPUT 
                                 
// }

// /**
//  * @brief Init GPIO port C
//  *        Pin 0-3: inputs with internal pull down resistors
//  *        Pin 4-7: outputs
//  *
//  */
// void initc() {
//   RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
//   GPIOC->MODER &= ~0xFFFF;  //00000000000000001111111111111111 //INPUT 
//   GPIOC->MODER |= 0x5500; //00000000000000000101010100000000 //OUTPUT
//   // GPIOC->MODER &= ~0xFF;  //00000000000000000000000011111111 //INPUT 
  
//   //PULL UP PULL DOWN PART 
//   GPIOC->PUPDR |= 0xAA;  //00000000000000000000000010101010 //INPUT 
  
  
// }

// /**
//  * @brief Set GPIO port B pin to some value
//  *
//  * @param pin_num: Pin number in GPIO B
//  * @param val    : Pin value, if 0 then the
//  *                 pin is set low, else set high
//  */
// void setn(int32_t pin_num, int32_t val) {
  
//   if( val == 0){
//     GPIOB->BRR |= 1<< pin_num;
//   }
  
//   else {
//     GPIOB->BSRR |= 1<< pin_num; 
//   }


// }

// /**
//  * @brief Read GPIO port B pin values
//  *
//  * @param pin_num   : Pin number in GPIO B to be read
//  * @return int32_t  : 1: the pin is high; 0: the pin is low
//  */
// int32_t readpin(int32_t pin_num) {
//   int32_t var; 
//   var = GPIOB->IDR & 1<< pin_num; 
//   var = var>>pin_num; 
//   return var;


  
// }

// /**
//  * @brief Control LEDs with buttons
//  *        Use PB0 value for PB8
//  *        Use PB4 value for PB9
//  *
//  */
// void buttons(void) {
//   int32_t val0;
//   val0= readpin(0);
//   setn(8, val0);

//    int32_t val4;
//   val4= readpin(4);
//   setn(9, val4);


  
// }

// /**
//  * @brief Control LEDs with keypad
//  * 
//  */
// void keypad(void) {
//   int i = 0;
  
//   while (i<4){
//     GPIOC->ODR = 1<< (7-i); 
//     nano_wait(1000000);
//     int vara = GPIOC->IDR & 0xF;
//     setn(i+8, vara & 1<< (3-i));
//     i++; 

//   }
  
// }
// ////////////////gpio code done /////////////////////////////////////////////////////////////////////////////////////

/* Question handler libraries */
#include "cJSON.h"
#include "questions.h"
#include "lcd.h"

#define MAX_STRING_LENGTH 40  // Max length per string to fit on the LCD

void splitAndDisplayString(char *inputString) {
    int length = strlen(inputString);
    int numParts = (length + MAX_STRING_LENGTH - 1) / MAX_STRING_LENGTH; // Calculate the number of parts required

    // Loop over and display each part of the string
    for (int i = 0; i < numParts; i++) {
        // Create a temporary string to hold the current part of the input string
        char part[MAX_STRING_LENGTH + 1];  // +1 for null terminator
        int startIndex = i * MAX_STRING_LENGTH;
        
        // Copy the appropriate part of the input string
        strncpy(part, inputString + startIndex, MAX_STRING_LENGTH);
        part[MAX_STRING_LENGTH] = '\0';  // Null terminate the string
        
        // Set up the y position for each part of the string to be displayed
        int yPosition = 100 + (i * 20);  // Adjust the 20 for spacing between lines
        
        // Display the part of the string on the LCD
        LCD_DrawString(0, yPosition, RED, BLACK, part, 16, 0);
    }
}

int main() {
    internal_clock();
    init_usart5();
    enable_tty_interrupt();
    
    /* Open command shell */
    // setbuf(stdin,0); // These turn off buffering; more efficient, but makes it hard to explain why first 1023 characters not dispalyed
    // setbuf(stdout,0);
    // setbuf(stderr,0);
    // command_shell();
    // printf("Enter your name: "); // Types name but shouldn't echo the characters; USE CTRL-J to finish
    // char name[80];
    // fgets(name, 80, stdin);
    // printf("Your name is %s", name);
    // printf("Type any characters.\n"); // After, will type TWO instead of ONE
    // for(;;) {
    //     char c = getchar();
    //     putchar(c);
    // }

    /* Test question handler */
    // char *question = "hebbani is a clevery boy with a big nose and he loves to eat cake and dance but he is very annoying at times"; //need to implement a checker to write another drawstring if the length of the string is too long i.e longer than 41 
    // // char *question = "bro";
    // LCD_Setup();
    // LCD_Clear(WHITE);
    // // LCD_DrawString(0, 100, RED, BLACK, question, 16, 0);
    // splitAndDisplayString(question); 

    /* What's in the sd card? */
    // listDirectory("/");

    /* Print in command line */
    // srand(time(NULL));
    // Question questions[MAX_QUESTIONS];
    // int question_count;
    // // loadQuestionsFromJSON("qs.txt", questions, &question_count);
    // char *question = printRandomQuestion(questions, question_count);

    // setbuf(stdin,0); // These turn off buffering; more efficient, but makes it hard to explain why first 1023 characters not dispalyed
    // setbuf(stdout,0);
    // setbuf(stderr,0);
    // command_shell();
    // printf("Enter your name: "); // Types name but shouldn't echo the characters; USE CTRL-J to finish
    // char name[80];
    // fgets(name, 80, stdin);
    // printf("Your name is %s", name);
    // printf("Type any characters.\n"); // After, will type TWO instead of ONE
    // printf("%s\n", question);
    // for(;;) {
    //     char c = getchar();
    //     putchar(c);
    // }
    
    /* Display question */
    srand(time(NULL));
    Question questions[MAX_QUESTIONS];
    int question_count;
    loadQuestionsFromJSON("qs_3.txt", questions, &question_count);
    char *question = printRandomQuestion(questions, question_count);
    LCD_Setup(); 
    LCD_Clear(WHITE);
    splitAndDisplayString(question); 

    /* Display scoreboard */
    int *score = {100, 200, 500, 1000, 10000, 100000, 10000000};
    

    //create an exti from the keypad which causes the score to increase and the question to go to the next one 

    //create a timer which displays the question for 10 seconds, then the options for 10 seconds and once it finishes, the game ends 

    //optional: add leaderboard to the thing 

    //add game ending and starting display and maybe if time option to take away ur winnings before each new question 
}
#endif
