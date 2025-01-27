#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include <sys/types.h>

#define MAX_JOBS 20

typedef enum {
    RUNNING,
    STOPPED,
    DONE
} JobStatus;

typedef struct {
    int job_id;
    pid_t pgid;
    char **argv;
    JobStatus status;
    int is_background;
} Job;

void init_job_table(void);
int add_job(pid_t pgid, char **argv, int is_background);
void update_job_status(pid_t pgid, JobStatus status);
void remove_job(pid_t pgid);
void check_job_statuses(void);
void list_jobs(void);
Job *get_recent_job(int is_background);
int foreground_job(void);
int background_job(void);

#endif //JOB_CONTROL_H
