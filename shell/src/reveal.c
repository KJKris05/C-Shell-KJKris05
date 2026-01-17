#include "../include/reveal.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/shell.h"



static int compare(const void *a, const void *b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

extern char prev_dir[MAX_PATH_LENGTH]; // Declared in main.c
extern char home_dir[MAX_PATH_LENGTH]; // Declared in main.c

int isValid_a_l(const char* command){

    if(command[1]=='\0') return 1;

    for(int i=1; command[i]!='\0'; i++){
        if(command[i]!= 'a' && command[i] != 'l'){
            return 0;   // invalid character found
        }
    }

    return 1;
}

char** files_in_dir(const char* dir_path, int hidden_files){
    DIR* d = opendir(dir_path);
    if(d==NULL){
        printf("No such directory!\n");
        return NULL;
    }
    struct dirent* dir;
    char** files = malloc(100 * sizeof(char*));
    int count = 0;;

    while((dir = readdir(d))){
        if(hidden_files){
            files[count++] = strdup(dir->d_name); // Store the name of the entry            
        } else {
            if(dir->d_name[0] == '.'){
                continue; // Skip hidden files
            }
            files[count++] = strdup(dir->d_name); // Store the name of the entry            
        }
    }
    files[count]=NULL; // Null-terminate the array
    closedir(d); // Close the directory stream
    qsort(files, count, sizeof(char*), compare);
    return files;

}

static void free_file_list(char** files) {
    if (files == NULL) return;
    for (int i = 0; files[i] != NULL; i++) {
        free(files[i]); // Free each string from strdup
    }
    free(files); // Free the array itself
}

void print_files(char** files, int has_l){
    if(files == NULL) return;

    if(has_l){
        for(int i=0; files[i]!=NULL; i++){
            printf("%s\n", files[i]);
        }
    } else {
        for(int i=0; files[i]!=NULL; i++){
            printf("%s ", files[i]);
        }
        printf("\n");
    }

    free_file_list(files); // Free the allocated memory after use
}

void execute_reveal(const AtomicCommand* cmd) {
    bool show_all = false;
    bool long_format = false;
    const char* target_path_str = NULL;
    int path_arg_count = 0;

    // --- CORRECTED Argument Parsing Loop ---
    for (int i = 1; cmd->args[i] != NULL; i++) {
        // FIX: A command is only a flag if it starts with '-' AND has other characters.
        // The special command "-" is treated as a path.
        if (cmd->args[i][0] == '-' && cmd->args[i][1] != '\0') {
            for (int j = 1; cmd->args[i][j] != '\0'; j++) {
                if (cmd->args[i][j] == 'a') show_all = true;
                else if (cmd->args[i][j] == 'l') long_format = true;
                // Ignore invalid flag characters as per test behavior
            }
        } else {
            // This is now correctly identified as a path argument (e.g., ".", "..", "mydir", or "-")
            if (target_path_str != NULL) {
                // If we've already seen a path, this is the second one.
                path_arg_count++;
            }
            target_path_str = cmd->args[i];
            path_arg_count++;
        }
    }

    // Test requirement: 'reveal' with multiple path arguments is invalid.
    if (path_arg_count > 1) {
        fprintf(stderr, "Invalid Syntax!\n");
        return;
    }

    // --- Determine final target path ---
    const char* final_target_path = "."; // Default to current directory
    if (target_path_str != NULL) {
        if (strcmp(target_path_str, "-") == 0) {
            if (prev_dir[0] == '\0') {
                fprintf(stderr, "No such directory!\n");
                return;
            }
            final_target_path = prev_dir;
        } else if (strcmp(target_path_str, "~") == 0) {
            final_target_path = home_dir;
        } else {
            final_target_path = target_path_str;
        }
    }

    // --- Directory Reading and Listing ---
    DIR* dir = opendir(final_target_path);
    if (dir == NULL) {
        fprintf(stderr, "No such directory!\n");
        return;
    }

    size_t capacity = 10;
    size_t count = 0;
    char** files = malloc(capacity * sizeof(char*));
    if (files == NULL) { closedir(dir); return; }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!show_all && entry->d_name[0] == '.') {
            continue;
        }
        if (count >= capacity - 1) {
            capacity *= 2;
            char** temp = realloc(files, capacity * sizeof(char*));
            if (temp == NULL) { free_file_list(files); closedir(dir); return; }
            files = temp;
        }
        files[count] = strdup(entry->d_name);
        if (files[count] == NULL) { free_file_list(files); closedir(dir); return; }
        count++;
    }
    closedir(dir);
    files[count] = NULL;

    qsort(files, count, sizeof(char*), compare);

    // Printing Logic
    for (size_t i = 0; i < count; i++) {
        printf("%s%s", files[i], long_format ? "\n" : " ");
    }
    if (!long_format && count > 0) {
        printf("\n");
    }

    free_file_list(files);
}

