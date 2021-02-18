#include  "ush.h"

static void put_alias_value(t_shell *shell, t_key_value *alias) {
    char *new_line = strdup(alias->value);
    char *line_copy = shell->line;
    for(unsigned long i = 0; i < strlen(alias->name); i++) {
        line_copy++;
    }
    new_line = mx_strrejoin(new_line, line_copy);
    free(shell->line);
    shell->line = new_line;
}

static void delete_node(t_shell *shell, pid_t pid) {
    t_jobs_list *p1 = NULL;
    t_jobs_list *p2 = shell->jobs_list;
    for(int i = 1; i <= shell->job_max_number; i++){
        if(p2->pid == pid){ //if the node is found
            free(p2->job_args);
            *(shell->jobs_counter) = *(shell->jobs_counter) - 1;
            if(i == 1){ //if desired node is first
                t_jobs_list *p1 = p2->next;
                free(p2);
                shell->jobs_list = p1;
            }
            else if(i < *(shell->jobs_counter)) { //if desired node is in the middle
                p1->next = p2->next;
                free(p2);
            }
            else { //if desired node is last
                free(p2);
                p1->next = NULL;
                shell->job_max_number = p1->job_number;
            }
            return;
        }
        else {
            p1 = p2;
            p2 = p2->next;
        }
    }
    //got here if there is no job with desired pid
    //TODO: error
}

static int add_node(t_shell *shell, int pid, char **line) {
    t_jobs_list *temp = malloc(sizeof(t_jobs_list));
    temp->next = NULL;
    temp->pid = pid;
    temp->job_args = *line; 
    if (!shell->jobs_list) {
        shell->jobs_list = temp;
        shell->jobs_list->job_number = 1;
        shell->job_max_number = 1;
    }
    else {
        if(*(shell->jobs_counter) == shell->job_max_number){
            shell->job_max_number++;
            temp->job_number = shell->job_max_number;
            t_jobs_list *p = shell->jobs_list;
            while(p->next) //finding the end node
                p = p->next;
            p->next = temp;
        }
        else{
            t_jobs_list *p1 = NULL;
            t_jobs_list *p2 = shell->jobs_list;
            for(int i = 1; i <= shell->job_max_number; i++) {
                if(p2->job_number != i){
                    //inserting job in list
                    if(p1 == NULL){ //if first is missing
                        shell->jobs_list = temp;
                        temp->job_number = 1;
                    }
                    else { 
                        p1->next = temp;
                        temp->job_number = i;
                    }
                    temp->next = p2;
                    break;
                }
                else{
                    p1 = p2;
                    p2 = p2->next;
                }
            }
        }
    }
    *(shell->jobs_counter) = *(shell->jobs_counter) + 1;
    return temp->job_number;
}

