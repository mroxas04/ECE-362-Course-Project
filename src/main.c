/**
  ******************************************************************************
  * @file    main.c
<<<<<<< HEAD
  * @author  Matthrew Roxas, Armaan Kanchan, Muhammad Zohaib Ali, Aditya Hebbani
  * @date    Nov 10, 2024
  * @brief   ECE 362 Course project
  ******************************************************************************
*/


#include "stm32f0xx.h"

void set_char_msg(int, char);
void nano_wait(unsigned int);
void game(void);
void internal_clock();
void check_wiring();
void autotest();

//===========================================================================
// Configure GPIOC
//===========================================================================
void enable_ports(void) {
    // Only enable port C for the keypad
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER &= ~0xffff;
    GPIOC->MODER |= 0x55 << (4*2);
    GPIOC->OTYPER &= ~0xff;
    GPIOC->OTYPER |= 0xf0;
    GPIOC->PUPDR &= ~0xff;
    GPIOC->PUPDR |= 0x55;
}


uint8_t col; // the column being scanned

void drive_column(int);   // energize one of the column outputs
int  read_rows();         // read the four row inputs
void update_history(int col, int rows); // record the buttons of the driven column
char get_key_event(void); // wait for a button event (press or release)
char get_keypress(void);  // wait for only a button press event.
float getfloat(void);     // read a floating-point number from keypad
void show_keys(void);     // demonstrate get_key_event()

//===========================================================================
// Bit Bang SPI LED Array
//===========================================================================
int msg_index = 0;
uint16_t msg[8] = { 0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700 };
extern const char font[];

//===========================================================================
// Configure PB12 (CS), PB13 (SCK), and PB15 (SDI) for outputs
//===========================================================================
void setup_bb(void) {
     RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    // Configure PB12, PB13, and PB15 as output
    GPIOB-> MODER |= GPIO_MODER_MODER12_0  | GPIO_MODER_MODER13_0 | GPIO_MODER_MODER15_0 ;

  
    GPIOB->ODR |= GPIO_ODR_12;  //CS (PB12) high
    GPIOB->ODR &= ~GPIO_ODR_13 ;   //  SCK (PB13) low

    
}

void small_delay(void) {
    nano_wait(5000000);
}

//===========================================================================
// Set the MOSI bit, then set the clock high and low.
// Pause between doing these steps with small_delay().
//===========================================================================
void bb_write_bit(int val) {
 

    if (val == 0) {
         GPIOB->ODR &= ~GPIO_ODR_15;
    }

    else {
        GPIOB->ODR |= GPIO_ODR_15;
    }

    small_delay(); 

    GPIOB->ODR |= GPIO_ODR_13 ;   //  SCK (PB13) high

    small_delay(); 

    GPIOB->ODR &= ~GPIO_ODR_13 ;   //  SCK (PB13) low


    // CS (PB12)
    // SCK (PB13)
    // SDI (PB15)
    
}

//===========================================================================
// Set CS (PB12) low,
// write 16 bits using bb_write_bit,
// then set CS high.
//===========================================================================
void bb_write_halfword(int halfword) {
    
    GPIOB->ODR &= ~GPIO_ODR_12; 

    for (int i = 15; i >= 0; i--) {
        // Isolate the bit to write
        int bit = (halfword >> i) & 1;
        bb_write_bit(bit);
    }

    GPIOB->ODR |= GPIO_ODR_12; 
    
    
}

//===========================================================================
// Continually bitbang the msg[] array.
//===========================================================================
void drive_bb(void) {
    for(;;)
        for(int d=0; d<8; d++) {
            bb_write_halfword(msg[d]);
            nano_wait(1000000); // wait 1 ms between digits
        }
}

//============================================================================
// Configure Timer 15 for an update rate of 1 kHz.
// Trigger the DMA channel on each update.
// Copy this from lab 4 or lab 5.
//============================================================================
void init_tim15(void) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
    TIM15->PSC = 2400-1;
    TIM15->ARR = 20-1;
    TIM15->DIER |= TIM_DIER_UDE;
    TIM15->CR1 |= TIM_CR1_CEN;

}


