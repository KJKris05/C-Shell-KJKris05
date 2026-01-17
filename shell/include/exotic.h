#ifndef EXOTIC_H
#define EXOTIC_H

#include "parser.h"

void execute_activities(const AtomicCommand* cmd);

void execute_ping(const AtomicCommand* cmd);

void execute_fg(const AtomicCommand* cmd);

void execute_bg(const AtomicCommand* cmd);

#endif  // EXOTIC_H