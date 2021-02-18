#include "ush.h"

// static void clear_line(int n) {
//     for (int i = 0; i < n; i++)
//         printf("\b");
//     for (int i = 0; i < n; i++)
//         printf(" ");
//     for (int i = 0; i < n; i++)
//         printf("\b");
// }

static void copy_history(t_shell *shell) {
    for(int i = 0; i < shell->history_count; i++) {
        if(shell->history_copy[i] != NULL)
            free(shell->history_copy[i]);
        shell->history_copy[i] = strdup(shell->history[i]);
    }
}

static void free_command(t_shell *shell) {
    if (shell->line) {
        free(shell->line);
        shell->line = NULL;
        shell->line_len = 0;
        shell->carriage_pos = 0;
    }
}

static void backspace(t_shell *shell) {
    if (shell->line == NULL)
        return;
    else if (strlen(shell->line) == 1 && shell->carriage_pos == 1) {
        free(shell->line);
        shell->line = NULL;
        shell->line_len -= 1;
        shell->carriage_pos -= 1;
        printf("\r\033[2K%s", MX_USH);
    }
    else if (shell->carriage_pos > 0) {
        for (int i = shell->carriage_pos - 1; i < shell->line_len; i++)
            shell->line[i] = shell->line[i + 1];
        shell->line_len -= 1;
        shell->carriage_pos -= 1;
        printf("\r\033[2K%s%s", MX_USH, shell->line);
        for(int i = shell->carriage_pos; i < shell->line_len; i++)
            printf("\033[1D");
    }
}

static void change_line(t_shell *shell, char *new_char, int line_len, int carriage_pos) {
    if (line_len == carriage_pos)
        shell->line = mx_strrejoin(shell->line, new_char);
    else {
        char *line = strdup(shell->line);
        free(shell->line);
        shell->line = mx_strnew(shell->line_len + 1);
        for (int i = 0; i < carriage_pos - 1; i++)
            shell->line[i] = line[i];
        shell->line = mx_strrejoin(shell->line, new_char);
        for (int i = carriage_pos - 1; i < line_len; i++)
            shell->line[i + 1] = line[i];
        mx_strdel(&line);
        printf("\r\033[2K%s%s", MX_USH, shell->line);
        for (int i = line_len; i > carriage_pos; i--)
            printf("\033[1D");
    }
    shell->line_len = line_len;
    shell->carriage_pos = carriage_pos;
}

static void save_command(t_shell *shell) {
    if (shell->history_count < MX_HISTORY_SIZE && shell->line) {
        shell->history[shell->history_count] = strdup(shell->line);
        shell->history_count++;
        shell->history_now = shell->history_count;
    }
}

