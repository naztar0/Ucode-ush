#pragma once

typedef struct  s_key_value { // Struct for a list of exported variables
    char *name;
    char *value;
    struct s_key_value *next;
}               t_key_value;

typedef struct s_jobs_list {
    pid_t  pid;
    int    job_number;
    char  *job_args;
    struct termios tmodes;
    struct s_jobs_list *next;
}              t_jobs_list;

typedef struct s_shell {
    char **history;
    char **history_copy;
    int    history_count;
    int    history_now;

    char *line;
    char *line_temp;
    char *command_now;
    int   line_len;
    int   carriage_pos;
    bool  new_line;
    bool  bg;
    bool  give_env;
    bool  subecho;

    int         *jobs_counter;
    int          job_max_number;
    t_jobs_list *jobs_list;

    char *executable;                // Path to the shell executable
    char *pwd;                       // Path to current working directory
    char *prompt_name;               // Prompt name

    t_key_value *aliases;

    struct termios backup;
    
    int exit_flag;                     // Defaults 0, check if you have suspended jobs
    int exit_code;                     // Return if exit
}             t_shell;

/* Executable function flags */
typedef struct s_echo {
    bool E, e, n;
}              t_echo;
typedef struct s_pwd {
    bool P, L;
}              t_pwd;
typedef struct s_cd {
    bool P, L, s;
}              t_cd;
typedef struct s_env {
    bool i, v, P, u;
}              t_env;
typedef struct s_which {
    bool a, s;
}              t_which;
