#include "ush.h"

static void check_flags(char **word, t_echo *echo, int *flags) {
    echo->E = false;
    echo->e = false;
    echo->n = false;
    bool E = false;
    bool e = false;
    bool n = false;
    bool flag = true;
    for (int i = 1; word[i]; i++) {
        if (word[i][0] == '-' && strlen(word[i]) > 1) {
            for (int j = 1; j < (int)strlen(word[i]) && flag; j++) {
                switch (word[i][j]) {
                    case 'E':
                        echo->E = true;
                        break;
                    case 'e':
                        echo->e = true;
                        break;
                    case 'n':
                        echo->n = true;
                        break;
                    default:
                        if (!E) echo->E = false;
                        if (!e) echo->e = false;
                        if (!n) echo->n = false;
                        flag = false;
                        break;
                }
            }
            if (flag)
                *flags = *flags + 1;
        }
        if (echo->E) E = true;
        if (echo->e) e = true;
        if (echo->n) n = true;
        flag = true;
    }
}

static bool env_var_handler(t_shell *shell, char **result, char **var, int quote) {
    char *var_value = NULL;
    if (strcmp(*var, "$") == 0) {
        pid_t curr_id = getpid();
        var_value = mx_itoa(curr_id);
    }
    else if (strcmp(*var, "?") == 0) {
        var_value = mx_itoa(shell->exit_code);
    }
    else {
        char *temp = getenv(*var);
        if (temp)
            var_value = strdup(temp);
    }
    mx_strdel(var);
    if (!var_value) {
        if (*result && !quote) {
            int len = strlen(*result);
            if ((*result)[len - 1] == ' ') {
                char *temp = strdup(*result);
                mx_strdel(result);
                *result = strndup(temp, len - 1);
                mx_strdel(&temp);
            }
            return false;
        }
        return true;
    }
    *result = mx_strrejoin(*result, var_value);
    mx_strdel(&var_value);
    return false;
}