//===========================================================================
// Configure timer 7 to invoke the update interrupt at 1kHz
// Copy from lab 4 or 5.
//===========================================================================
void init_tim7(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7 ->PSC = 1000-1;
    TIM7->ARR = 48-1;
    TIM7->DIER |= TIM_DIER_UIE; 
    NVIC -> ISER[0] |= (1 << TIM7_IRQn);
    TIM7->CR1 |= TIM_CR1_CEN;
}


//===========================================================================
// Copy the Timer 7 ISR from lab 5
//===========================================================================
// TODO To be copied
void TIM7_IRQHandler() {
  TIM7->SR &= ~TIM_SR_UIF;
  
  
  int rows = read_rows(); 
 

//   if (rows != 0) {
    // key = rows_to_key(rows); 
    // handle_key(key); 
    update_history(col,rows);
//   }
  col = (col+1) & 3; 
  
  
    // car = disp[col]; 
    // show_char(col, car);
    // col++; 
    drive_column(col); 

  
 
}



//===========================================================================
// Initialize the SPI2 peripheral.
//===========================================================================
void init_spi2(void) {
    RCC-> AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= ~  (0xCF000000); //(GPIO_MODER_MODER12 | GPIO_MODER_MODER13 | GPIO_MODER_MODER15);
    GPIOB->MODER |= (0x8A000000) ; // (GPIO_MODER_MODER12_0 | GPIO_MODER_MODER13_0 | GPIO_MODER_MODER15_0);

    GPIOB-> AFR[1] &= ~(0xF0FF0000); 
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    SPI2->CR1 &= ~SPI_CR1_SPE;

    // Set the baud rate as low as possible (max divisor for BR)
    // SPI2->CR1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0;  // Set BR = 256  this sets it to 111

    SPI2->CR1 |= SPI_CR1_BR;  //set to 111
    SPI2->CR2 |= SPI_CR2_DS;

    // Configure as master
    SPI2->CR1 |= SPI_CR1_MSTR;  

    // Set the SS output enable bit and enable NSSP
    // SPI2->CR1 |= SPI_CR1_SSI | SPI_CR1_SSM;  
    SPI2->CR2 |= SPI_CR2_NSSP | SPI_CR2_SSOE ;

    // Set the TXDMAEN bit to enable DMA transfers on transmit buffer empty
    SPI2->CR2 |= SPI_CR2_TXDMAEN;  

    // Enable the SPI channel
    SPI2->CR1 |= SPI_CR1_SPE;  

}

//===========================================================================
// Configure the SPI2 peripheral to trigger the DMA channel when the
// transmitter is empty.  Use the code from setup_dma from lab 5.
//===========================================================================
void spi2_setup_dma(void) {
    RCC->AHBENR |=  RCC_AHBENR_DMA1EN;
    DMA1_Channel5->CCR &= ~DMA_CCR_EN ;
    DMA1_Channel5->CMAR = (uint32_t)&msg;
    DMA1_Channel5->CPAR = &SPI2->DR;
    DMA1_Channel5->CNDTR = 8;
    DMA1_Channel5->CCR |= DMA_CCR_DIR;
    DMA1_Channel5->CCR |= DMA_CCR_MINC;
    DMA1_Channel5->CCR |= DMA_CCR_MSIZE_0;
    DMA1_Channel5->CCR |= DMA_CCR_PSIZE_0;
    DMA1_Channel5->CCR |= DMA_CCR_CIRC;
}

//===========================================================================
// Enable the DMA channel.
//===========================================================================
void spi2_enable_dma(void) {
     DMA1_Channel5->CCR |= DMA_CCR_EN;
    
}

