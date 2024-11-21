#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ff.h"
#include "cJSON.h"

typedef struct {
    char *username;
    int score;
} Username;

void loadUsernamesFromJSON(const char *filename, Username *users, int *user_count);
void saveUsernamesToJSON(const char *filename, Username *users, int user_count);