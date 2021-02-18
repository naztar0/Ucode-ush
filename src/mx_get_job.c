#include  "ush.h"

char *mx_get_job_args(t_shell *shell) {
    t_jobs_list *p = shell->jobs_list;
    for (; p->next; p = p->next);
    return p->job_args;
}

int mx_get_job_id(t_shell *shell) {
    t_jobs_list *p = shell->jobs_list;
    for (; p->next; p = p->next);
    return p->pid;
}

pid_t mx_get_pid_by_job_id(t_shell *shell, int job_id) {
    t_jobs_list *temp = shell->jobs_list;
    while(temp != NULL) {
        if(temp->job_number == job_id) {
            return temp->pid;
        }
        temp = temp->next;
    }
    return -1;
}
