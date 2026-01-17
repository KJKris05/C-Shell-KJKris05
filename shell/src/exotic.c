#include "../include/exotic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <ctype.h>      // For isdigit
#include <stdbool.h>    // For bool type

#include "../include/jobs.h"
#include "../include/shell.h"
#include <sys/types.h>

// ####### LLM Generated Code begins #######

// --- Helper Functions (private to this file) ---

static int compare_jobs_by_name(const void* a, const void* b) {
    const JobInfo* job_a = (const JobInfo*)a;
    const JobInfo* job_b = (const JobInfo*)b;
    return strcmp(job_a->command_name, job_b->command_name);
}

// Checks if a string contains only digits.
static bool is_numeric(const char* str) {
    if (str == NULL || *str == '\0') return false;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) return false;
    }
    return true;
}

// --- Main Command Implementations ---

void execute_activities(const AtomicCommand* cmd) {
    (void)cmd;
    JobInfo all_jobs[MAX_JOBS];
    int num_jobs = get_all_jobs(all_jobs, MAX_JOBS);

    if (num_jobs == 0) return;

    qsort(all_jobs, num_jobs, sizeof(JobInfo), compare_jobs_by_name);

    for (int i = 0; i < num_jobs; i++) {
        const char* state_str = (all_jobs[i].state == RUNNING) ? "Running" : "Stopped";
        printf("[%d] : %s - %s\n", (int)all_jobs[i].pid, all_jobs[i].command_name, state_str);
    }
}

void execute_ping(const AtomicCommand* cmd) {
    if (cmd->args[1] == NULL || cmd->args[2] == NULL) {
        // fprintf(stderr, "ping: usage: ping <pid> <signal_number>\n");
        return;
    }

    // FIX: Validate that both arguments are numeric per test plan
    if (!is_numeric(cmd->args[1]) || !is_numeric(cmd->args[2])) {
        fprintf(stderr, "Invalid syntax!\n");
        return;
    }

    pid_t pid = atoi(cmd->args[1]);
    int signal_number = atoi(cmd->args[2]);
    int actual_signal = signal_number % 32;

    if (kill(pid, 0) == -1 && errno == ESRCH) {
        // FIX: Corrected error message to match test plan
        fprintf(stderr, "No such process found\n");
        return;
    }

    if (kill(pid, actual_signal) == 0) {
        printf("Sent signal %d to process with pid %d\n", actual_signal, (int)pid);
    } else {
        // perror("ping: kill failed");
    }
}

void execute_fg(const AtomicCommand* cmd) {
    if (cmd->args[1] == NULL) {
        // Note: The test plan is ambiguous here. This implements 'fg <job_number>'.
        // 'fg' with no arguments would require tracking the "most recent" job.
        fprintf(stderr, "Invalid Syntax!\n");
        return;
    }

    int job_num = atoi(cmd->args[1]);
    JobInfo* job = find_job_by_job_number(job_num);

    if (job == NULL) {
        // FIX: Corrected error message to match test plan
        fprintf(stderr, "No such job\n");
        return;
    }

    printf("%s\n", job->command_name);

    if (job->state == STOPPED) {
        if (kill(job->pid, SIGCONT) < 0) {
            // perror("fg: kill (SIGCONT) failed");
            return;
        }
    }

    remove_job_by_pid(job->pid);
    foreground_pid = job->pid;
    
    int status;
    waitpid(job->pid, &status, WUNTRACED);

    if (WIFSTOPPED(status)) {
        add_job(job->pid, job->command_name, STOPPED);
    }

    foreground_pid = 0;
}

void execute_bg(const AtomicCommand* cmd) {
    // --- FIX: Full implementation for the 'bg' command ---
    if (cmd->args[1] == NULL) {
        // fprintf(stderr, "bg: usage: bg <job_number>\n");
        return;
    }

    int job_num = atoi(cmd->args[1]);
    JobInfo* job = find_job_by_job_number(job_num);

    if (job == NULL) {
        // Corrected error message
        fprintf(stderr, "No such job\n");
        return;
    }

    if (job->state == RUNNING) {
        // Corrected error message
        fprintf(stderr, "Job already running\n");
        return;
    }
    
    // Send the continue signal to the stopped process
    if (kill(job->pid, SIGCONT) < 0) {
        // perror("bg: kill (SIGCONT) failed");
        return;
    }
    
    // Update the job's state in the internal list
    job->state = RUNNING; 
    
    // Print the required confirmation message
    printf("[%d] %s &\n", job->job_number, job->command_name);
}

// ####### LLM Generated Code ends #######