void mx_echo(t_shell *shell) {
    char *shell_line = shell->bg ? mx_get_job_args(shell) : shell->command_now;
    char **words = mx_strsplit(shell_line, ' ');
    if (!words[1]) {
        mx_free_words(words);
        return;
    }

    t_echo *echo = malloc(sizeof(t_echo));
    int flags = 0, words_count = 0;
    check_flags(words, echo, &flags);
    while (words[++words_count]);
    words_count--;
    if (words_count == flags) {
        mx_free_words(words);
        free(echo);
        return;
    }
    if (words[flags + 1][0] == '~' && !words[flags + 2]) {
        char *temp = words[flags + 1];
        temp++;
        if (*temp == '+') {
            printf("%s", getenv("PWD"));
            temp++;
        }
        else if (*temp == '-') {
            printf("%s", getenv("OLDPWD"));
            temp++;
        }
        else
            printf("%s", getenv("HOME"));
        printf("%s\n", temp);
        mx_free_words(words);
        free(echo);
        return;
    }

    char sequenses[] = "abfnrtv";
    char escapes[] = "\a\b\f\n\r\t\v";
    int backslash = 0;

    int quote = 0;
    int brace1 = 0;
    int brace2 = 0;
    int bracket1 = 0;
    int bracket2 = 0;
    char *dollar_sequense = NULL;
    bool dollar = false;
    char *result = NULL;
    bool printable = false;
    bool skip_space = false;
    bool subecho = shell->subecho;

    char p[2];
    for (int i = flags + 1; words[i]; i++) {
        for (int j = 0; words[i][j]; j++) {
            if (words[i][j] == '\\') {
                if (backslash < 2)
                    backslash++;
                if (backslash == 2 && (!words[i][j + 1] || echo->E)) {
                    backslash = 0;
                    result = mx_strrejoin(result, "\\");
                    printable = true;
                }
                continue;
            }
            if (!echo->E) {
                if (backslash == 2 || (quote == 1 && backslash == 1)) {
                    bool correct = false;
                    for (int a = 0; sequenses[a]; a++) {
                        if (words[i][j] == sequenses[a]) {
                            MX_C_TO_P(escapes[a], p);
                            result = mx_strrejoin(result, p);
                            backslash = 0;
                            correct = true;
                            break;
                        }
                    }
                    if (!correct) {
                        result = mx_strrejoin(result, "\\");
                        MX_C_TO_P(words[i][j], p);
                        result = mx_strrejoin(result, p);
                        backslash = 0;
                        printable = true;
                    }
                    continue;
                }
            }
            if (words[i][j] == '"' || words[i][j] == '\'') {
                if (backslash == 1) {
                    MX_C_TO_P(words[i][j], p);
                    result = mx_strrejoin(result, p);
                    backslash = 0;
                    printable = true;
                    continue;
                }
                else {
                    quote++;
                    continue;
                }
            }
            if (words[i][j] == '$') {
                if (!dollar) {
                    dollar = true;
                    continue;
                }
            }
            if (dollar) {
                if (words[i][j] == '{') {
                    if (bracket1 == 0)
                        brace1++;
                    else {
                        MX_C_TO_P(words[i][j], p);
                        dollar_sequense = mx_strrejoin(dollar_sequense, p);
                    }
                    continue;
                }
                else if (words[i][j] == '(') {
                    if (bracket1 == 0)
                        bracket1++;
                    else {
                        bracket1++;
                        MX_C_TO_P(words[i][j], p);
                        dollar_sequense = mx_strrejoin(dollar_sequense, p);
                    }
                    continue;
                }
                else if (words[i][j] == '}') {
                    if (bracket1 == 0) {
                        brace2++;
                        if (brace1 != brace2 && words[i][j + 1] != '}') {
                            fprintf(stderr, "zsh: bad substitution\n");
                            mx_strdel(&result);
                            mx_strdel(&dollar_sequense);
                            mx_free_words(words);
                            free(echo);
                            shell->exit_code = EXIT_FAILURE;
                            return;
                        }
                        else if (words[i][j + 1] != '}') {
                            dollar = false;
                            brace1 = 0, brace2 = 0;
                            skip_space = env_var_handler(shell, &result, &dollar_sequense, quote);
                            printable = true;
                        }
                    }
                    else {
                        MX_C_TO_P(words[i][j], p);
                        dollar_sequense = mx_strrejoin(dollar_sequense, p);
                    }
                    continue;
                }
                else if (words[i][j] == ')') {
                    bracket2++;
                    if (bracket1 == 1) {
                        // if (bracket1 != bracket2) {
                        //     fprintf(stderr, "ush: parse error near `)'\n");
                        //     mx_strdel(&result);
                        //     mx_strdel(&dollar_sequense);
                        //     mx_free_words(words);
                        //     free(echo);
                        //     shell->exit_code = EXIT_FAILURE;
                        //     return;
                        // }
                        // else if (words[i][j + 1] != ')') {
                            mx_strdel(&shell->line);
                            shell->line = strdup(dollar_sequense);
                            shell->new_line = false;
                            shell->subecho = true;
                            mx_command_handler(shell);
                            shell->new_line = true;
                            bracket1 = 0, bracket2 = 0;
                            mx_strdel(&dollar_sequense);
                            dollar = false;
                            shell->subecho = subecho;
                        // }
                    }
                    else {
                        bracket1--, bracket2--;
                        MX_C_TO_P(words[i][j], p);
                        dollar_sequense = mx_strrejoin(dollar_sequense, p);
                    }
                    continue;
                }
                else {
                    MX_C_TO_P(words[i][j], p);
                    dollar_sequense = mx_strrejoin(dollar_sequense, p);
                    if (!words[i][j + 1] && !brace1 && !bracket1) {
                        dollar = false;
                        skip_space = env_var_handler(shell, &result, &dollar_sequense, quote);
                        printable = true;
                    }
                    continue;
                }
            }
            if (backslash == 1)
                backslash = 0;
            MX_C_TO_P(words[i][j], p);
            result = mx_strrejoin(result, p);
            printable = true;
        }
        if (words[i + 1]) {
            if (backslash == 1)
                backslash = 0;
            if (dollar)
                dollar_sequense = mx_strrejoin(dollar_sequense, " ");
            else if (!skip_space || (skip_space && quote))
                result = mx_strrejoin(result, " ");
            if (skip_space)
                skip_space = false;
        }
    }
    if (quote % 2 != 0) {
        printf("Odd number of quotes.\n");
        mx_free_words(words);
        mx_strdel(&result);
        free(echo);
        shell->exit_code = EXIT_FAILURE;
        return;
    }
    if (!result)
        result = strdup("");
    printf("%s", result);
    if (!subecho) {
        if (echo->n) {
            if (printable && isatty(0))
                printf("%s%s%%%s\n", MX_BLACK_F, MX_WHITE_B, MX_RESET);
        }
        else
            printf("\n");
    }

    mx_strdel(&result);
    mx_free_words(words);
    free(echo);
    shell->exit_code = EXIT_SUCCESS;
}
