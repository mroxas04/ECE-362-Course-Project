#include "questions.h"

void loadQuestionsFromJSON(const char *filename, Question *questions, int *question_count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Unable to open file");
        return;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = malloc(length + 1);
    fread(data, 1, length, file);
    fclose(file);
    data[length] = '\0';

    cJSON *json = cJSON_Parse(data);
    if (!json) {
        printf("Error parsing JSON\n");
        free(data);
        return;
    }

    *question_count = 0;
    cJSON *item;
    cJSON_ArrayForEach(item, json) {
        if (*question_count >= MAX_QUESTIONS) break;
        
        cJSON *qtext = cJSON_GetObjectItem(item, "question");
        cJSON *optA = cJSON_GetObjectItem(item, "A");
        cJSON *optB = cJSON_GetObjectItem(item, "B");
        cJSON *optC = cJSON_GetObjectItem(item, "C");
        cJSON *optD = cJSON_GetObjectItem(item, "D");
        cJSON *answer = cJSON_GetObjectItem(item, "answer");

        if (qtext && optA && optB && optC && optD && answer) {
            strncpy(questions[*question_count].question, qtext->valuestring, sizeof(questions[*question_count].question));
            strncpy(questions[*question_count].choices[0], optA->valuestring, sizeof(questions[*question_count].choices[0]));
            strncpy(questions[*question_count].choices[1], optB->valuestring, sizeof(questions[*question_count].choices[1]));
            strncpy(questions[*question_count].choices[2], optC->valuestring, sizeof(questions[*question_count].choices[2]));
            strncpy(questions[*question_count].choices[3], optD->valuestring, sizeof(questions[*question_count].choices[3]));

            // Map answer letter to index
            char answer_char = answer->valuestring[0];
            questions[*question_count].correct_answer = answer_char - 'A';

            (*question_count)++;
        }
    }

    cJSON_Delete(json);
    free(data);
}

void printRandomQuestion(Question *questions, int question_count) {
    if (question_count == 0) {
        printf("No questions available.\n");
        return;
    }

    int random_index = rand() % question_count;
    Question selected_question = questions[random_index];

    printf("Question: %s\n", selected_question.question);
    printf("A: %s\n", selected_question.choices[0]);
    printf("B: %s\n", selected_question.choices[1]);
    printf("C: %s\n", selected_question.choices[2]);
    printf("D: %s\n", selected_question.choices[3]);
}

// int main () {
//     srand(time(NULL));
//     Question questions[MAX_QUESTIONS];
//     int question_count;

//     loadQuestionsFromJSON("questions.json", questions, &question_count);
//     printRandomQuestion(questions, question_count);

//     return 0;
// }

