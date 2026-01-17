#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/prompt.h"
#include "../include/shell.h"

#define MAX_PATH_LENGTH 1024

void display_prompt(const char* home_dir) {
    char username[256];
    char hostname[256];
    char current_dir[MAX_PATH_LENGTH];
    char display_path[MAX_PATH_LENGTH];

    if(getlogin_r(username, sizeof(username)) != 0) { // it gets the username of user logged in
        // if getlogin_r fails, using a default username
        strcpy(username, "user");
    }

    if(gethostname(hostname, sizeof(hostname)) != 0) { // it gets the hostname of the system
        // if gethostname fails, using a default hostname
        strcpy(hostname, "localhost");
    }

    if(getcwd(current_dir, sizeof(current_dir)) == NULL) {
        //perror("getcwd");
        strcpy(current_dir, "?");
    }

    if(strcmp(current_dir, home_dir) == 0) {
        strcpy(display_path, "~");
    } else if(strstr(current_dir, home_dir) == current_dir) {
        snprintf(display_path, sizeof(display_path), "~%s", current_dir + strlen(home_dir)); 
    } else {
        strcpy(display_path, current_dir);
    }

    printf("<%s@%s:%s> ", username, hostname, display_path);
    fflush(stdout); // Ensure prompt is displayed immediately

}