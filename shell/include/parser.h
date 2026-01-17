#ifndef PARSER_H
#define PARSER_H

#define MAX_ARGS 64
#define MAX_CMDS_IN_PIPELINE 16
#define MAX_CMD_GROUPS 16

typedef struct AtomicCommand {
    char* args[MAX_ARGS];   // Arguments for the command, to be null-terminated
    char* input_redirect;   // Input redirection file   (<)
    char* output_redirect;  // Output redirection file (> or >>)
    int is_append;          // Flag for append mode (>>)    
} AtomicCommand;

typedef struct {
    AtomicCommand commands[MAX_CMDS_IN_PIPELINE]; // Array of commands in the pipeline
    int num_commands; // Number of commands in the pipeline
} CommandPipeline;

typedef struct {
    CommandPipeline groups[MAX_CMD_GROUPS]; // Array of command groups
    int num_groups; // Number of command groups
    char separators[MAX_CMD_GROUPS]; // Separators between command groups (e.g. ;, &)
    int background; // Flag for background execution (&)
} ShellCommand;

ShellCommand parse_command(char* input);

void free_shell_command(ShellCommand* cmd);

#endif // PARSER_H
