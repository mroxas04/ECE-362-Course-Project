// #include "stm32f0xx.h"

// /* Timer 2 to handle 30 second countdown */

// volatile uint32_t countdown = 30;

// // Key scanning
// // 16 history bytes.  Each byte represents the last 8 samples of a button.
// uint8_t hist[16];
// char queue[2];  // A two-entry queue of button press/release events.
// int qin;        // Which queue entry is next for input
// int qout;       // Which queue entry is next for output
// uint8_t col;

// const char keymap[] = "DCBA#9630852*741";

// void push_queue(int n) {
//     queue[qin] = n;
//     qin ^= 1;
// }

// char pop_queue() {
//     char tmp = queue[qout];
//     queue[qout] = 0;
//     qout ^= 1;
//     return tmp;
// }

// void update_history(int c, int rows)
// {
//     // We used to make students do this in assembly language.
//     for(int i = 0; i < 4; i++) {
//         hist[4*c+i] = (hist[4*c+i]<<1) + ((rows>>i)&1);
//         if (hist[4*c+i] == 0x01)
//             push_queue(0x80 | keymap[4*c+i]);
//         if (hist[4*c+i] == 0xfe)
//             push_queue(keymap[4*c+i]);
//     }
// }

// void drive_column(int c)
// {
//     GPIOC->BSRR = 0xf00000 | ~(1 << (c + 4));
// }

// int read_rows()
// {
//     return (~GPIOC->IDR) & 0xf;
// }

// char get_key_event(void) {
//     for(;;) {
//         asm volatile ("wfi");   // wait for an interrupt
//        if (queue[qout] != 0)
//             break;
//     }
//     return pop_queue();
// }

// char get_keypress() {
//     char event;
//     for(;;) {
//         // Wait for every button event...
//         event = get_key_event();
//         // ...but ignore if it's a release.
//         if (event & 0x80)
//             break;
//     }
//     return event & 0x7f;
// }

// void show_keys(void)
// {
//     char buf[] = "        ";
//     for(;;) {
//         char event = get_key_event();
//         memmove(buf, &buf[1], 7);
//         buf[7] = event;
//         print(buf);
//     }
// }

// // Timer 
// void init_tim2(void) {
//     RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
//     TIM2->PSC = 48000 - 1;
//     TIM2->ARR = 1000 - 1;
//     TIM2->DIER |= TIM_DIER_UIE;
//     TIM2->CR1 |= TIM_CR1_CEN;
//     NVIC->ISER[0] = 1<<TIM2_IRQn;
// }

// void TIM2_IRQHandler(void){
//     // Acknowledge interrupt
// 	TIM2->SR &= ~TIM_SR_UIF;

//     // Decrement counter
// 	if (countdown > 0) countdown--;

//     // Reset counter when it hits 0
//     if (countdown == 0) {
//         countdown = 30;
//         TIM2->CNT = 0;
//         TIM2->CR1 |= TIM_CR1_CEN;
//     }

//     // Scan keys A, B, C, D for answer choices
//     int rows = read_rows();
// 	update_history(col, rows);
// 	col = (col + 1) & 3;
// 	drive_column(col);

//     // Add logic to reset the timer when the question is answered and display the next question

// }


/* Timer */

// void init_tim2(void) {
//     RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
//     TIM2->PSC = 48000 - 1;
//     TIM2->ARR = 1000 - 1;
//     TIM2->DIER |= TIM_DIER_UIE;
//     TIM2->CR1 |= TIM_CR1_CEN;
//     NVIC->ISER[0] = 1<<TIM2_IRQn;
// }

// void TIM2_IRQHandler(void) {
//     // Acknowledge interrupt
// 	TIM2->SR &= ~TIM_SR_UIF;

//     // Decrement counter
// 	if (countdown > 0) countdown--;

//     // Reset counter when it hits 0
//     if (countdown == 0 && question_active) {
//         question_active = 0;   // End the current question
//         question_index++;      // Move to the next question
//         LCD_Setup();
//         LCD_Clear(BLACK);
//         splitAndDisplayString("You lose.");
//     }

//     // Scan keys A, B, C, D for answer choices
//     int rows = read_rows();
// 	update_history(col, rows);
// 	col = (col + 1) & 3;
// 	drive_column(col);

//     // Handle keypad input after a key press event is detected
//     char key_event = get_key_event();
//     if (key_event != 0) {
//         handle_keypad_input(key_event & 0x7f);  // Extract the key without the event flag
//     }

//     // If countdown reaches zero and no key was pressed, display "You lose."
//     if (countdown == 0 && !question_active) {
//         LCD_Setup();
//         LCD_Clear(BLACK);
//         splitAndDisplayString("You lose.");
//     }
// }

// void init_tim2(void) {
//     RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
//     TIM2->PSC = 72000 - 1;
//     TIM2->ARR = 5 - 1;
//     TIM2->DIER |= TIM_DIER_UIE;
//     TIM2->CR1 |= TIM_CR1_CEN;
//     NVIC->ISER[0] = 1<<TIM2_IRQn;
// }

// void TIM2_IRQHandler() {
//     // Acknowledge interrupt
//     // if (TIM2->SR & TIM_SR_UIF) 
//     TIM2->SR &= ~TIM_SR_UIF;

//     // New question every 30 seconds
//     loadQuestionsFromJSON("qs_3.txt", questions, &question_count);
//     char *question = printRandomQuestion(questions, question_count, question_index);
//     LCD_Setup(); 
//     LCD_Clear(BLACK);
//     splitAndDisplayString(question); 
//     question_index++;

//     // Reset
//     TIM2->CNT = 0;
// }


/* Loading questions */

// Load questions from file and display the first question
// void load_next_question() {
//     if (question_index >= question_count) {
//         // No more questions, go to the end screen
//         LCD_Setup();
//         LCD_Clear(BLACK);
//         splitAndDisplayString("You win!");
//         return;
//     }

//     // Load the next question
//     char *question = printRandomQuestion(questions, question_count);
//     LCD_Setup(); 
//     LCD_Clear(BLACK);
//     splitAndDisplayString(question);
//     question_active = 1;  // Mark that a question is now active
//     countdown = 30;       // Reset the countdown
//     // TIM2->CNT = 0;        // Reset the timer counter
//     // TIM2->CR1 |= TIM_CR1_CEN;  // Start the timer
//     // init_tim2();
// }

// // Keypad event handling: reset timer on valid key press
// void handle_keypad_input(char key) {
//     // If a valid key (A, B, C, or D) is pressed, reset the timer and move to the next question
//     if (key == 'A' || key == 'B' || key == 'C' || key == 'D') {
//         countdown = 30;          // Reset the timer
//         TIM2->CNT = 0;           // Reset the timer counter
//         TIM2->CR1 |= TIM_CR1_CEN;  // Restart the timer
//         question_active = 0;      // Move on to the next question
//         question_index++;        // Increment to the next question
//         load_next_question();    // Load and display the next question
//     }
// }

// void start_game() {
//     loadQuestionsFromJSON("qs_3.txt", questions, &question_count);
//     load_next_question();  // Display the first question
// }
