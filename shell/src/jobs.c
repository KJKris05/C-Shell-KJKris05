#include "../include/jobs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include "../include/shell.h"

// ######## LLM Generated Code begins #######

static JobInfo job_list[MAX_JOBS];
// why static ? because we want to limit the scope of job_list to this file only
// static int job_count = 0;
static int next_job_number = 1;

void add_job(pid_t pid, const char* command_name, JobState state){
    
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!job_list[i].is_active) { // Find the first free slot
            job_list[i].pid = pid;
            strncpy(job_list[i].command_name, command_name, sizeof(job_list[i].command_name) - 1);
            job_list[i].command_name[sizeof(job_list[i].command_name) - 1] = '\0';
            job_list[i].state = state;
            job_list[i].is_active = 1;
            job_list[i].job_number = next_job_number++;
            
            if (state == RUNNING) {
                fprintf(stderr, "[%d] %d\n", job_list[i].job_number, (int)pid);
            } else if (state == STOPPED) {
                // This message is for when a job is added directly in a stopped state (e.g., from Ctrl-Z)
                fprintf(stderr, "\n[%d] Stopped %s\n", job_list[i].job_number, job_list[i].command_name);
            }
            return;
        }
    }
    // fprintf(stderr, "shell: too many background jobs\n");
}

void reap_finished_jobs(void){
    for(int i=0; i<MAX_JOBS; i++){  // Changed from job_count to MAX_JOBS
        if(job_list[i].is_active){
            int status;
            // WNOHANG: return immediately even if process hasn't finished
            // WUNTRACED : reports on children that have been stopped
            pid_t result = waitpid(job_list[i].pid, &status, WNOHANG | WUNTRACED);

            if (result == job_list[i].pid) {
                if (WIFEXITED(status)) {
                    fprintf(stderr, "%s with pid %d exited normally\n", job_list[i].command_name, job_list[i].pid);
                    job_list[i].is_active = 0; // Free the slot
                } else if (WIFSIGNALED(status)) { // killed by signal
                     fprintf(stderr, "%s with pid %d exited abnormally\n", job_list[i].command_name, job_list[i].pid);
                    job_list[i].is_active = 0; // Free the slot
                } else if (WIFSTOPPED(status)) {
                    // The process was stopped by a signal (Ctrl-Z)
                    if(job_list[i].state!=STOPPED){
                        fprintf(stderr, "\n[%d] Stopped %s\n", job_list[i].job_number, job_list[i].command_name);
                        job_list[i].state = STOPPED; // Update the state
                    }
                }
            }

        }
    }
}

int get_active_jobs(JobInfo* job_info_array, int max_size){
    int active_count = 0;
    // Loop through our private, internal job_list
    for (int i = 0; i < MAX_JOBS && active_count < max_size; i++) {
        if (job_list[i].is_active) {
            // Copy the relevant info into the public-facing array provided by the caller
            job_info_array[active_count].pid = job_list[i].pid;
            strcpy(job_info_array[active_count].command_name, job_list[i].command_name);
            active_count++;
        }
    }
    return active_count;
}

int get_all_jobs(JobInfo* job_info_array, int max_size) {
    int count = 0;
    for (int i = 0; i < MAX_JOBS && count < max_size; i++) {
        if (job_list[i].is_active) {
            job_info_array[count].pid = job_list[i].pid;
            strcpy(job_info_array[count].command_name, job_list[i].command_name);
            job_info_array[count].state = job_list[i].state;
            job_info_array[count].job_number = job_list[i].job_number;
            count++;
        }
    }
    return count;
}

JobInfo* find_job_by_job_number(int job_num) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_list[i].is_active && job_list[i].job_number == job_num) {
            // We need to return a pointer to a JobInfo, not the internal BackgroundJob.
            // A static local variable is a simple way to do this.
            return (JobInfo*)&job_list[i];
        }
    }
    return NULL;
}

void remove_job_by_pid(pid_t pid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_list[i].is_active && job_list[i].pid == pid) {
            job_list[i].is_active = 0; // Mark the slot as free
            return;
        }
    }
}

void kill_all_jobs(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_list[i].is_active) {
            kill(job_list[i].pid, SIGKILL);
        }
    }
}

JobInfo* get_most_recent_job(void) {
    int max_job_number = 0;
    int max_index = -1;
    
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_list[i].is_active && job_list[i].job_number > max_job_number) {
            max_job_number = job_list[i].job_number;
            max_index = i;
        }
    }
    
    if (max_index == -1) return NULL;

    return (JobInfo*)&job_list[max_index];
}

void update_job_state(pid_t pid, JobState new_state) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_list[i].is_active && job_list[i].pid == pid) {
            job_list[i].state = new_state;
            break;
        }
    }
}

// ######## LLM Generated Code ends #######