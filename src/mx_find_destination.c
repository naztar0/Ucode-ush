#include "ush.h"

char *mx_find_destination(char *path, char *p) {
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
            if (i == 0)
                destination = strdup(current_dir);
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
    return destination;
}
