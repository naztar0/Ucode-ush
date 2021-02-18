#include "ush.h"

void mx_unset(t_shell *shell) {
    char **words = mx_strsplit(shell->command_now, ' ');

    if (!words[1]) {
        fprintf(stderr, "unset: not enough arguments\n");
        mx_free_words(words);
        shell->exit_code = EXIT_FAILURE;
        return;
    }

    // BEGIN
    char p[2];
    int quote = 0;
    int words_count = 0;
    while (words[++words_count]);
    words_count--;
    char **words_split = malloc(sizeof(char*) * (words_count + 1));
    int k = 0;
    words_split[k] = NULL;
    for (int i = 1; words[i]; i++) {
        if (strcmp(words[i], "''") == 0 || strcmp(words[i], "\"\"") == 0)
            continue;
        for (int j = 0; words[i][j]; j++) {
            if (words[i][j] == '"' || words[i][j] == '\'') {
                if (quote < 2) {
                    quote++;
                    continue;
                }
            }
            MX_C_TO_P(words[i][j], p);
            words_split[k] = mx_strrejoin(words_split[k], p);
        }
        if (quote % 2 == 0) {
            words_split[++k] = NULL;
            quote = 0;
            continue;
        }
        if (words[i + 1])
            words_split[k] = mx_strrejoin(words_split[k], " ");
    }
    words_split[k] = NULL;
    mx_free_words(words);
    words = malloc(sizeof(char*) * (k + 1));
    for (int i = 0; words_split[i]; i++)  {
        words[i] = strdup(words_split[i]);
    }
    words[k] = NULL;
    mx_free_words(words_split);
    // END

    bool valid = true;
    for(int i = 0; words[i]; i++) {
        unsetenv(words[i]);
    }
    mx_free_words(words);
    if (valid)
        shell->exit_code = EXIT_SUCCESS;
}
