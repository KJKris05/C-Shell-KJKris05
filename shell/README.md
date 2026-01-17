# C Shell

> **Note:** This was a project for the Operating Systems and Networks course, cloned here to my personal repository for reference and documentation.

## Overview

A custom Unix shell implementation in C that provides an interactive command-line interface with support for both built-in and system commands. The shell handles process management, I/O redirection, piping, and job control.

## Features

### Built-in Commands

- **hop**: Change directory functionality (similar to `cd`)
  - Supports relative and absolute paths
  - `hop ~` to return to home directory
  - `hop -` to switch to previous directory

- **reveal**: List directory contents (similar to `ls`)
  - Display files and directories with detailed information
  - Support for various flags and options

- **log**: Command history management
  - Stores last 15 commands
  - Persistent across shell sessions
  - Commands can be retrieved and executed from history

- **jobs**: Display all background processes
  - Shows process IDs and states (Running/Stopped)
  - Track up to 50 concurrent jobs

### Process Management

- **Foreground Execution**: Commands run in foreground by default
- **Background Execution**: Append `&` to run commands in background
- **Signal Handling**: 
  - Ctrl+C (SIGINT) to interrupt foreground processes
  - Ctrl+Z (SIGTSTP) to stop foreground processes
- **Job Control**: Track and manage multiple background processes

### Advanced Features

- **Piping**: Connect multiple commands with `|` operator
- **I/O Redirection**: 
  - Input redirection with `<`
  - Output redirection with `>` (overwrite)
  - Append redirection with `>>`
- **Command Chaining**: Execute multiple commands sequentially with `;`
- **Custom Prompt**: Dynamic prompt showing username, hostname, and current directory

## Building

```bash
cd shell
make
```

The executable `shell.out` will be created in the current directory.

## Running

```bash
./shell.out
```

## Cleaning

```bash
make clean
```

This removes all object files and the executable.

## Architecture

- **Parser**: Tokenizes input and builds command structures supporting pipes and redirections
- **Executor**: Handles command execution with fork/exec for system commands
- **Job Manager**: Tracks background processes and their states
- **Signal Handlers**: Manage Ctrl+C and Ctrl+Z for process control
- **Command History**: Persistent log system stored in `.myshell_history`
