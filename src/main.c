/**
  ******************************************************************************
  * @file    main.c
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
