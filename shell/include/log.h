#ifndef LOG_H
#define LOG_H

#include "parser.h"

void load_logs(void);

void save_logs(void);

const char* get_log_entry(int index);

void add_to_logs(const char* command);

void free_history(void);

void execute_log(const AtomicCommand* cmd);

#endif  // LOG_H