#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ff.h"        // FATFS library header
#include "cJSON.h"     // cJSON library header

FATFS fs;              // File system object
FIL file;              // File object
FRESULT fr;            // File operation result

// Function to read the contents of a file into a string using FATFS
char *fatfs_read_file(const char *filename) {
    char *content = NULL;
    UINT br; // Bytes read

    // Open the file for reading
    fr = f_open(&file, filename, FA_READ);
    if (fr != FR_OK) {
        printf("Failed to open file: %s (Error: %d)\n", filename, fr);
        return NULL;
    }

    // Get the file size
    DWORD size = f_size(&file);
    content = malloc(size + 1);
    if (!content) {
        printf("Failed to allocate memory\n");
        f_close(&file);
        return NULL;
    }

    // Read the file contents
    fr = f_read(&file, content, size, &br);
    if (fr != FR_OK || br != size) {
        printf("Failed to read file: %s (Error: %d)\n", filename, fr);
        free(content);
        f_close(&file);
        return NULL;
    }

    content[size] = '\0'; // Null-terminate the string
    f_close(&file);
    return content;
}

// Function to write a string to a file using FATFS
void fatfs_write_file(const char *filename, const char *content) {
    UINT bw; // Bytes written

    // Open the file for writing (create/overwrite)
    fr = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        printf("Failed to open file: %s (Error: %d)\n", filename, fr);
        return;
    }

    // Write the content to the file
    fr = f_write(&file, content, strlen(content), &bw);
    if (fr != FR_OK || bw != strlen(content)) {
        printf("Failed to write file: %s (Error: %d)\n", filename, fr);
    }

    f_close(&file);
}

int main() {
    const char *filename = "scores.json";

    // Mount the FATFS file system
    fr = f_mount(&fs, "", 1);
    if (fr != FR_OK) {
        printf("Failed to mount file system (Error: %d)\n", fr);
        return EXIT_FAILURE;
    }

    // Step 1: Read the JSON file
    char *json_data = fatfs_read_file(filename);
    if (!json_data) {
        return EXIT_FAILURE;
    }

    // Step 2: Parse the JSON data
    cJSON *json = cJSON_Parse(json_data);
    free(json_data); // Free the allocated memory for the file content
    if (!json) {
        printf("Error parsing JSON\n");
        return EXIT_FAILURE;
    }

    // Step 3: Update scores
    cJSON *user1_score = cJSON_GetObjectItem(json, "user1");
    if (user1_score && cJSON_IsNumber(user1_score)) {
        user1_score->valuedouble += 10; // Increment score for user1
    }

    cJSON *user2_score = cJSON_GetObjectItem(json, "user2");
    if (user2_score && cJSON_IsNumber(user2_score)) {
        user2_score->valuedouble += 20; // Increment score for user2
    }

    // Step 4: Serialize the updated JSON object back to a string
    char *updated_json_data = cJSON_Print(json);
    if (!updated_json_data) {
        printf("Failed to serialize JSON\n");
        cJSON_Delete(json);
        return EXIT_FAILURE;
    }

    // Step 5: Write the updated JSON back to the file
    fatfs_write_file(filename, updated_json_data);

    // Step 6: Clean up
    free(updated_json_data);
    cJSON_Delete(json);

    // Unmount the FATFS file system
    f_mount(NULL, "", 1);

    printf("Scores updated successfully!\n");
    return EXIT_SUCCESS;
}
