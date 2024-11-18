#include "questions.h"
#include "cJSON.h"

int main() {
    // What is the question
    int question_index = 0;                // Index of the current question
    Question questions[MAX_QUESTIONS];      // Array of questions
    int question_count = 0;                // Total number of questions
    loadQuestionsFromJSON("qs_3.txt", questions, &question_count);
    
    // Print string
    char wrong[50];
    char *right = "Correct";
    snprintf(wrong, sizeof(wrong), "Wrong. The right answer was %s\n", questions[question_index].correct_answer);
    printf("%s", right);

    return 0;
}