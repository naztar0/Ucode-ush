#pragma once

// Colors
#define MX_RESET        "\x1B[0m"
#define MX_BLACK_F      "\x1B[30m"
#define MX_RED_F        "\x1B[31m"
#define MX_GREEN_F      "\x1B[32m"
#define MX_YELLOW_F     "\x1B[33m"
#define MX_BLUE_F       "\x1B[34m"
#define MX_MAGENTA_F    "\x1B[35m"
#define MX_CYAN_F       "\x1B[36m"
#define MX_WHITE_F      "\x1B[37m"
#define MX_BLACK_B      "\x1B[40m"
#define MX_RED_B        "\x1B[41m"
#define MX_GREEN_B      "\x1B[42m"
#define MX_YELLOW_B     "\x1B[43m"
#define MX_BLUE_B       "\x1B[44m"
#define MX_MAGENTA_B    "\x1B[45m"
#define MX_CYAN_B       "\x1B[46m"
#define MX_WHITE_B      "\x1B[47m"
/* example COLOR "\x1B[0;30;41m" bold foreground background */

// Keyboard keys
#define MX_CTRL_C       3
#define MX_CTRL_D       4
#define MX_TAB          9
#define MX_ENTER        13
#define MX_CTRL_R       18
#define MX_BACKSPACE    127
#define MX_C_PROMPT     42946
#define MX_K_DOWN       4348699
#define MX_K_HOME       4741915
#define MX_P_UP         2117425947
#define MX_P_DOWN       2117491483

#define MX_CAR_RIGHT    "\033[1C"
#define MX_CAR_LEFT     "\033[1D"

// For export(temporarly)
#define MX_EXPORT       0
#define MX_VARIABLES    1

// For dir
#define MX_MAXDIR          0xff
#define MX_BUILTINS_COUNT  16
#define MX_BUILTINS_ARRAY  {"alias", "bg", "cd", "chdir", "echo", "env", "exit", "export",\
                            "false", "fg", "jobs", "kill", "pwd", "true", "unset", "which"}

// Histiry setup
#define MX_HISTORY_SIZE    100
#define MX_JOBS            100

#define MX_USH             "u$h> "

#define MX_C_TO_P(x, p)    p[0] = x; p[1] = '\0'

#define MX_STATUS_RUNNING 0
#define MX_STATUS_DONE 1
#define MX_STATUS_SUSPENDED 2
#define MX_STATUS_CONTINUED 3
#define MX_STAT_TERMINATED 4
#define MX_FILTER_ALL 0
#define MX_FILTER_DONE 1
#define MX_FILT_IN_PROGR 2
#define MX_FOREGROUND 1
#define MX_BACKGROUND 0
#define MAX_LEN 10
