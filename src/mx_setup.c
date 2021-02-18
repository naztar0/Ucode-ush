#include "ush.h"

static void setup(t_shell *shell) {
    int term = STDIN_FILENO;
    pid_t pgid;

    if (isatty(term)) {
        while (tcgetpgrp(term) != (pgid = getpgrp()))
            kill(-pgid, SIGTTIN);

        close_to_signals();

        pgid = getpid();
        if (setpgid (pgid, pgid) < 0) {
            perror("Couldn't put the shell in its own process group");
            exit(EXIT_FAILURE);
        }

        tcsetpgrp(term, pgid);
        tcgetattr(term, &shell->backup);
    }


    struct termios tty;
    tty.c_lflag &= ~(ICANON|ECHO|ISIG|BRKINT|ICRNL
        |INPCK|ISTRIP|IXON|OPOST|IEXTEN);
    tty.c_cflag |= (CS8);
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 1;
    tcsetattr (term, TCSAFLUSH, &tty);
}

//INIT

static char *get_pwd() {
    char *pwd = getenv("PWD");
    char *real_path = realpath(pwd, NULL);
    char *cwd = getcwd(NULL, MX_MAXDIR);

    if (cwd && strcmp(cwd, real_path) == 0)
        pwd = strdup(getenv("PWD"));
    else
        pwd = strdup(cwd);
    free(cwd);
    free(real_path);
    return pwd;
}

t_shell *mx_shell_init() {
    t_shell *shell = (t_shell*)malloc(sizeof(t_shell));

    //#########################################################################################################
    //History init
    shell->history_count = 0;
    shell->history_now = 0;
    shell->history = malloc(sizeof(char*) * MX_HISTORY_SIZE);
    shell->history_copy = malloc(sizeof(char*) * MX_HISTORY_SIZE);

    //Jobs init
    shell->jobs_counter = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
    *(shell->jobs_counter) = 0;
    shell->jobs_list = NULL;
    shell->job_max_number = 0;

    //Setting path variables
    char *current_dir = getcwd(NULL, MX_MAXDIR);
    shell->executable = mx_strjoin(current_dir, "/ush");
    free(current_dir);
    if (!getenv("PATH")) {
        char *path = NULL;
        path = strdup("/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/munki");
        setenv("PATH", path, 1);
        free(path);
    }
    shell->pwd = get_pwd();
    setenv("PWD", shell->pwd, 1);
    setenv("OLDPWD", shell->pwd, 1);

    //Prompt variables
    // shell->prompt_name = strdup("u$h");

    //Setting level of shell
    char *shlvl = NULL;
    int lvl;
    shlvl = getenv("SHLVL");
    if (!shlvl)
        shlvl = "0";
    lvl = atoi(shlvl);
    lvl++;
    shlvl = mx_itoa(lvl);
    setenv("SHLVL", shlvl, 1);
    free(shlvl);

    //Line variables
    shell->line = NULL;
    shell->line_temp = NULL;
    shell->line_len = 0;
    shell->carriage_pos = 0;
    shell->new_line = true;
    shell->give_env = true;
    shell->subecho = false;
    //#########################################################################################################

    shell->exit_flag = EXIT_SUCCESS;
    shell->exit_code = EXIT_SUCCESS;

    if (isatty(0))
        mx_printstr(MX_USH);

    tcgetattr(0, &shell->backup);
    setup(shell);

    return shell;
}
