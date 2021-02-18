#include "ush.h"

static void check_flags(char **word, t_cd *cd, int *flags) {
    cd->P = false;
    cd->L = false;
    cd->s = false;
    for (int i = 1; word[i]; i++) {
        if (word[i][0] == '-' && strlen(word[i]) > 1) {
            for (int j = 1; j < (int)strlen(word[i]); j++) {
                switch (word[i][j]) {
                    case 'P':
                        cd->P = true;
                        break;
                    case 'L':
                        cd->L = true;
                        break;
                    case 's':
                        cd->s = true;
                        break;
                    default:
                        return;
                }
            }
            *flags = *flags + 1;
        }
    }
}

void mx_chdir(t_shell *shell) {
    char **words = mx_strsplit(shell->command_now, ' ');

    t_cd *cd = malloc(sizeof(t_cd));
    int flags = 0;
    check_flags(words, cd, &flags);

    char *path = NULL;
    // cd
    if (!words[1]) {
        path = strdup("~");
    }

    int quote = 0;
    bool end = false;

    char p[2];
    for (int i = flags + 1; words[i] && !end; i++) {
        for (int j = 0; words[i][j] && !end; j++) {
            if (words[i][j] == '"' || words[i][j] == '\'') {
                if (quote < 2) {
                    quote++;
                    continue;
                }
                end = true;
            }
            MX_C_TO_P(words[i][j], p);
            path = mx_strrejoin(path, p);
        }
        if (quote == 0)
            break;
        if (words[i + 1])
            path = mx_strrejoin(path, " ");
    }

    if (quote % 2 != 0) {
        fprintf(stderr, "Odd number of quotes.\n");
        mx_free_words(words);
        free(cd);
        shell->exit_code = EXIT_FAILURE;
        return;
    }

    char **path_split = mx_strsplit(path, '/');
    if (!path_split) {
        path_split = malloc(sizeof(char**));
        path_split[0] = strdup("/");
    }
    char *current_dir = getenv("PWD");
    char *destination = NULL;

    for (int i = 0; path_split[i]; i++) {
        if (strcmp(path_split[i], "/") == 0) {
            destination = strdup("/");
        }
        else if (strcmp(path_split[i], "~") == 0) {
            char *home = getenv("HOME");
            destination = mx_strrejoin(destination, home);
        }
        else if (strcmp(path_split[i], ".") == 0) {
            if (i == 0) {
                destination = strdup(current_dir);
            }
        }
        else if (strcmp(path_split[i], "..") == 0) {
            if (i == 0) {
                destination = strdup(current_dir);
            }

            char **folders = mx_strsplit(destination, '/');
            if (folders) {
                char *prev_dir = NULL;
                for (int a = 0; folders[a + 1]; a++) {
                    prev_dir = mx_strrejoin(prev_dir, "/");
                    for (int j = 0; folders[a][j]; j++) {
                        MX_C_TO_P(folders[a][j], p);
                        prev_dir = mx_strrejoin(prev_dir, p);
                    }
                }
                mx_strdel(&destination);
                destination = prev_dir ? strdup(prev_dir) : strdup("/");
                mx_strdel(&prev_dir);
                mx_free_words(folders);
            }
        }
        else if (strcmp(path_split[i], "-") == 0) {
            char *home = getenv("HOME");
            char *oldpwd = getenv("OLDPWD");
            char **folders = mx_strsplit(oldpwd, '/');
            char **homefolder = mx_strsplit(home, '/');
            char *prev_path = NULL;
            if (folders) {
                if (folders[0] && folders[1] && homefolder[0] && homefolder[1]) {
                    if (strcmp(folders[0], homefolder[0]) == 0 &&
                        strcmp(folders[1], homefolder[1]) == 0 &&
                        !path_split[1] && i == 0) {
                        prev_path = mx_strrejoin(prev_path, "~");
                        for (int a = 2; folders[a]; a++) {
                            prev_path = mx_strrejoin(prev_path, "/");
                            for (int j = 0; folders[a][j]; j++) {
                                MX_C_TO_P(folders[a][j], p);
                                prev_path = mx_strrejoin(prev_path, p);
                            }
                        }
                    }
                }
                if (prev_path) {
                    printf("%s\n", prev_path);
                    mx_strdel(&prev_path);
                    prev_path = mx_strdup(oldpwd);
                }
                else {
                    prev_path = mx_strdup(oldpwd);
                    printf("%s\n", prev_path);
                }
            }
            else {
                prev_path = mx_strdup("/");
                printf("%s\n", prev_path);
            }
            mx_strdel(&destination);
            destination = strdup(prev_path);
            mx_strdel(&prev_path);
            mx_free_words(folders);
            mx_free_words(homefolder);
        }
        else if (strcmp(path_split[i], "~+") == 0) {
            if (i == 0)
                destination = strdup(getenv("PWD"));
        }
        else if (strcmp(path_split[i], "~-") == 0) {
            if (i == 0)
                destination = strdup(getenv("OLDPWD"));
        }
        else {
            // cd ~user
            int j = 0;
            if (path_split[i][0] == '~' && i == 0) {
                char *home = getenv("HOME");
                char **homefolder = mx_strsplit(home, '/');
                destination = mx_strrejoin(destination, "/");
                destination = mx_strrejoin(destination, homefolder[0]);
                mx_free_words(homefolder);
                j = 1;
            }
            else {
                if (i == 0) {
                    destination = strdup(current_dir);
                }
            }
            if (strcmp(current_dir, "/") != 0)
                destination = mx_strrejoin(destination, "/");
            // cd dir
            for (; path_split[i][j]; j++) {
                MX_C_TO_P(path_split[i][j], p);
                destination = mx_strrejoin(destination, p);
            }
        }
    }


    DIR *dir = opendir(destination);
    if (dir) {
        if (cd->P) {
            char *real_path = realpath(destination, NULL);
            if (real_path) {
                mx_strdel(&destination);
                destination = mx_strdup(real_path);
                mx_strdel(&real_path);
            }
        }
        setenv("OLDPWD", current_dir, 1);
        setenv("PWD", destination, 1);
    }
    else {
        fprintf(stderr, "ush: chdir: %s: No such file or directory\n", path);
        mx_strdel(&path);
        mx_strdel(&destination);
        mx_free_words(path_split);
        free(cd);
        shell->exit_code = EXIT_FAILURE;
        return;
    }
    mx_strdel(&path);
    mx_strdel(&destination);
    mx_free_words(path_split);
    free(cd);
    shell->exit_code = EXIT_SUCCESS;
}
