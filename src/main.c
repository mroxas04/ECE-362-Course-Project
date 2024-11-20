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
void init_usart1_tx();
void usart1_send_char(char c);

int* score = 0;
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

// void splitAndDisplayString(char *inputString) {
//     int length = strlen(inputString);
//     int numParts = (length + MAX_STRING_LENGTH - 1) / MAX_STRING_LENGTH; // Calculate the number of parts required

//     // Loop over and display each part of the string
//     for (int i = 0; i < numParts; i++) {
//         // Create a temporary string to hold the current part of the input string
//         char part[MAX_STRING_LENGTH + 1];  // +1 for null terminator
//         int startIndex = i * MAX_STRING_LENGTH;
        
//         // Copy the appropriate part of the input string
//         strncpy(part, inputString + startIndex, MAX_STRING_LENGTH);
//         part[MAX_STRING_LENGTH] = '\0';  // Null terminate the string
        
//         // Set up the y position for each part of the string to be displayed
//         int yPosition = 100 + (i * 20);  // Adjust the 20 for spacing between lines
        
//         // Display the part of the string on the LCD
//         LCD_DrawString(0, yPosition, RED, BLACK, part, 16, 0);
//     }
// }

void splitAndDisplayString(char *inputString) {
    int length = strlen(inputString);
    int startIndex = 0;
    int lineIndex = 0;
    
    // Loop over the input string, handling lines of up to MAX_STRING_LENGTH
    while (startIndex < length) {
        int maxLength = MAX_STRING_LENGTH;  // Max length of current part
        char part[MAX_STRING_LENGTH + 1];   // +1 for null terminator

        // Find the first newline or the end of the string within the max length
        int endIndex = startIndex + maxLength;
        if (endIndex > length) {
            endIndex = length;
        }

        // Check for newline character within the current line range
        char *newlinePos = strchr(inputString + startIndex, '\n');
        if (newlinePos != NULL && newlinePos < inputString + endIndex) {
            // If a newline exists within the current range, cut the line at the newline
            endIndex = newlinePos - inputString + 1;  // +1 to include the newline character
        }

        // Copy the current part of the string
        int partLength = endIndex - startIndex;
        strncpy(part, inputString + startIndex, partLength);
        part[partLength] = '\0';  // Null terminate the string

        // Display the part on the LCD at the correct y position
        int yPosition = 10 + (lineIndex * 20);  // Adjust for vertical spacing
        LCD_DrawString(0, yPosition, RED, BLACK, part, 16, 0);

        // Move the startIndex forward and increment the line index
        startIndex = endIndex;
        lineIndex++;

        // Skip over the newline character if it's part of the current segment
        if (inputString[startIndex] == '\n') {
            startIndex++;  // Skip over the newline
        }
    }
}

















/* Timer 2 to handle 30 second countdown */

volatile uint32_t countdown = 30;     // 30-second countdown
volatile int question_active = 0;      // Whether a question is active
int question_index = -1;                // Index of the current question
Question questions[MAX_QUESTIONS];      // Array of questions
int question_count = 0;                // Total number of questions

// Key scanning
// 16 history bytes.  Each byte represents the last 8 samples of a button.
uint8_t hist[16];
char queue[2];  // A two-entry queue of button press/release events.
int qin;        // Which queue entry is next for input
int qout;       // Which queue entry is next for output
uint8_t col;

const char keymap[] = "DCBA#9630852*741";

/* Keypad scanning and debouncing */

void push_queue(int n) {
    queue[qin] = n;
    qin ^= 1;
}

char pop_queue() {
    char tmp = queue[qout];
    queue[qout] = 0;
    qout ^= 1;
    return tmp;
}

void update_history(int c, int rows)
{
    // We used to make students do this in assembly language.
    for(int i = 0; i < 4; i++) {
        hist[4*c+i] = (hist[4*c+i]<<1) + ((rows>>i)&1);
        if (hist[4*c+i] == 0x01)
            push_queue(0x80 | keymap[4*c+i]);
        if (hist[4*c+i] == 0xfe)
            push_queue(keymap[4*c+i]);
    }
}

void drive_column(int c)
{
    GPIOC->BSRR = 0xf00000 | ~(1 << (c + 4));
}

int read_rows()
{
    return (~GPIOC->IDR) & 0xf;
}

char get_key_event(void) {
    for(;;) {
        asm volatile ("wfi");   // wait for an interrupt
       if (queue[qout] != 0)
            break;
    }
    return pop_queue();
}

char get_keypress() {
    char event;
    for(;;) {
        // Wait for every button event...
        event = get_key_event();
        // ...but ignore if it's a release.
        if (event & 0x80)
            break;
    }
    return event & 0x7f;
}

