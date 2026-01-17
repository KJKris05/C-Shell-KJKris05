#ifndef SHELL_H
#define SHELL_H

#include <sys/types.h> // For pid_t
#include <stdbool.h>   // For bool type

// --- Global Constants ---
#define MAX_PATH_LENGTH 1024
#define MAX_LOG_SIZE 15
#define MAX_JOBS 50

// --- Global Type Definitions ---
typedef enum {
    RUNNING,
    STOPPED
} JobState;

// --- Global Variable Declarations ---
extern pid_t foreground_pid;
extern char home_dir[MAX_PATH_LENGTH];
extern char prev_dir[MAX_PATH_LENGTH];

#endif // SHELL_H

