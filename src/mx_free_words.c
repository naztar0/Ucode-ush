#include "ush.h"

void mx_free_words(char** words) {
    if (words) {
        for (int i = 0; words[i]; i++)
            free(words[i]);
        free(words);
    }
}
