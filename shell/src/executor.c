#include "../include/executor.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

#include "../include/shell.h"
#include "../include/reveal.h"
#include "../include/hop.h"
#include "../include/exotic.h"
#include "../include/log.h"
#include "../include/jobs.h"

// ###### LLM Generated Code Begins ######

static void execute_single_cmd_in_child(const AtomicCommand* cmd);

void execute_pipeline(const CommandPipeline* pipeline, int is_background){

    if(pipeline->num_commands==0){
        return;
    }

    // single command, no piping
    if(pipeline->num_commands==1){
        const AtomicCommand* cmd = &pipeline->commands[0];
        if(cmd->args[0] == NULL || cmd->args[0][0] == '\0'){
            return;
        }
        if(strcmp(cmd->args[0], "hop")==0){
            execute_hop(cmd);

        } else if(strcmp(cmd->args[0], "reveal")==0){
            execute_reveal(cmd);

        } else if(strcmp(cmd->args[0], "log")==0){
            // Handle 'log' command
            execute_log(cmd);
        
        } else if(strcmp(cmd->args[0], "activities") == 0){
            execute_activities(cmd);
        } else if(strcmp(cmd->args[0], "ping") == 0){
            execute_ping(cmd);
        } else if(strcmp(cmd->args[0], "fg") == 0){
            execute_fg(cmd);
        } else if(strcmp(cmd->args[0], "bg") == 0){
            execute_bg(cmd);
        
        } else {
            pid_t pid = fork();
            if (pid == -1) {
                // perror("fork");
                return;
            }
            if (pid == 0) {
                // Child process executes the command
                execute_single_cmd_in_child(cmd);
            } else {
                if (is_background) {
                    add_job(pid, cmd->args[0], RUNNING);
                } else {
                    foreground_pid = pid;
                    int status;
                    // Use WUNTRACED to detect if the child is stopped
                    waitpid(pid, &status, WUNTRACED);

                    // Check if the child was stopped by a signal (e.g., Ctrl-Z)
                    if (WIFSTOPPED(status)) {
                        add_job(pid, cmd->args[0], STOPPED);
                    }
                    foreground_pid = 0; // Clear the foreground pid
                }
            }
        }
        return;   
    }

    // multiple commands with piping
    int num_pipes = pipeline->num_commands - 1;
    int pipe_fds[num_pipes][2];
    pid_t pids[pipeline->num_commands];

    // creating pipes
    for(int i=0; i<num_pipes; i++){
        if(pipe(pipe_fds[i]) == -1){        // pipe_fds[i][0] for reading, pipe_fds[i][1] for writing
            // perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // forking a child
    for(int i=0; i<pipeline->num_commands; i++){
        pids[i] = fork();
        if(pids[i]<0){
            // perror("fork");
            exit(EXIT_FAILURE);
        
        } else if(pids[i]==0){
            // Child process
            // The shell must redirect the standard output of command[i] to the write end of pipe[i].
            if(i < num_pipes){  // not the last command
                if(dup2(pipe_fds[i][1], STDOUT_FILENO) < 0){
                    // perror("dup2 stdout");
                    exit(EXIT_FAILURE);
                }
            
            }
            
            // The shell must redirect the standard input of command[i+1] to the read end of pipe[i].
            if(i > 0){  // not the first command
                if(dup2(pipe_fds[i-1][0], STDIN_FILENO) < 0){
                    // perror("dup2 stdin");
                    exit(EXIT_FAILURE);
                }
            }

            for(int j=0; j<num_pipes; j++){
                close(pipe_fds[j][0]);
                close(pipe_fds[j][1]);
            }

            execute_single_cmd_in_child(&pipeline->commands[i]);
        }
    }

    // Parent process
    for(int i=0; i<num_pipes; i++){
        close(pipe_fds[i][0]);
        close(pipe_fds[i][1]);
    }

    if(is_background){
        // add first command and return immediately without waiting
        for (int i = 0; i < pipeline->num_commands; i++) {
            add_job(pids[i], pipeline->commands[i].args[0], RUNNING);
        }
    
    } else {

        foreground_pid = pids[pipeline->num_commands - 1];      // tracking the last command;

        int status;
        waitpid(foreground_pid, &status, WUNTRACED);

        for(int i=0; i<pipeline->num_commands; i++){

            if (WIFSTOPPED(status)) {
                // The last process was stopped; add all pipeline processes to the job list.
                for(int j=0; j<pipeline->num_commands; j++){
                    add_job(pids[j], pipeline->commands[j].args[0], STOPPED);
                }
            } else {
                // The last process terminated; clean up any other processes in the pipe.
                for(int i=0; i<pipeline->num_commands - 1; i++){
                    waitpid(pids[i], NULL, 0);
                }
            }
            foreground_pid = 0;
        }
    }
}

static void execute_single_cmd_in_child(const AtomicCommand* cmd){

    if(cmd->args[0] == NULL || cmd->args[0][0] == '\0'){
        exit(EXIT_SUCCESS);
    }
        
    //input redirection
            if(cmd->input_redirect){

                int input_fd = open(cmd->input_redirect, O_RDONLY);
                if(input_fd < 0){
                    perror(cmd->input_redirect);
                    exit(EXIT_FAILURE);
                }

                if(dup2(input_fd, STDIN_FILENO) < 0){
                    // perror("dup2 input");
                    close(input_fd);
                    exit(EXIT_FAILURE);
                }
                close(input_fd); // Close the original file descriptor after duplicating

            } 
            
            if(cmd->output_redirect){
                //output redirection
                int flags = O_WRONLY | O_CREAT;
                if(cmd->is_append){
                    flags |= O_APPEND; // Append mode
                } else {
                    flags |= O_TRUNC;  // Overwrite mode
                }

                int output_fd = open(cmd->output_redirect, flags, 0644);
                if(output_fd < 0){
                    perror(cmd->output_redirect);
                    exit(EXIT_FAILURE);
                }

                if(dup2(output_fd, STDOUT_FILENO) < 0){
                    // perror("dup2 output");
                    close(output_fd);
                    exit(EXIT_FAILURE);
                }

                close(output_fd); // Close the original file descriptor after duplicating

            }

            execvp(cmd->args[0], cmd->args);
            // If execvp returns, an error occurred
            fprintf(stderr, "Command not found!\n");
            exit(EXIT_FAILURE);
            
}


// ###### LLM Generated Code Ends ######