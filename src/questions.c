#include "questions.h"
#include "ff.h"

void listDirectory(const char *path) {
    DIR dir;           // Directory object
    FILINFO fileInfo;  // File information structure

    // Open the directory
    FRESULT res = f_opendir(&dir, path);
    if (res != FR_OK) {
        printf("Failed to open directory: %s\n", path);
        return;
    }

    printf("Contents of %s:\n", path);

    // Read each item in the directory
    while (1) {
        res = f_readdir(&dir, &fileInfo);
        if (res != FR_OK || fileInfo.fname[0] == 0) {
            // Break on error or end of directory
            break;
        }

        // Check if it's a directory
        if (fileInfo.fattrib & AM_DIR) {
            printf("[DIR] %s\n", fileInfo.fname);
        } else {
            printf("%s (Size: %lu bytes)\n", fileInfo.fname, fileInfo.fsize);
        }
    }

    // Close the directory
    f_closedir(&dir);
}

void loadQuestionsFromJSON(const char *filename, Question *questions, int *question_count) {
    /* Test for reading for regular file system */
    // FILE *file = fopen(filename, "r");
    // if (!file) {
    //     perror("Unable to open file");
    //     return;
    // }

    // fseek(file, 0, SEEK_END);
    // long length = ftell(file);
    // fseek(file, 0, SEEK_SET);

    // char *data = malloc(length + 1);
    // fread(data, 1, length, file);
    // fclose(file);
    // data[length] = '\0';

    /* Reading file using FATFS file system from SD card*/
    FIL file;
    FRESULT res;
    UINT bytesRead;

    // mount that shi
    FATFS FatFs;
    if (f_mount(&FatFs, "", 1 != FR_OK)) {
        printf("Couldn't mount the sd card.");
        return;
    }

    // Open the file
    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) {
        printf("Unable to open file.");
        return;
    }

    // Get file size and read
    DWORD fileSize = f_size(&file);
    char *data = malloc(fileSize + 1);
    res = f_read(&file, data, fileSize, &bytesRead);
    data[fileSize] = '\0';
    f_close(&file);

    // Parse json
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

void formatQuestionToString(char *question, size_t size, Question selected_question) {
    snprintf(question, size, 
        "Question: %s\n"
        "A: %s\n"
        "B: %s\n"
        "C: %s\n"
        "D: %s\n",
        selected_question.question,
        selected_question.choices[0],
        selected_question.choices[1],
        selected_question.choices[2],
        selected_question.choices[3]);

    // snprintf(question, size, 
    //     "Question: %s\n",
    //     selected_question.question);
}

char *printRandomQuestion(Question *questions, int question_count) {
    // if (question_count == 0) {
    //     printf("No questions available.\n");
    //     return NULL;
    // }

    //use sprintf to format string for it to be fed into lcd draw string 
    int random_index = rand() % question_count;
    Question selected_question = questions[random_index];

    // store everything in one string
    char *question = malloc(sizeof(char) * 500);
    // question = "Zohaib has no mustache";
    formatQuestionToString(question, 500, selected_question);

    // return string
    return question;

    // Print question to terminal
    // printf("Question: %s\n", selected_question.question);
    // printf("A: %s\n", selected_question.choices[0]);
    // printf("B: %s\n", selected_question.choices[1]);
    // printf("C: %s\n", selected_question.choices[2]);
    // printf("D: %s\n", selected_question.choices[3]);
}

// int main () {
//     srand(time(NULL));
//     Question questions[MAX_QUESTIONS];
//     int question_count;

//     loadQuestionsFromJSON("questions.json", questions, &question_count);
//     printRandomQuestion(questions, question_count);

//     return 0;
// }

