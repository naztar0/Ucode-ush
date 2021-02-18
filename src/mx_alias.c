#include "ush.h"
/*
static char* find_name(char *line, int equal_index) {
    char border;
    int letters_count = 0;
    int count_index = equal_index - 1;
    //Deciding from which sign to copy
    switch(line[count_index]) {
        case ' ':
            return NULL;
            break;
        case '\'':
            border = '\'';
            count_index--;
            break;
        case '\"':
            border = '\"';
            count_index--;
            break;
        default:
            border = ' ';
            break;
    }

    //Counting number of letters in the name
    while(true) {
        if(count_index == 5){
            break;
        }
        if(line[count_index] != border) {
            letters_count++;
            count_index--;
        }
        else {
            break;
        }
    }

    //Copying the name
    if (letters_count == 0)
        return NULL;
    else {
        char *copy = line;
        for(int i = 0; line[i] != border; i++)
            copy++;
        copy++;
        return strndup(copy, letters_count);
    }

    return NULL;
}

static char* find_value(char *line, int equal_index) {
    char border;
    int letters_count = 0;
    int count_index = equal_index + 1;
    //Deciding from which sign to copy
    switch(line[count_index]) {
        case ' ':
            return NULL;
            break;
        case '=':
            return NULL;
            break;
        case '\'':
            border = '\'';
            count_index++;
            break;
        case '\"':
            border = '\"';
            count_index++;
            break;
        default:
            border = '=';
            break;
    }

    //Counting number of letters in the value
    while(true) {
        if(line[count_index] == '=')
            return NULL;
        if(line[count_index] == border)
            break;
        if(line[count_index])
            letters_count++;
        else
            break;
        count_index++;
    }

    //Copying the value
    if(letters_count == 0)
        return NULL;
    else {
        char *copy = line;
        for(int i = 0; i < equal_index; i++)
            copy++;
        copy++;
        if(border != '=')
            copy++;
        return strndup(copy, letters_count);
    }
    return NULL;
}
*/

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

void alias_push(t_shell *shell, char *name, char *value) {
    if(shell->aliases) {
        t_key_value *copy = shell->aliases;
        bool already_exists = false;
        if (strcmp(copy->name, name) == 0)
            already_exists = true;
        else {
            while(copy->next) {
                if (strcmp(copy->name, name) == 0) {
                    already_exists = true;
                    break;
                }
                copy = copy->next;
            }
        }
        if (already_exists) {
            mx_strdel(&copy->value);
            copy->value = strdup(value);
        }
        else {
            t_key_value *new_node = malloc(sizeof(t_key_value));
            new_node->name = strdup(name);
            new_node->value = strdup(value);
            new_node->next = NULL;
            copy->next = new_node;
        }
    }
    else {
        t_key_value *new_node = malloc(sizeof(t_key_value));
        new_node->name = strdup(name);
        new_node->value = strdup(value);
        new_node->next = NULL;
        shell->aliases = new_node;
    }
}

void mx_alias(t_shell *shell) {
    //stupid, works only with one equal sign
    char **words = mx_strsplit(shell->command_now, ' ');
    t_key_value *copy = shell->aliases;
    if(!words[1]) {
        while(copy) {
            printf("%s=%s\n", copy->name, copy->value);
            copy = copy->next;
        }
        mx_free_words(words);
        shell->exit_code = EXIT_SUCCESS;
        return;
    }


    bool valid = false;
    char *var = NULL;
    char *val = NULL;
    int quote = 0;
    int eq = 0;
    int skip_spaces = 0;
    char p[2];
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
                        else if (words[i][j] == '}') {
                            brace2++;
                            if (brace1 != brace2 && words[i][j + 1] != '}') {
                                fprintf(stderr, "zsh: bad substitution\n");
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
            alias_push(shell, var, val);
            valid = true;
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
    if (valid)
        shell->exit_code = EXIT_SUCCESS;
    else
        shell->exit_code = EXIT_FAILURE;

/*
    char *name = NULL;
    char *value = NULL;
    bool valid = false;
    // переделать эту часть если оракл будет ругаться на тупую реализацию
    for(int equal_index_last = 6; shell->line[equal_index_last]; equal_index_last++) {
        if(shell->line[equal_index_last] == '=') {
            name = find_name(shell->line, equal_index_last);
            if(name != NULL)
                value = find_value(shell->line, equal_index_last);

            //Putting into the list
            alias_push(shell, name, value);
            if(name)
                free(name);
            if(value)
                free(value);

            // Checking if was put in
            // t_export *copy = shell->aliases;
            // while(copy->next != NULL) {
            //     copy = copy->next;
            // }
            // printf("value = %s\nname = %s\n", copy->value, copy->name);
            valid = true;
            break;
        }
    }
    if (valid)
        shell->exit_code = EXIT_SUCCESS;
    else
        shell->exit_code = EXIT_FAILURE;*/
    mx_free_words(words);
}
