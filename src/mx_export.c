#include "ush.h"

static void sort_vars(char **vars) {
    char *temp;
    int p = 1;
	while (p == 1) {
		p = 0;
		for (int i = 0; vars[i + 1]; i++) {
            if (strcmp(vars[i], vars[i + 1]) > 0) {
                temp = vars[i];
                vars[i] = vars[i + 1];
                vars[i + 1] = temp;
                p = 1;
            }
        }
	}
}

static void env_var_handler(t_shell *shell, char **result, char *var) {
    char *var_value = NULL;
    if (strcmp(var, "$") == 0) {
        pid_t curr_id = getpid();
        var_value = mx_itoa(curr_id);
    }
    else if (strcmp(var, "?") == 0) {
        var_value = mx_itoa(shell->exit_code);
    }
    else {
        var_value = strdup(getenv(var));
    }
    mx_strdel(&var);
    if (!var_value)
        return;
    *result = mx_strrejoin(*result, var_value);
    mx_strdel(&var_value);
}

void mx_export(t_shell *shell) {
    char **words = mx_strsplit(shell->command_now, ' ');
    bool fp = false;
    char p[2];
    if (words[1]) {
        if (words[1][0] == '-') {
            if (words[1][1] == 'p' && !words[1][2]) {
                fp = true;
            }
            else {
                fprintf(stderr, "export: bad option: %s\n", words[1]);
                mx_free_words(words);
                shell->exit_code = EXIT_FAILURE;
                return;
            }
        }
        else {
            char *var = NULL;
            char *val = NULL;
            int quote = 0;
            int eq = 0;
            int skip_spaces = 0;
            for (int a = 1; words[a] && quote % 2 == 0; a++) {
                if (quote % 2 == 0) {
                    for (int j = 0; words[a][j]; j++) {
                        if (words[a][j] == '=') {
                            eq = j + 1;
                            break;
                        }
                        MX_C_TO_P(words[a][j], p);
                        var = mx_strrejoin(var, p);
                    }
                }
                if (eq > 0 && words[a][eq]) {
                    char *sequenses = "abfnrtv";
                    char *escapes = "\a\b\f\n\r\t\v";
                    int backslash = 0;
                    int brace1 = 0;
                    int brace2 = 0;
                    int bracket1 = 0;
                    int bracket2 = 0;
                    bool dollar = false;
                    char *dollar_sequense = NULL;
                    for (int i = a; words[i]; i++) {
                        if (i > a) eq = 0;
                        for (int j = eq; words[i][j]; j++) {
                            if (backslash == 2) {
                                bool correct = false;
                                for (int k = 0; sequenses[k]; k++) {
                                    if (words[i][j] == sequenses[k]) {
                                        MX_C_TO_P(escapes[k], p);
                                        val = mx_strrejoin(val, p);
                                        backslash = 0;
                                        correct = true;
                                        break;
                                    }
                                }
                                if (!correct) {
                                    val = mx_strrejoin(val, "\\");
                                    MX_C_TO_P(words[i][j], p);
                                    val = mx_strrejoin(val, p);
                                    backslash = 0;
                                }
                                continue;
                            }
                            if (words[i][j] == '"' || words[i][j] == '\'') {
                                if (backslash == 1) {
                                    MX_C_TO_P(words[i][j], p);
                                    val = mx_strrejoin(val, p);
                                    backslash = 0;
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
                                else {
                                    dollar = false;
                                    continue;
                                }
                            }
                            if (dollar) {
                                if (words[i][j] == '{') {
                                    brace1++;
                                    continue;
                                }
                                else if (words[i][j] == '(') {
                                    bracket1++;
                                    continue;
                                }
                                else if (words[i][j] == '}') {
                                    brace2++;
                                    if (brace1 != brace2 && words[i][j + 1] != '}') {
                                        fprintf(stderr, "ush: bad substitution\n");
                                        mx_strdel(&dollar_sequense);
                                        mx_strdel(&var);
                                        mx_strdel(&val);
                                        mx_free_words(words);
                                        shell->exit_code = EXIT_FAILURE;
                                        return;
                                    }
                                    else if (words[i][j + 1] != '}') {
                                        dollar = false;
                                        env_var_handler(shell, &val, dollar_sequense);
                                    }
                                    continue;
                                }
                                else if (words[i][j] == ')') {
                                    bracket2++;
                                    if (bracket1 != bracket2 && words[i][j + 1] != ')') {
                                        fprintf(stderr, "ush: parse error near `)'\n");
                                        mx_strdel(&dollar_sequense);
                                        mx_strdel(&var);
                                        mx_strdel(&val);
                                        mx_free_words(words);
                                        shell->exit_code = EXIT_FAILURE;
                                        return;
                                    }
                                    else {
                                        mx_strdel(&shell->line);
                                        shell->line = strdup(dollar_sequense);
                                        shell->new_line = false;
                                        mx_command_handler(shell);
                                        shell->new_line = true;
                                        mx_strdel(&dollar_sequense);
                                        dollar = false;
                                    }
                                }
                                else {
                                    MX_C_TO_P(words[i][j], p);
                                    dollar_sequense = mx_strrejoin(dollar_sequense, p);
                                    if (!words[i][j + 1]) {
                                        dollar = false;
                                        env_var_handler(shell, &val, dollar_sequense);
                                    }
                                    continue;
                                }
                            }
                            MX_C_TO_P(words[i][j], p);
                            val = mx_strrejoin(val, p);
                        }
                        if (quote % 2 == 0) {
                            break;
                        }
                        if (words[i + 1]) {
                            val = mx_strrejoin(val, " ");
                            skip_spaces++;
                        }
                    }
                }
                if (quote % 2 == 0) {
                    // printf("%s -> %s\n", var, val);  // for test
                    if (!val) val = strdup("");
                    setenv(var, val, 1);
                    mx_strdel(&var);
                    mx_strdel(&val);
                    a += skip_spaces;
                    skip_spaces = 0;
                }
            }
            if (quote % 2 != 0) {
                fprintf(stderr, "Odd number of quotes.\n");
                mx_free_words(words);
                shell->exit_code = EXIT_FAILURE;
                return;
            }
        }
    }
    if (!words[1] || fp) {
        extern char **environ;
        int env_len = 0;
        for (; environ[env_len]; env_len++);
        char **vars = malloc(sizeof(char*) * env_len + 1);
        int k = 0;
        for (int i = 0; environ[i]; i++) {
            if (!(environ[i][0] == '_' && environ[i][1] == '='))  // exclude '_=/last/command'
                vars[k++] = strdup(environ[i]);
        }
        vars[k] = NULL;
        sort_vars(vars);

        for (int i = 0; vars[i]; i++) {
            if (fp)
                printf("export ");
            printf("%s\n", vars[i]);
        }
        mx_free_words(vars);
    }
    mx_free_words(words);
    shell->exit_code = EXIT_SUCCESS;
}