void show_keys(void)
{
    char buf[] = "        ";
    for(;;) {
        char event = get_key_event();
        memmove(buf, &buf[1], 7);
        buf[7] = event;
        print(buf);
    }
}


/* Software timer */
void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 6000; i++) {
        __NOP();
    }
}
















volatile int timer_countdown = 100; // Timer countdown in seconds
volatile int game_over = 0;        // Game over state flag
volatile int key_pressed = 0;      // Flag to detect a key press

void initc();
void set_col(int col);
void SysTick_Handler();
void init_systick();

extern void autotest();
extern void internal_clock();
extern void nano_wait(int);

/**
 * @brief Init GPIO port C
 *        PC0-PC3 as input pins with the pull down resistor enabled
 *        PC4-PC9 as output pins
 * 
 */
void initc() {
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN; //enabling the port 
  GPIOC->MODER &= ~0x000000FF; //setting input 
  GPIOC->MODER |= 0x00055500; //output 
  GPIOC->PUPDR |= 0x000000AA; //pull down

}

/**
 * @brief Enable the SysTick interrupt to occur every 1/16 seconds.
 * 
 */
void init_systick() {
  SysTick->LOAD = 375000-1;
  SysTick->VAL = 0;
    SysTick->CTRL &= ~0x4;

  SysTick->CTRL |= 0x3;
}


volatile int current_col = 1;

/**
 * @brief The ISR for the SysTick interrupt.
 * 
//  */
// void SysTick_Handler() {
//    //1. Read the row pins using GPIOC->IDR
//     //    You can check the pins used for rows 
//     //    of keypad in lab 1 manual
//     // 2. If the var `current_col` corresponds to
//     //    the row value, toggle one of the leds connected 
//     //    to PB8-11.
//     //    Basically the same we have done in lab 1
//     // 3. Increment the `current_col` and wrap around
//     //    to 1 if `current_col` > 4. So that next time
//     //    we scan the next column
//     // 4. Set the changed column pin designated by `current_col`
//     //    to 1 and rest of the column pins to 0 to energized that
//     //    particular column for next read of keypad.

//    int current_row_val = GPIOC->IDR; //determines row number that is on 

//    if(current_col == 4)
//    {
//     if(current_row_val & 0x1){   // if key D 
//     char* question3 = "free imran khan"; 
//         LCD_Setup(); 
//         LCD_Clear(BLACK);
//         splitAndDisplayString(question3); 
//    }

//    if(current_row_val & 0x2){   // if key C
//     char* question3 = "free massage 6507858464"; 
//         LCD_Setup(); 
//         LCD_Clear(BLACK);
//         splitAndDisplayString(question3); 
//    }

//    if(current_row_val & 0x4){   // if key B
//     char* question3 = "free patti "; 
//         LCD_Setup(); 
//         LCD_Clear(BLACK);
//         splitAndDisplayString(question3); 
//    }

//    if(current_row_val & 0x8){   // if key A
//     char* question3 = "free badhri "; 
//         LCD_Setup(); 
//         LCD_Clear(BLACK);
//         splitAndDisplayString(question3); 
//    }

//    }
//     current_col++;
    
//    if(current_col > 4){
//     current_col = 1;
//    }
//    set_col(current_col);
// }


/////buzzer ////////////////////////////////////////////////////////////////////////////
void init_buzzer() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;   // Enable GPIOB clock

    GPIOA->MODER &= ~GPIO_MODER_MODER5; // Clear mode bits for PB5
    GPIOA->MODER |= GPIO_MODER_MODER5_0; // Set Pa5 to output mode
     
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT_5;  // Set Pa5 to push-pull
    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR5; // Set Pa5 to high speed
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR5;  // No pull-up or pull-down
}

void buzzer_beep() {
    for (int i = 0; i < 3; i++) { // Beep 3 times
        GPIOA->ODR |= (1 << 5);  // Set PA5 high
        delay_ms(5000);
        GPIOA->ODR &= ~(1 << 5); // Set Pa5 low
        delay_ms(5000);
    }
}

/////BUZZER CODE//////////////////////////////////////////////////////////////////////////


void outOfTime() {
    question_index = -1;
    game_over = 1;  // Set the game over flag
    char *game_over_msg = "Out of time! Game over.";
    LCD_Setup();
    LCD_Clear(BLACK);
    splitAndDisplayString(game_over_msg);
    buzzer_beep();  // Buzzer beep when oUT OF TIME DING DONG 
}

