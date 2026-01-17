#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h"

// ####### LLM Generated Code begins #######

static void strip_quotes(char* str) {
    int len = strlen(str);
    if (len >= 2 && ((str[0] == '"' && str[len - 1] == '"') )) {
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';
    }
}

static AtomicCommand parse_atomic_command(char* command_str) {
    AtomicCommand cmd;
    memset(&cmd, 0, sizeof(AtomicCommand)); // Initializing to 0s

    char* saveptr;
    int arg_index = 0;

    // strtok_r is used to tokenize the input string, it takes parameters as the string to tokenize, the separators, and a pointer to save the context of the tokenization

    char* token = strtok_r(command_str, " \t", &saveptr);
    while(token != NULL){
        //Handling  input redirection
        if(strcmp(token, "<")==0){
            cmd.input_redirect = strtok_r(NULL, " \t", &saveptr);
        }

        //Handling output redirection (write)
        else if(strcmp(token, ">")==0){
            cmd.is_append = 0; // Default to overwrite mode
            cmd.output_redirect = strtok_r(NULL, " \t", &saveptr);
        }

        //Handling output redirection (append)
        else if(strcmp(token, ">>")==0){
            cmd.is_append = 1; // Set append mode
            cmd.output_redirect = strtok_r(NULL, " \t", &saveptr);
        }

        else {
            if(arg_index < MAX_ARGS - 1) { // Ensure we don't exceed the maximum number of arguments
                strip_quotes(token); // Remove surrounding quotes if any
                cmd.args[arg_index++] = token; // Store the argument
            }
        }
        token = strtok_r(NULL, " \t", &saveptr); // Get the next token
    }

    cmd.args[arg_index] = NULL; // Null-terminate the args array
    
    return cmd;

}

static CommandPipeline parse_command_pipeline(char* pipeline_str) {
    CommandPipeline pipeline;
    memset(&pipeline, 0, sizeof(CommandPipeline)); // Initializing to 0s
 
    char* saveptr;
    char* command_str = strtok_r(pipeline_str, "|", &saveptr);
    while(command_str != NULL && pipeline.num_commands < MAX_CMDS_IN_PIPELINE) {
        // Changes
        char* temp = command_str;
        while(*temp == ' ' || *temp == '\t') temp++;
        if (*temp == '\0') {
            pipeline.num_commands = -1; // Use a special value to indicate a syntax error
            return pipeline;
        }
        pipeline.commands[pipeline.num_commands++] = parse_atomic_command(temp);
        command_str = strtok_r(NULL, "|", &saveptr); // Get the next command in the pipeline
    }
    return pipeline;
}

ShellCommand parse_command(char* line){
    ShellCommand shell_cmd;
    memset(&shell_cmd, 0, sizeof(ShellCommand)); // Initializing to 0s
    
    line[strcspn(line, "\n")] = '\0'; // Remove trailing newline character
    // strcspn finds the first instance of "\n" in the string and replaces it with null terminator

    // check for background execution
    char* last_char = line + strlen(line) - 1;

    while (last_char > line && (*last_char == ' ' || *last_char == '\t')) {
        *last_char = '\0'; // Trim trailing whitespace
        last_char--;
    }

    if(*last_char == '&'){
        shell_cmd.background = 1;
        *last_char = '\0'; // Remove '&' from the command line to avoid interference with parsing
    }

    char* cursor = line;
    int group_index = 0;

    while(*cursor!='\0' && group_index < MAX_CMD_GROUPS){
        
        while(*cursor == ' ' || *cursor == '\t'){
            cursor++; // Skip leading whitespace
        }
        if(*cursor == '\0'){
            break; // End of line reached
        }

        size_t group_len = strcspn(cursor, ";&");   // length of current command group

        char separator = cursor[group_len];

        cursor[group_len] = '\0'; // Temporarily terminate the current command group string

        shell_cmd.groups[group_index] = parse_command_pipeline(cursor);
        if (shell_cmd.groups[group_index].num_commands == -1) {
            shell_cmd.num_groups = -1; // Propagate the error up
            return shell_cmd;
        }
        shell_cmd.separators[group_index] = separator;
        
        group_index++;

        if(separator == '\0'){
            break; // No more command groups, read the whole line
        } else {
            cursor += group_len + 1; // Move cursor to the start of the next command group
        } 
    }

    shell_cmd.num_groups = group_index;

    return shell_cmd;

}

void free_shell_command(ShellCommand* cmd) {
    // No dynamic memory to free in current implementation
    // If dynamic memory allocation is added in the future, it should be freed here
    (void)cmd; // To avoid unused parameter warning
}

// ####### LLM Generated Code ends #######