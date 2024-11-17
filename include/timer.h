#include "stm32f0xx.h"
#include "questions.h"
#include "commands.h"
// #include <stdint.h>
// #include <string.h>
#include "lcd.h"

/* Timer 2 to handle 30 second countdown */

// volatile uint32_t countdown = 30;     // 30-second countdown
// volatile int question_active = 0;      // Whether a question is active
// int question_index = 0;                // Index of the current question
// Question questions[MAX_QUESTIONS];      // Array of questions
// int question_count = 0;                // Total number of questions

// // Key scanning
// // 16 history bytes.  Each byte represents the last 8 samples of a button.
// uint8_t hist[16];
// char queue[2];  // A two-entry queue of button press/release events.
// int qin;        // Which queue entry is next for input
// int qout;       // Which queue entry is next for output
// uint8_t col;

// const char keymap[] = "DCBA#9630852*741";


// void push_queue(int n);
// char pop_queue();
// void update_history(int c, int rows);
// void drive_column(int c);
// int read_rows();
// char get_key_event(void);
// char get_keypress();
// void show_keys(void);
// void load_next_question();
// void handle_keypad_input(char key);
// void start_game();
// void init_tim2(void);
// void TIM2_IRQHandler(void);

