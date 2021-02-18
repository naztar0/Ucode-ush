#include "ush.h"

static void check_flags(char **word, t_which *which, int *flags) {
    which->a = false;
    which->s = false;
    for (int i = 1; word[i]; i++) {
        if (word[i][0] == '-' && strlen(word[i]) > 1) {
            for (int j = 1; j < (int)strlen(word[i]); j++) {
                switch (word[i][j]) {
                    case 'a':
                        which->a = true;
                        break;
                    case 's':
                        which->s = true;
                        break;
                    default:
                        return;
                }
            }
            *flags = *flags + 1;
        }
    }
}

void mx_which(t_shell *shell) {
    char **words = mx_strsplit(shell->command_now, ' ');

    t_which *which = malloc(sizeof(t_which));
    int flags = 0;
    check_flags(words, which, &flags);

    // which
    if (!words[1]) {
        mx_free_words(words);
        free(which);
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
    for (int i = flags + 1; words[i]; i++) {
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
    if (quote % 2 != 0) {
        fprintf(stderr, "Odd number of quotes.\n");
        mx_free_words(words_split);
        free(which);
        shell->exit_code = EXIT_FAILURE;
        return;
    }
    words = malloc(sizeof(char*) * (k + 1));
    for (int i = 0; words_split[i]; i++)  {
        words[i] = strdup(words_split[i]);
    }
    words[k] = NULL;
    mx_free_words(words_split);
    // END

    char *path = getenv("PATH");
    if (!path) path = "";
    char **path_split = mx_strsplit(path, ':');

    char *builtins[] = MX_BUILTINS_ARRAY;
    DIR *dir;
    struct dirent *dirent;
    bool exists;
    bool all_exists = true;
    for (int i = 0; words[i]; i++) {
        exists = false;
        for (int j = 0; j < MX_BUILTINS_COUNT; j++) {
            if (strcmp(words[i], builtins[j]) == 0) {
                exists = true;
                if (/*!which->s*/1)
                    printf("%s: ush built-in command\n", words[i]);
                break;
            }
        }
        if (exists && !(which->a))
            continue;
        for (int j = 0; path_split[j]; j++) {
            dir = opendir(path_split[j]);
            if (!dir)
                continue;
            while ((dirent = readdir(dir)) != 0) {
                if (strcmp(dirent->d_name, words[i]) == 0 && words[i][0] != '.') {
                    exists = true;
                    if (/*!which->s*/1) {
                        printf("%s/%s\n", path_split[j], dirent->d_name);
                    }
                    if (!which->a)
                        break;
                }
            }
            closedir(dir);
            if (!which->a && exists)
                break;
        }
        if (!exists) {
            all_exists = false;
            if (!which->s)
                printf("%s not found\n", words[i]);
        }
    }
    if (all_exists) {
        shell->exit_code = EXIT_SUCCESS;
    }
    else {
        shell->exit_code = EXIT_FAILURE;
    }
    mx_free_words(words);
    mx_free_words(path_split);
    free(which);
}
