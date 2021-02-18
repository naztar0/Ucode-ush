#include "ush.h"

void mx_print_error(char *command, char *error) {
    mx_printerr("ush: ");
    if (error)
        fprintf(stderr, "%s%s\n", command, error);
    else
        perror(command);
}

char *get_error(char **name, char *command) {
    char *error = NULL;

    if (strstr(command, "/")) {
        *name = command;
        struct stat buff;
        if (lstat(*name, &buff) >= 0 && mx_get_type(buff) == 'd') {
            error = strdup("is a directory: ");
        }
    }
    else
        error = strdup("command not found: ");
    return error;
}

bool read_dir(char *dir, char *command, char **name) {
    DIR *dptr = opendir(dir);
    struct dirent *ds;
    bool exists = false;

    if (dptr != NULL) {
        while ((ds = readdir(dptr)) != 0) {
            if (strcmp(ds->d_name, command) == 0 && command[0] != '.') {
                char *tmp = mx_strjoin(dir, "/");
                *name = mx_strjoin(tmp, command);
                free(tmp);
                exists = true;
                break;
            }
        }
        closedir(dptr);
    }
    return exists;
}

bool check_path(char **arr, char *command, char **name) {
    int i = 0;
    *name = NULL;
    bool bin_exist = false;
    char **command_splitted = mx_strsplit(command, '/');
    int words_count = 0;
    while (command_splitted[++words_count]);
    words_count--;

    while (arr[i] != NULL) {
        bin_exist = read_dir(arr[i], command_splitted[words_count], name);
        if (bin_exist)
            break;
        i++;
    }
    mx_free_words(command_splitted);
    return bin_exist;
}

void mx_builtin_child(char **job_path, char ***argv, bool give_env) {
    extern char **environ;
    execve(*job_path, *argv, give_env ? environ : NULL);
    free(*job_path);
    mx_free_words(*argv);
    exit(EXIT_FAILURE);
}