void mx_command_handler(t_shell *shell) {
    if (!shell->line) {
        printf("\n");
        return;
    }
    char **words = mx_strsplit(shell->line, ' ');
    if (!words) {
        mx_free_words(words);
        printf("\n");
        return;
    }
    mx_free_words(words);
    shell->bg = false;
    if (shell->line[shell->line_len - 1] == '&') {
        shell->bg = true;
    }
    char *builtins[] = MX_BUILTINS_ARRAY;


    int backslash = 0;
    int quote = 0;
    int amp = 0;
    int pipe = 0;
    bool oper_last = true;
    char *result = NULL;
    t_list *commands_queue = NULL;
    t_list *operators_queue = NULL;

    char p[2];
    for (int i = 0; shell->line[i]; i++) {
        if (shell->line[i] == '\\') {
            backslash++;
            result = mx_strrejoin(result, "\\");
            oper_last = false;
            continue;
        }
        if (shell->line[i] == '"' || shell->line[i] == '\'') {
            if (backslash % 2 == 1)
                quote++;
            MX_C_TO_P(shell->line[i], p);
            result = mx_strrejoin(result, p);
            backslash = 0;
            oper_last = false;
            continue;
        }
        if (shell->line[i] == ';') {
            if (backslash % 2 == 1 || quote % 2 != 0) {
                MX_C_TO_P(shell->line[i], p);
                result = mx_strrejoin(result, p);
                backslash = 0;
                oper_last = false;
            }
            else if (!oper_last) {
                char *res_copy = strdup(result);
                mx_push_back(&commands_queue, res_copy);
                mx_push_back(&operators_queue, ";");
                mx_strdel(&result);
                oper_last = true;
            }
            continue;
        }
        if (shell->line[i] == '&') {
            if (backslash % 2 == 1 || quote % 2 != 0) {
                MX_C_TO_P(shell->line[i], p);
                result = mx_strrejoin(result, p);
                backslash = 0;
                oper_last = false;
            }
            else {
                if (amp == 0)
                    amp++;
                else if (!oper_last) {
                    char *res_copy = strdup(result);
                    mx_push_back(&commands_queue, res_copy);
                    mx_push_back(&operators_queue, "&");
                    mx_strdel(&result);
                    oper_last = true;
                    amp = 0;
                }
            }
            continue;
        }
        if (shell->line[i] == '|') {
            if (backslash % 2 == 1 || quote % 2 != 0) {
                MX_C_TO_P(shell->line[i], p);
                result = mx_strrejoin(result, p);
                backslash = 0;
                oper_last = false;
            }
            else {
                if (pipe == 0)
                    pipe++;
                else if (!oper_last) {
                    char *res_copy = strdup(result);
                    mx_push_back(&commands_queue, res_copy);
                    mx_push_back(&operators_queue, "|");
                    mx_strdel(&result);
                    oper_last = true;
                    pipe = 0;
                }
            }
            continue;
        }
        MX_C_TO_P(shell->line[i], p);
        result = mx_strrejoin(result, p);
        oper_last = false;
    }
    mx_push_back(&commands_queue, result);
    mx_push_back(&operators_queue, ";");


    /*void (*functions[]) (t_shell*) = {
        &mx_alias, &mx_bg, &mx_cd, &mx_chdir, &mx_echo, &mx_env, &mx_exit, &mx_export,
        &mx_false, &mx_fg, &mx_jobs, &mx_kill, &mx_pwd, &mx_true, &mx_unset, &mx_which};*/

    void (*functions[]) (t_shell*) = {
        &mx_alias, NULL, &mx_cd, &mx_chdir, &mx_echo, &mx_env, &mx_exit, &mx_export,
        &mx_false, &mx_fg, NULL, &mx_kill, &mx_pwd, &mx_true, &mx_unset, &mx_which};

    t_list *list = commands_queue;
    t_list *operator = operators_queue;
    bool exec_next = true;
    if (!list)
        printf("\n");
    for (int num = 0; list && exec_next; list = list->next, operator = operator->next, num++) {
        shell->command_now = strdup(list->data);
        words = mx_strsplit(shell->command_now, ' ');

        if (shell->new_line && !num)
            printf("\n");
        int words_count = 0;
        while (words[++words_count]);
        t_key_value *copy = shell->aliases;
        bool found = true;
        if (!copy)
            found = false;
        while(copy) {
            char **copy_split = mx_strsplit(copy->name, ' ');
            int copy_name_count = 0;
            while (copy_split[++copy_name_count]);
            if (words_count != copy_name_count)
                found = false;
            else {
                for (int i = 0; words[i]; i++)
                    if (strcmp(words[i], copy_split[i]) != 0)
                        found = false;
            }
            mx_free_words(copy_split);
            if (found) {
                put_alias_value(shell, copy);
                break;
            }
            else
                copy = copy->next;
        }
        if (found) {
            mx_free_words(words);
            words = mx_strsplit(shell->line, ' ');
        }
        for (int i = 0; i < MX_BUILTINS_COUNT; i++) {
            if (strcmp(words[0], builtins[i]) == 0) {
                if (shell->bg) {
                    int *f = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
                    *f = -1;
                    pid_t pid = fork();
                    if (pid == 0 /* Child process */) {
                        char *fixed_line = strndup(shell->line, shell->line_len - 1);
                        *f = add_node(shell, pid, &fixed_line);
                        if (isatty(STDIN_FILENO)){  //Reacting to signals
                            open_to_signals();
                        }
                        printf("\033[2K");
                        functions[i](shell);
                        char *txtcopy = strdup(mx_get_job_args(shell));
                        delete_node(shell, pid);
                        printf("[%d]  %c done       %s\n", *f, '+', txtcopy);
                        free(txtcopy);
                        exit(shell->exit_code);
                    }
                    else if (pid < 0) {
                        exit(EXIT_FAILURE);
                    }
                    else /* Main process */ {
                        while (*f < 0);
                        printf("[%d] %d\n", *(shell->jobs_counter), pid);
                        break;
                    }
                }
                else {
                    functions[i](shell);
                    if (!strcmp(operator->data, "&") && shell->exit_code == EXIT_FAILURE)
                        exec_next = false;
                    if (!strcmp(operator->data, "|") && shell->exit_code == EXIT_SUCCESS)
                        exec_next = false;
                }
                break;
            }
            else if (i == MX_BUILTINS_COUNT - 1) {
                // printf("ush: command not found: %s\n", words[0]);
                // shell->exit_code = EXIT_FAILURE;
                mx_launch_other_builtin(shell);
            }
        }
        mx_strdel(&shell->command_now);
        mx_free_words(words);
    }
    mx_clear_list(&commands_queue);
}
