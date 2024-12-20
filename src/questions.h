#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "cJSON.h"

#define NUM_CHOICES 4
#define MAX_QUESTIONS 10

typedef struct {
    char question[500];  
    char choices[NUM_CHOICES][128]; 
    char correct_answer[5];
} Question;

void loadQuestionsFromJSON(const char *filename, Question *questions, int *question_count);
char * printRandomQuestion(Question *questions, int question_count, int question_index);
