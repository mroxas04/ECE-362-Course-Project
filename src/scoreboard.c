#include "scoreboard.h"

FATFS fs;              // File system object
FIL file;              // File object
FRESULT fr;            // File operation result

// Function to read JSON file using FATFS
char *fatfs_read_file(const char *filename) {
    UINT br; // Bytes read
    char *content = NULL;

    // Open the file for reading
    fr = f_open(&file, filename, FA_READ);
    if (fr != FR_OK) {
        printf("Failed to open file: %s (Error: %d)\n", filename, fr);
        return NULL;
    }

    // Get file size
    DWORD size = f_size(&file);
    content = malloc(size + 1);
    if (!content) {
        printf("Failed to allocate memory\n");
        f_close(&file);
        return NULL;
    }

    // Read file content
    fr = f_read(&file, content, size, &br);
    if (fr != FR_OK || br != size) {
        printf("Failed to read file: %s (Error: %d)\n", filename, fr);
        free(content);
        f_close(&file);
        return NULL;
    }
    content[size] = '\0'; // Null-terminate string
    f_close(&file);
    return content;
}

// Function to write JSON file using FATFS
void fatfs_write_file(const char *filename, const char *content) {
    UINT bw; // Bytes written

    // Open the file for writing (create/overwrite)
    fr = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        printf("Failed to open file for writing: %s (Error: %d)\n", filename, fr);
        return;
    }

    // Write content to file
    fr = f_write(&file, content, strlen(content), &bw);
    if (fr != FR_OK || bw != strlen(content)) {
        printf("Failed to write file: %s (Error: %d)\n", filename, fr);
    }

    f_close(&file);
}

// Load usernames from JSON
void loadUsernamesFromJSON(const char *filename, Username *users, int *user_count) {
    // Mount the FATFS file system
    fr = f_mount(&fs, "/", 1);
    if (fr != FR_OK) {
        printf("Failed to mount file system (Error: %d)\n", fr);
        return;
    }

    char *json_data = fatfs_read_file(filename);
    if (!json_data) return;

    cJSON *json = cJSON_Parse(json_data);
    free(json_data); // Free memory for file content
    if (!json) {
        printf("Error parsing JSON\n");
        return;
    }

    // Populate user array from JSON object
    *user_count = 0;
    cJSON *user_item;
    cJSON_ArrayForEach(user_item, json) {
        if (*user_count >= 10) break; // Limit to 10 users

        users[*user_count].username = strdup(user_item->string);
        users[*user_count].score = cJSON_GetNumberValue(user_item);
        (*user_count)++;
    }

    cJSON_Delete(json); // Free cJSON object

    f_unmount("/");
}

// Comparator function for sorting in descending order
int compareScores(const void *a, const void *b) {
    Username *userA = (Username *)a;
    Username *userB = (Username *)b;
    return userB->score - userA->score; // Descending order
}

// Save usernames back to JSON
char *saveUsernamesToJSON(const char *filename, Username *users, int user_count) {
    // Sort usernames by score in descending order
    qsort(users, user_count, sizeof(Username), compareScores);

    // Create a new JSON object
    cJSON *json = cJSON_CreateObject();

    // Add usernames and scores to JSON object
    for (int i = 0; i < user_count; i++) {
        cJSON_AddNumberToObject(json, users[i].username, users[i].score);
    }

    // Serialize JSON to string
    char *json_data = cJSON_Print(json);

    // Write JSON string to file
    fatfs_write_file(filename, json_data);

    // Clean up
    cJSON_Delete(json);
    free(json_data);

    return json_data;
}