static void *read_char(t_shell *shell) {
    struct termios tty_backup_my;
    char c, p[2];
    if (!isatty(0)) {
        tcsetattr(STDIN_FILENO, TCSANOW, &shell->backup);
        while (true) {
            c = getchar();
            if (c == 10 || c == -1)
                break;
            p[0] = c;
            p[1] = '\0';
            change_line(shell, p, shell->line_len + 1, shell->carriage_pos + 1);
        }
        shell->new_line = false;
        mx_command_handler(shell);
        free_command(shell);
        mx_free_shell(shell);
        exit(shell->exit_code);
    }
    c = getchar();
    switch (c) {
        case MX_CTRL_D:
            if (shell->line)
                return NULL;
            tcgetattr(STDIN_FILENO, &tty_backup_my);  
            tcsetattr(STDIN_FILENO, TCSANOW, &shell->backup);
            printf("\n");
            tcsetattr(STDIN_FILENO, TCSANOW, &tty_backup_my);
            return "\1";
        case MX_CTRL_C:
            tcgetattr(STDIN_FILENO, &tty_backup_my);  
            tcsetattr(STDIN_FILENO, TCSANOW, &shell->backup);
            free_command(shell);;
            printf("\n%s", MX_USH);
            tcsetattr(STDIN_FILENO, TCSANOW, &tty_backup_my);
            break;
        case MX_ENTER:
            tcgetattr(STDIN_FILENO, &tty_backup_my);  
            tcsetattr(STDIN_FILENO, TCSANOW, &shell->backup);
            save_command(shell);
            copy_history(shell);
            mx_command_handler(shell);
            free_command(shell);
            printf("%s", MX_USH);
            tcsetattr(STDIN_FILENO, TCSANOW, &tty_backup_my);
            break;
        case MX_BACKSPACE:
            backspace(shell);
            break;
        case MX_TAB:
            printf("\t");
            break;
        case '\033':
            getchar();
            c = getchar();
            switch (c) {
                case 'A':
                    if (shell->history_count > 0 && shell->history_now > 0) {
                        if(shell->history_copy[shell->history_now] != NULL) {
                            free(shell->history_copy[shell->history_now]);
                            shell->history_copy[shell->history_now] = NULL;
                        }
                        if(shell->line != NULL)
                            shell->history_copy[shell->history_now] = strdup(shell->line);
                        shell->history_now--;
                        printf("\r\033[2K%s%s", MX_USH, shell->history_copy[shell->history_now]);  // carriage return and clear line
                        if ((shell->history_now + 1) == shell->history_count) {
                            if(shell->line != NULL) {
                                shell->line_temp = strdup(shell->line);
                                free(shell->line);
                                shell->line = NULL;
                                shell->line_len = 0;
                            }
                        }
                        else {
                            free(shell->line);
                            shell->line = NULL;
                            shell->line_len = 0;
                        }
                        int h_line_len = strlen(shell->history_copy[shell->history_now]);
                        change_line(shell, shell->history_copy[shell->history_now], h_line_len, h_line_len);
                    }
                    break;
                case 'B':
                    if (shell->history_count > 0 && shell->history_now < shell->history_count - 1) {
                        if(shell->history_copy[shell->history_now] != NULL) {
                            free(shell->history_copy[shell->history_now]);
                            shell->history_copy[shell->history_now] = NULL;
                        }
                        if(shell->line != NULL)
                            shell->history_copy[shell->history_now] = strdup(shell->line);
                        shell->history_now++;
                        printf("\r\033[2K%s%s", MX_USH, shell->history_copy[shell->history_now]);
                        free(shell->line);
                        shell->line = NULL;
                        int h_line_len = strlen(shell->history_copy[shell->history_now]);
                        change_line(shell, shell->history_copy[shell->history_now], h_line_len, h_line_len);
                    }
                    else if (shell->history_count > 0 && shell->history_now == shell->history_count - 1) {
                        if(shell->history_copy[shell->history_now] != NULL) {
                            free(shell->history_copy[shell->history_now]);
                            shell->history_copy[shell->history_now] = NULL;
                        }
                        if(shell->line != NULL)
                            shell->history_copy[shell->history_now] = strdup(shell->line);
                        shell->history_now++;
                        if(shell->line_temp != NULL) {
                            printf("\r\033[2K%s%s", MX_USH, shell->line_temp);
                            shell->line = strdup(shell->line_temp);
                            shell->line_len = strlen(shell->line_temp);
                            shell->carriage_pos = shell->line_len;
                            free(shell->line_temp);
                            shell->line_temp = NULL;
                        }
                        else {
                            printf("\r\033[2K%s", MX_USH);
                            if(shell->line != NULL) {
                                free(shell->line);
                                shell->line = NULL;
                            }
                        }
                    }
                    break;
                case 'C':
                    if (shell->carriage_pos < shell->line_len) {
                        printf(MX_CAR_RIGHT);
                        shell->carriage_pos = shell->carriage_pos + 1;
                    }
                    break;
                case 'D':
                    if (shell->carriage_pos > 0) {
                        printf(MX_CAR_LEFT);
                        shell->carriage_pos = shell->carriage_pos - 1;
                    }
                    break;
            }
            break;
        default:
            printf("%c", c);
            p[0] = c;
            p[1] = '\0';
            change_line(shell, p, shell->line_len + 1, shell->carriage_pos + 1);
            break;
    }
    return NULL;
}

void mx_loop(t_shell *shell) {
    while (true) {
        if (read_char(shell)) {
            mx_free_shell(shell);
            return;
        }
    }
}
