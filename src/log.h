#ifndef LOG_H
#define LOG_H

#include "helper.h"
#include <stdbool.h>

struct CPU;

struct LOG {
  bool logging;
  FILE * file;
};

void init_logs(struct CPU *cpu, FILE * log_file);
void write_log_entry(struct CPU *cpu);
void log_cleanup(struct CPU *cpu);

#endif
