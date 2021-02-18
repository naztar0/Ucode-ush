#include "ush.h"

static void check_flags(char **word, t_pwd *pwd, int *flags) {
    pwd->P = false;
    pwd->L = false;
    for (int i = 1; word[i]; i++) {
        if (word[i][0] == '-' && strlen(word[i]) > 1) {
            for (int j = 1; j < (int)strlen(word[i]); j++) {
                switch (word[i][j]) {
                    case 'P':
                        pwd->P = true;
                        break;
                    case 'L':
                        pwd->L = true;
                        break;
                    default:
                        *flags = -1;
                        return;
                }
            }
        }
        else if (word[i][0] != '-') {
            *flags = -2;
            return;
        }
    }
    if (pwd->P) *flags = *flags + 1;
    if (pwd->L) *flags = *flags + 1;
}

void mx_pwd(t_shell *shell) {
    char *var_value = getenv("PWD");
    if (!var_value)
        return;
    char **words = mx_strsplit(shell->command_now, ' ');
    t_pwd *pwd = malloc(sizeof(t_pwd));
    int flags = 0;
    check_flags(words, pwd, &flags);
    if (flags == -1) {
        fprintf(stderr, "pwd: bad option: %s\n", words[1]);
        mx_free_words(words);
        free(pwd);
        shell->exit_code = EXIT_FAILURE;
        return;
    }
    if (flags == -2) {
        fprintf(stderr, "pwd: too many arguments\n");
        mx_free_words(words);
        free(pwd);
        shell->exit_code = EXIT_FAILURE;
        return;
    }

    if (pwd->P)
        var_value = realpath(var_value, NULL);
    printf("%s", var_value);
    if (!shell->subecho)
        printf("\n");
    mx_free_words(words);
    if (pwd->P)
        free(var_value);
    free(pwd);
    shell->exit_code = EXIT_SUCCESS;
}
