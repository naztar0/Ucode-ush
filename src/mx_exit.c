#include "ush.h"

void mx_exit(t_shell *shell) {
    char **words = mx_strsplit(shell->command_now, ' ');
    int words_count = 0;
    while (words[++words_count]);
    words_count--;
    if (words_count > 1) {
        fprintf(stderr, "exit: too many arguments\n");
        mx_free_words(words);
        shell->exit_code = EXIT_FAILURE;
        return;
    }
    int exit_code = EXIT_SUCCESS;
    if (words_count == 1)
        exit_code = atoi(words[1]);
    mx_free_words(words);
    mx_free_shell(shell);
    tcsetattr(0, TCSANOW, &shell->backup);
    exit(exit_code);
}
