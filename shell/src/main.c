#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "../include/shell.h"
#include "../include/prompt.h"
#include "../include/parser.h"
#include "../include/executor.h"
#include "../include/log.h"
#include "../include/jobs.h"

char home_dir[MAX_PATH_LENGTH];
char prev_dir[MAX_PATH_LENGTH] = ""; // To store the previous directory for 'cd -' command. Initialized to empty string
pid_t foreground_pid = 0;

static void handle_sigint(int signo){
    (void)signo; // unused
    if(foreground_pid!=0){ // if there's a foreground process
        kill(foreground_pid, SIGINT); // SIGINT is Ctrl-C
    }
}

static void handle_sigtstp(int signo) {
    (void)signo; // Unused
    if (foreground_pid != 0) { 
        kill(foreground_pid, SIGTSTP); // SIGTSTP is Ctrl-Z
    }
}

int main(){
    
    if(getcwd(home_dir, sizeof(home_dir))==NULL){
        perror("getcwd");
        return 1;

    }
    // ####### LLM Generated Code begins #######

    signal(SIGINT, handle_sigint);   // For Ctrl-C
    signal(SIGTSTP, handle_sigtstp); // For Ctrl-Z
    signal(SIGTTOU, SIG_IGN); // Important: Ignore signals from background processes trying to write to the terminal, by default they stop the process

    load_logs();
    atexit(free_history);   // when the shell terminates, logs is saved on the disk, and memory is freed, LIFO
    atexit(save_logs);

    while(1){
        
        reap_finished_jobs();
        display_prompt(home_dir);

        char* line = NULL;
        size_t len = 0;
        if(getline(&line, &len, stdin) == -1) { 
            // EOF (Ctrl+D)
            printf("\nlogout\n");
            kill_all_jobs();
            free(line);
            exit(EXIT_SUCCESS);
        }


        if(strlen(line) > 1) {

            char clean_line[strlen(line) + 1];
            strcpy(clean_line, line);
            clean_line[strcspn(clean_line, "\n")] = 0;

            ShellCommand cmd = parse_command(line);
            if(cmd.num_groups==-1){
                fprintf(stderr, "Invalid Syntax!\n");
            } else {
    
                if (strlen(clean_line) > 0) {
                    add_to_logs(clean_line);
                }
    
                for(int i=0; i<cmd.num_groups; i++){
                    // Sequential Execution of command groups
                    // background execution
                    int is_bg = (cmd.separators[i] == '&') || (i == cmd.num_groups - 1 && cmd.background);
                    execute_pipeline(&cmd.groups[i], is_bg);
                }
            }

            free_shell_command(&cmd);
        }

        free(line);

    }

    // ######## LLM Generated Code ends #######


    return 0;

}