//===========================================================================
// 4.4 SPI OLED Display
//===========================================================================
void init_spi1() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // Configure PA15 (NSS), PA5 (SCK), PA7 (MOSI) as alternate function
    GPIOA->MODER &= ~(GPIO_MODER_MODER15 |GPIO_MODER_MODER5 |GPIO_MODER_MODER7 ); // Clear first
    GPIOA->MODER |= (GPIO_MODER_MODER15_1 |GPIO_MODER_MODER5_1 |GPIO_MODER_MODER7_1);  

    // Set alternate function type for PA15, PA5, PA7 to SPI1
    // GPIOA->AFR[1] &= ~(GPIO_AFRH_AFRH7); // Clear AF for PA15, PA5, PA7
    // GPIOA->AFR[0] &= ~(GPIO_AFRL_AFRL5 | GPIO_AFRL_AFRL7 ); // Clear AF for PA15, PA5, PA7
   

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
   
    SPI1->CR1 &= ~SPI_CR1_SPE;
    SPI1->CR1 |= SPI_CR1_BR;  

    // SPI1->CR2 |= (SPI_CR2_DS) ;
    SPI1->CR2 = (SPI_CR2_DS_3 | SPI_CR2_DS_0); //1001 = 10 bit 



    SPI1->CR1 |= SPI_CR1_MSTR;
    SPI1->CR2 |= SPI_CR2_TXDMAEN;  

    SPI1->CR2 |= SPI_CR2_NSSP | SPI_CR2_SSOE;
    SPI1->CR1 |= SPI_CR1_SPE;
    
}
void spi_cmd(unsigned int data) {
     
    while (!(SPI1->SR & SPI_SR_TXE)) {
    }

    SPI1->DR = data;

}
void spi_data(unsigned int data) {
     spi_cmd(data | 0x200);
    
}
void spi1_init_oled() {
    nano_wait(1);
    spi_cmd(0x38);
    spi_cmd(0x08);
    spi_cmd(0x01);
    nano_wait(2);
    spi_cmd(0x06);
    spi_cmd(0x02);
    spi_cmd(0x0C);  
}
void spi1_display1(const char *string) {
    
    spi_data(0x002); //position defined below 

    int i = 0; 
    for (i = 0; i < len(string); i++){
        spi_data(string[i]);
    }

    //while (string != '/0') 
    
}
void spi1_display2(const char *string) {

    spi_data(0x0c0); // position defined below 

     int j = 0; 
    for (j = 0; j < len(string); j++){
        spi_data(string[j]);
    }

    
    
}

//===========================================================================
// This is the 34-entry buffer to be copied into SPI1.
// Each element is a 16-bit value that is either character data or a command.
// Element 0 is the command to set the cursor to the first position of line 1.
// The next 16 elements are 16 characters.
// Element 17 is the command to set the cursor to the first position of line 2.
//===========================================================================
uint16_t display[34] = {
        0x002, // Command to set the cursor at the first position line 1
        0x200+'E', 0x200+'C', 0x200+'E', 0x200+'3', 0x200+'6', + 0x200+'2', 0x200+' ', 0x200+'i',
        0x200+'s', 0x200+' ', 0x200+'t', 0x200+'h', + 0x200+'e', 0x200+' ', 0x200+' ', 0x200+' ',
        0x0c0, // Command to set the cursor at the first position line 2
        0x200+'c', 0x200+'l', 0x200+'a', 0x200+'s', 0x200+'s', + 0x200+' ', 0x200+'f', 0x200+'o',
        0x200+'r', 0x200+' ', 0x200+'y', 0x200+'o', + 0x200+'u', 0x200+'!', 0x200+' ', 0x200+' ',
};

//===========================================================================
// Configure the proper DMA channel to be triggered by SPI1_TX.
// Set the SPI1 peripheral to trigger a DMA when the transmitter is empty.
//===========================================================================
void spi1_setup_dma(void) {
   
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    DMA1_Channel3->CCR &= ~DMA_CCR_EN; 

    DMA1_Channel3->CMAR = (uint32_t)display; 
    DMA1_Channel3->CPAR = (uint32_t)&SPI1->DR; 
    DMA1_Channel3->CNDTR = 34; 
    DMA1_Channel3->CCR |= DMA_CCR_DIR;
    DMA1_Channel3->CCR |= DMA_CCR_MINC; 
    DMA1_Channel3->CCR |= DMA_CCR_MSIZE_0; 
    DMA1_Channel3->CCR |= DMA_CCR_PSIZE_0; 
    DMA1_Channel3->CCR |= DMA_CCR_CIRC; 
    
}

//===========================================================================
// Enable the DMA channel triggered by SPI1_TX.
//===========================================================================
void spi1_enable_dma(void) {
       DMA1_Channel3->CCR |= DMA_CCR_EN; 
    
}

//===========================================================================
// Main function
//===========================================================================

