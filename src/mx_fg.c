#include "ush.h"

static void fg_send_signal(t_shell *shell, char **argv, int pid, int job_id) {
    if (kill(-pid, SIGCONT) < 0) {
        mx_printerr("fg: ");
        mx_printerr(argv[1]);
        mx_printerr(": no such job\n");
        return;
    }
    
    tcsetpgrp(STDIN_FILENO, pid);
    // mx_set_job_status(shell, job_id, MX_STATUS_CONTINUED);
    // mx_print_job_status(shell, job_id, 0);
    // status = mx_wait_job(shell, job_id);
    // if (mx_job_completed(shell, job_id))
    //     mx_remove_job(shell, job_id);
    t_jobs_list *temp = shell->jobs_list;
    while(temp != NULL) {
        if(temp->job_number == job_id)
            break;
        temp = temp->next;
    }

    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(STDIN_FILENO, getpid());
    signal(SIGTTOU, SIG_DFL);
    tcgetattr(STDIN_FILENO, &temp->tmodes);
    tcsetattr(STDIN_FILENO, TCSADRAIN, &temp->tmodes);
}

static int mx_check_arg(char **argv) {
    int job_id = -1;

    if(argv[1][0] == '%') {
        if(mx_isdigit(argv[1][1])) {
            if((job_id = atoi((argv[1] + 1))) < 1) {
                mx_printerr("fg: ");
                mx_printerr(argv[1]);
                mx_printerr(": no such job\n");
                return -1;
            }
        }
        else{
            mx_printerr("fg: job not found: ");
            mx_printerr(argv[1] + 1);
            mx_printerr("\n");
            return -1;
        }
    }
    else {
        mx_printerr("fg: job not found: ");
        mx_printerr(argv[1]);
        mx_printerr("\n");
        return -1;
    }
    return job_id;
}

static int fg_get_job_id (t_shell *shell, char **p_argv) {
    int job_id;
    int p_args = 0;

    for (int i = 0; p_argv[i] != NULL; i++)
        p_args++;
    if (p_args > 2) {
        mx_printerr("ush: fg: too many arguments\n");
        return -1;
    }
    else if (p_args == 1) {
        if ((job_id = shell->job_max_number) < 1) {
            mx_printerr("fg: no current job\n");
            return -1;
        }
    }
    else {
        if ((job_id = mx_check_arg(p_argv)) < 1)
            return -1;
    }
    return job_id;
}

void mx_fg(t_shell *shell) {
    pid_t pid = 0;
    int job_id = 0;
    char **p_argv = mx_strsplit(shell->command_now, ' ');

    if ((job_id = fg_get_job_id(shell, p_argv)) < 1)
        return;
    if ((pid = mx_get_pid_by_job_id(shell, job_id)) < 1) {
        mx_printerr("fg: ");
        mx_printerr(p_argv[1]);
        mx_printerr(": no such job\n");
        return;
    }
    fg_send_signal(shell, p_argv, pid, job_id);
    return;
}

