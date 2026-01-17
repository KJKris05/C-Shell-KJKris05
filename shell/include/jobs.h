#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>
#include "shell.h"

#define MAX_JOBS 50

typedef struct {
    pid_t pid;               // Process ID of the job
    char command_name[MAX_PATH_LENGTH];         // Command string
    JobState state;
    int job_number;
    int is_active;      // 1 if running, 0 if finished
} JobInfo;

void add_job(pid_t pid, const char* command, JobState state);

void reap_finished_jobs(void);

int get_all_jobs(JobInfo* job_info_array, int max_size);

JobInfo* find_job_by_job_number(int job_num);

void remove_job_by_pid(pid_t pid);

void kill_all_jobs(void);


#endif // JOBS_H