void wrongAnswer() {
    game_over = 1;
    char wrong[50];
    snprintf(wrong, sizeof(wrong), "Game over! The right answer was %s. ", questions[question_index].correct_answer);
    question_index = -1;
    LCD_Setup();
    LCD_Clear(BLACK);
    splitAndDisplayString(wrong);
    // hold timer
    buzzer_beep();  // Buzzer beep wOMP Womp answer 
}

void correctAnswer(int* score) {
    char *right = "Correct! Press 1 for next question.";
    char scoreString[100];
    score++;
    LCD_Setup();
    LCD_Clear(BLACK);
    splitAndDisplayString(right);
    snprintf(scoreString, sizeof(scoreString), "Your score is %d", score);
    usart1_send_string(scoreString);
    // hold timer
}

void nextQuestion() {
    char *question = printRandomQuestion(questions, question_count, ++question_index);
    LCD_Setup();
    LCD_Clear(BLACK);
    splitAndDisplayString(question);
    timer_countdown = 10;
}

void checkAnswer(char answer) {
    if (answer != (char)questions[question_index].correct_answer[0]) {
        wrongAnswer();
    }
    else {
        correctAnswer();
    }
}

void SysTick_Handler() {
    static int systick_ticks = 0;  // Counts SysTick ticks (1/16 second per tick)

    systick_ticks++;
    if (systick_ticks >= 16) {  // 16 ticks = 1 second
        systick_ticks = 0;

        if (timer_countdown > 0) {  // Decrement the timer if not zero
            timer_countdown--;
        } else if (!game_over) {  // Transition to "Game Over" if timer reaches zero
            outOfTime();
        }
    }

    // Handle keypad scanning
    int current_row_val = GPIOC->IDR;  // Read the row pins
    if (current_row_val) {  // If a key is pressed

        if(current_col == 3) //RESET BUTTON 
        {
            if(current_row_val & 0x1)
            {
                key_pressed = 1;      // Set the key pressed flag
                // timer_countdown = 10; // Reset the timer to 20 seconds (but it continues counting down)
                game_over = 0;        // Clear the game over flag
                // char *question = "Press a key to restart"; 
                char *question = "Press 1 to start";
                LCD_Setup(); 
                LCD_Clear(BLACK);
                splitAndDisplayString(question); 
            }
        }

        if (current_col == 1) { // First question
            if (current_row_val & 0x8) {
                timer_countdown = 10;
                if(question_index < (question_count - 1))
                {
                    nextQuestion();
                }
                else
                {
                    char *question = "\n\n\n     Congratulations! You are now are a millionaire!\n            Press # to restart"; 
                    splitAndDisplayString(question); 
                    question_index = 0; 
                }
            }
        }
    
        // Handle specific key presses (row 4 example)
        if (current_col == 4) {

            // Map choices
            char choice = 0;
            if (current_row_val & 0x1) {
                choice = 'D';
            } else if (current_row_val & 0x2) {
                choice = 'C';
            } else if (current_row_val & 0x4) {
                choice = 'B';
            } else if (current_row_val & 0x8) {
                choice = 'A';
            }

            if (choice) { // If A, B, C, or D is pressed, check answer and load next question / game over
                checkAnswer(choice);
            }

            // if (current_row_val & 0x1) {  // Key D
            //     // char *question = "Free Imran Khan";
            //     char *question = printRandomQuestion(questions, question_count, ++question_index);
            //     LCD_Setup();
            //     LCD_Clear(BLACK);
            //     splitAndDisplayString(question);
            // } else if (current_row_val & 0x2) {  // Key C
            //     // char *question = "Free massage 6507858464";
            //     char *question = printRandomQuestion(questions, question_count, ++question_index);
            //     LCD_Setup();
            //     LCD_Clear(BLACK);
            //     splitAndDisplayString(question);
            // } else if (current_row_val & 0x4) {  // Key B
            //     // char *question = "Free Patti";
            //     char *question = printRandomQuestion(questions, question_count, ++question_index);
            //     LCD_Setup();
            //     LCD_Clear(BLACK);
            //     splitAndDisplayString(question);
            // } else if (current_row_val & 0x8) {  // Key A
            //     // char *question = "Free Badhri";
            //     char *question = printRandomQuestion(questions, question_count, ++question_index);
            //     LCD_Setup();
            //     LCD_Clear(BLACK);
            //     splitAndDisplayString(question);
            // }
        }
    }

    // Increment the column for scanning
    current_col++;
    if (current_col > 4) {
        current_col = 1;
    }
    set_col(current_col);
}

/**
 * @brief For the keypad pins, 
 *        Set the specified column level to logic "high".
 *        Set the other three three columns to logic "low".
 * 
 * @param col 
 */
