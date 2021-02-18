#pragma once

#include "ushtd.h"

void mx_alias(t_shell*);
// void mx_bg(t_shell*);
void mx_cd(t_shell*);
void mx_chdir(t_shell*);
void mx_echo(t_shell*);
void mx_env(t_shell*);
void mx_exit(t_shell*);
void mx_export(t_shell*);
void mx_false(t_shell*);
void mx_fg(t_shell*);
// void mx_jobs(t_shell*);
void mx_kill(t_shell*);
void mx_pwd(t_shell*);
void mx_true(t_shell*);
void mx_unset(t_shell*);
void mx_which(t_shell*);

t_shell *mx_shell_init();
void     mx_loop(t_shell*);

void mx_command_handler(t_shell*);
void mx_free_words(char**);
void mx_free_shell(t_shell*);

char *mx_get_job_args(t_shell*);
int   mx_get_job_id(t_shell*);
pid_t mx_get_pid_by_job_id(t_shell*, int);
char *mx_find_destination(char*, char*);
void  mx_launch_other_builtin(t_shell*);
void  mx_print_error(char*, char*);
char  mx_get_type(struct stat);
void  mx_builtin_child(char**, char***, bool);
bool mx_help_launch_builtin(t_shell*, char**, char***);
char *get_error(char**, char*);
bool read_dir(char*, char*, char**);
bool check_path(char**, char*, char**);

void open_to_signals();
void close_to_signals();