int main(void) {
    internal_clock();

    msg[0] |= font['E'];
    msg[1] |= font['C'];
    msg[2] |= font['E'];
    msg[3] |= font[' '];
    msg[4] |= font['3'];
    msg[5] |= font['6'];
    msg[6] |= font['2'];
    msg[7] |= font[' '];

    // GPIO enable
    enable_ports();
    // setup keyboard
    // init_tim7();

    // LED array Bit Bang
//  #define BIT_BANG
#if defined(BIT_BANG)
    setup_bb();
    drive_bb();
#endif

    // Direct SPI peripheral to drive LED display
//#define SPI_LEDS
#if defined(SPI_LEDS)
    init_spi2();
    spi2_setup_dma();
    spi2_enable_dma();
    init_tim15();
    show_keys();
#endif

    // LED array SPI
// #define SPI_LEDS_DMA
#if defined(SPI_LEDS_DMA)
    init_spi2();
    spi2_setup_dma();
    spi2_enable_dma();
    show_keys();
#endif

    // SPI OLED direct drive
//#define SPI_OLED
#if defined(SPI_OLED)
    init_spi1();
    spi1_init_oled();
    spi1_display1("Hello again,");
    spi1_display2(username);
#endif

    // SPI
//#define SPI_OLED_DMA
#if defined(SPI_OLED_DMA)
    init_spi1();
    spi1_init_oled();
    spi1_setup_dma();
    spi1_enable_dma();
#endif

    // Uncomment when you are ready to generate a code.
     autotest();

    // Game on!  The goal is to score 100 points.
    game();
}
=======
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
 * @brief Init GPIO port B
 *        Pin 0: input
 *        Pin 4: input
 *        Pin 8-11: output
 *
 */
void initb() {
  
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
  GPIOB->MODER |= 0x550000;//00000000010101010000000000000000 //OUTPUT 
  GPIOB->MODER &= ~0x303;//0000000000000000000000001100000011  //INPUT 
                                 
}

/**
 * @brief Init GPIO port C
 *        Pin 0-3: inputs with internal pull down resistors
 *        Pin 4-7: outputs
 *
 */
void initc() {
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
  GPIOC->MODER &= ~0xFFFF;  //00000000000000001111111111111111 //INPUT 
  GPIOC->MODER |= 0x5500; //00000000000000000101010100000000 //OUTPUT
  // GPIOC->MODER &= ~0xFF;  //00000000000000000000000011111111 //INPUT 
  
  //PULL UP PULL DOWN PART 
  GPIOC->PUPDR |= 0xAA;  //00000000000000000000000010101010 //INPUT 
  
  
}

/**
 * @brief Set GPIO port B pin to some value
 *
 * @param pin_num: Pin number in GPIO B
 * @param val    : Pin value, if 0 then the
 *                 pin is set low, else set high
 */
void setn(int32_t pin_num, int32_t val) {
  
  if( val == 0){
    GPIOB->BRR |= 1<< pin_num;
  }
  
  else {
    GPIOB->BSRR |= 1<< pin_num; 
  }


}

/**
 * @brief Read GPIO port B pin values
 *
 * @param pin_num   : Pin number in GPIO B to be read
 * @return int32_t  : 1: the pin is high; 0: the pin is low
 */
int32_t readpin(int32_t pin_num) {
  int32_t var; 
  var = GPIOB->IDR & 1<< pin_num; 
  var = var>>pin_num; 
  return var;


  
}

/**
 * @brief Control LEDs with buttons
 *        Use PB0 value for PB8
 *        Use PB4 value for PB9
 *
 */
void buttons(void) {
  int32_t val0;
  val0= readpin(0);
  setn(8, val0);

   int32_t val4;
  val4= readpin(4);
  setn(9, val4);


  
}

/**
 * @brief Control LEDs with keypad
 * 
 */
void keypad(void) {
  int i = 0;
  
  while (i<4){
    GPIOC->ODR = 1<< (7-i); 
    nano_wait(1000000);
    int vara = GPIOC->IDR & 0xF;
    setn(i+8, vara & 1<< (3-i));
    i++; 

  }
  
}
////////////////gpio code done /////////////////////////////////////////////////////////////////////////////////////
/// @return // I DONT KNOW WHY THIS IS THERE 

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
>>>>>>> 0deaba4a008f0a7d0f783fca3c0c117e21b44a7a
