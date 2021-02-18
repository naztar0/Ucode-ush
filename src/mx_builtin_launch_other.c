#include "ush.h"

void mx_launch_other_builtin(t_shell *shell) {
    char *job_path = NULL;
    char **argv = NULL;
    char *error = NULL;

    // printf("\033[1A\r\n"); // TODO
    if(mx_help_launch_builtin(shell, &job_path, &argv)) {
        pid_t child_pid;
        int shell_is_interactive = isatty(STDIN_FILENO);

        child_pid = fork();
        if (child_pid < 0) {
            perror("error fork");
            exit(EXIT_FAILURE);
        }
        else if (child_pid == 0){
            // open_to_signals();
            mx_builtin_child(&job_path, &argv, shell->give_env);
        }
        else {
            if (shell_is_interactive) {
                // setpgid (pid, m_s->jobs[job_id]->pgid);
                waitpid(child_pid, NULL, 0);
            }
        }
    }
    else {
        if(!job_path || !argv) {
            char **words = mx_strsplit(shell->command_now, ' ');
            mx_print_error("command not found: ", words[0]);
            mx_free_words(words);
            shell->exit_code = EXIT_FAILURE;
        }
        else {
            error = get_error(&job_path, argv[0]);
            mx_print_error(error, argv[0]);
            free(error);
            error = NULL;
            shell->exit_code = EXIT_FAILURE;
        }
    }
}
