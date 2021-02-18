#include "ush.h"

int main() {
    t_shell *shell = mx_shell_init();

    mx_loop(shell);

    tcsetattr(0, TCSANOW, &shell->backup);
    return EXIT_SUCCESS;
}