void set_col(int col) {
// Set PC4-7 (i.e. all columns) output to be 0
    // Set the column `col` output to be 1
    //  if col = 1, PC7 will be set to 1 as 
    //  it is connected to column 1 of the keypad 
    //  Likewise, if col = 4, PC4 will be set to 1

    GPIOC->ODR &= ~0xFF00; //set them as 0 
    GPIOC->ODR = (1<<(8-col));

}


///UART CODE ////////////////////////////////////////////////////////////////////////////////////
void init_usart1_tx() {
    // Enable GPIOA and USART1 clocks
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // Configure PA9 as USART1_TX (Alternate Function)
    GPIOA->MODER &= ~(GPIO_MODER_MODER9);        // Clear mode bits
    GPIOA->MODER |= (GPIO_MODER_MODER9_1);       // Set to Alternate Function
    GPIOA->AFR[1] |= (1 << (4 * (9 - 8)));       // Set AF1 for PA9

    // Configure USART1
    USART1->CR1 &= ~USART_CR1_UE;                // Disable USART for configuration
    USART1->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0); // 8 data bits
    USART1->CR2 &= ~USART_CR2_STOP;              // 1 stop bit
    USART1->CR1 &= ~USART_CR1_PCE;               // No parity
    USART1->CR1 &= ~USART_CR1_OVER8;             // 16x oversampling

    // Set baud rate (assuming 48 MHz clock)   maybe an 84
    USART1->BRR = 48000000 / 115200;             // Set baud rate to 115200

    // Enable transmitter
    USART1->CR1 |= USART_CR1_TE;

    // Enable USART1
    USART1->CR1 |= USART_CR1_UE;

    // Wait for transmitter readiness
    while (!(USART1->ISR & USART_ISR_TEACK));
}

void usart1_send_char(char c) {
    while (!(USART1->ISR & USART_ISR_TXE));  // Wait until TX buffer is empty
    USART1->TDR = c;                         // Transmit the character
}

void usart1_send_string(const char *str) {
    while (*str) {
        usart1_send_char(*str++);
    }
}
////UART CODE DONE ///////////////////////////////////////////////////////////////////////


int main() {

    /* Setup */
    internal_clock();
    //init_usart5();
    enable_tty_interrupt();
    initc();
    init_systick();
    init_usart1_tx(); //FOR USART
    init_buzzer();    // Initialize the buzzer

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

    /* Display question */
    // srand(time(NULL));
    // Question questions[MAX_QUESTIONS];
    // int question_count;
    // loadQuestionsFromJSON("qs_3.txt", questions, &question_count);
    // char *question = printRandomQuestion(questions, question_count);
    // LCD_Setup(); 
    // LCD_Clear(BLACK);
    // splitAndDisplayString(question); 

    /* Go to the next question when timer reaches 0 */
    loadQuestionsFromJSON("qs_3.txt", questions, &question_count);

    //while (question_index < question_count) {
        //char *question = printRandomQuestion(questions, question_count, 0);

        //char *question = "hello world"; 

        // Start game
        char *question = "\n\n\n     Who's going to be a millionaire? \n            Press 1 to start"; 
        LCD_Setup(); 
        LCD_Clear(BLACK);
        splitAndDisplayString(question); 

        //delay_ms(2000);
        //LCD_Clear(BLACK);

        //delay_ms(1000); 
        // splitAndDisplayString(question2); 

        /* When game is over, reset question_index to 0. Hit # to go back to the title screen. Hit 1 again to start the game.
            When in a question, you should not be able to hit 1 to go to next. */
        // init_usart1_tx(); //FOR USART
        while(1)
        {
         // usart1_send_string("Hello from STM TX!\r\n");
        // usart1_send_char('a');
        // for (volatile int i = 0; i < 1000000; i++);  // Delay loop

        }

        // delay_ms(1000); 
        
        // *question = printRandomQuestion(questions, question_count, 1);
        // LCD_Setup(); 
        // LCD_Clear(BLACK);
        // splitAndDisplayString(question); 

        //delay_ms(100);
    //     question_index++;
    // }





    /* Use timer to iterate through questions and check keypad */
    // LCD_Setup(); 
    // LCD_Clear(BLACK);
    // start_game();
    // init_tim2();


    /* Display scoreboard */
    // int *score = {100, 200, 500, 1000, 10000, 100000, 10000000};
    
    //create an exti from the keypad which causes the score to increase and the question to go to the next one 

    //create a timer which displays the question for 10 seconds, then the options for 10 seconds and once it finishes, the game ends 

    //optional: add leaderboard to the thing 

    //add game ending and starting display and maybe if time option to take away ur winnings before each new question 
}
#endif
