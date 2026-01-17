#include "../include/hop.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/shell.h"

extern char home_dir[MAX_PATH_LENGTH]; // Declared in main.c
extern char prev_dir[MAX_PATH_LENGTH]; // Declared in main.c

void execute_hop(const AtomicCommand* cmd) {    
    // Case 1: 'hop' with no arguments is equivalent to 'hop ~'
    if (cmd->args[1] == NULL) {
        char current_dir_before_hop[MAX_PATH_LENGTH];
        if (getcwd(current_dir_before_hop, sizeof(current_dir_before_hop)) != NULL) {
            if (chdir(home_dir) == 0) { // Successfully changed to home
                strcpy(prev_dir, current_dir_before_hop);
            } else {
                fprintf(stderr, "No such directory!\n");
            }
        } else {
            // perror("getcwd");
        }
        return;
    }

    // Case 2: Process all arguments sequentially in a loop
    for (int i = 1; cmd->args[i] != NULL; i++) {
        const char* arg = cmd->args[i];
        char current_dir_before_change[MAX_PATH_LENGTH];
        const char* target_dir = NULL;

        if (getcwd(current_dir_before_change, sizeof(current_dir_before_change)) == NULL) {
            // perror("getcwd");
            return; // Exit on a critical error
        }

        if (strcmp(arg, "~") == 0) {
            target_dir = home_dir;
        } else if (strcmp(arg, "-") == 0) {
            if (prev_dir[0] == '\0') {
                return; // Stop processing on error
            }
            target_dir = prev_dir;
        } else {
            target_dir = arg; // Handles '.', '..', and any other path
        }

        // Attempt to change directory
        if (chdir(target_dir) != 0) {
            fprintf(stderr, "No such directory!\n");
            return; // Stop processing on the first error
        } else {
            // On success, update prev_dir with the path we were just in.
            strcpy(prev_dir, current_dir_before_change);
        }
    }
}