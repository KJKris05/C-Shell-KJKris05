#include "../include/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/shell.h"
#include "../include/parser.h"
#include "../include/executor.h"

extern char home_dir[MAX_PATH_LENGTH];

static char* logs[MAX_LOG_SIZE];
static int log_count = 0;
static const char* log_file = ".my_shell_logs";

void load_logs(void){

    char path[MAX_PATH_LENGTH];

    int len_path = snprintf(path, sizeof(path), "%s/%s", home_dir, log_file);

    // Check if snprintf failed (len < 0) or if the output was truncated.
    if (len_path < 0 || (size_t)len_path >= sizeof(path)) {
        // fprintf(stderr, "shell: history file path is too long to construct.\n");
        return;
    }

    FILE* file = fopen(path, "r");
    if(!file){
       // printf("Log file not found\n");
        return; // No logs to load
    }

    char* line = NULL;
    size_t len = 0;
    while (getline(&line, &len, file)!= -1 && log_count < MAX_LOG_SIZE) {
        line[strcspn(line, "\n")] = '\0'; // Remove newline character
        logs[log_count++] = strdup(line);
    }
    free(line);
    fclose(file);

}

void save_logs(void){
    char path[MAX_PATH_LENGTH];

    int len_path = snprintf(path, sizeof(path), "%s/%s", home_dir, log_file);

    // Check if snprintf failed (len < 0) or if the output was truncated.
    if (len_path < 0 || (size_t)len_path >= sizeof(path)) {
        // fprintf(stderr, "shell: history file path is too long to construct.\n");
        return;
    }

    FILE* file = fopen(path, "w");
    if(!file){
        // perror("fopen");
        return;
    }

    for(int i=0; i<log_count; i++){
        fprintf(file, "%s\n", logs[i]);
    }
    fclose(file);
}

void add_to_logs(const char* command){
    
    if(strncmp(command, "log", 3) == 0){
        return; // Don't log the 'log' command itself
    }

    if(log_count > 0 && strcmp(logs[log_count - 1], command) == 0){
        return; // Avoid duplicate consecutive entries
    }
    
    if(log_count == MAX_LOG_SIZE){
        // overwrite the oldest log
        free(logs[0]);
        memmove(&logs[0], &logs[1], sizeof(char*) * (MAX_LOG_SIZE - 1));
        logs[MAX_LOG_SIZE - 1] = strdup(command);

    } else {
        logs[log_count++] = strdup(command);
    }
}

void free_history(void){
    for(int i=0; i<log_count; i++){
        free(logs[i]);
    }
    log_count = 0;
}

const char* get_log_entry(int index){
    if(index < 0 || index >= log_count){
        return NULL;
    }
    return logs[index];
}

void execute_log(const AtomicCommand* cmd){
    if(cmd->args[1] == NULL){
        // case: log, print history
        for(int i=0; i<log_count; i++){
            printf("%s\n", logs[i]);
        }

    } else if(strcmp(cmd->args[1], "purge") == 0){
        free_history();
    
    } else if(strcmp(cmd->args[1], "execute") == 0){
        if(cmd->args[2] == NULL){
            // fprintf(stderr, "Usage: log execute <index>\n");
            return;
        }

        int user_index = atoi(cmd->args[2]);
        int actual_index = log_count - user_index;

        if(actual_index < 0 || actual_index >= log_count){
            fprintf(stderr, "Invalid log index: %d\n", user_index);
            return;
        }
        
        const char* command_to_execute = get_log_entry(actual_index);

        char* command_copy = strdup(command_to_execute);
        ShellCommand new_cmd = parse_command(command_copy);

        // sequential and background commands
        for (int i = 0; i < new_cmd.num_groups; i++) {
            const CommandPipeline* pipeline = &new_cmd.groups[i];
            int is_bg = (new_cmd.separators[i] == '&') || (i == new_cmd.num_groups - 1 && new_cmd.background); // explanation: if separator is '&' or if it's the last command and background flag is set
            execute_pipeline(pipeline, is_bg);
        }
        free(command_copy);
        free_shell_command(&new_cmd);

    } else {
        fprintf(stderr, "Unknown log command\n");
    }
}