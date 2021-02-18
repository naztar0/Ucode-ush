#include "ush.h"

void mx_kill(t_shell *shell) {
    char **words = mx_strsplit(shell->command_now, ' ');
    bool valid = true;
    for (int i = 1; words[i]; i++) {
        pid_t id = atoi(words[i]);
        if (id == 0) {
            shell->exit_code = EXIT_FAILURE;
            mx_free_words(words);
            return;
        }
        int res = kill(id, 9);
        if (res < 0) {
            valid = false;
            fprintf(stderr, "kill: kill %d failed: no such process\n", id);
        }
    }
    if (valid)
        shell->exit_code = EXIT_SUCCESS;
    else
        shell->exit_code = EXIT_FAILURE;
    mx_free_words(words);
}
