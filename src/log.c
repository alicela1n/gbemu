#include "log.h"
#include "cpu.h"
 
/* init_logs(): Initialize the log file object
 */
void init_logs(struct CPU *cpu, FILE *log_file) {
  struct LOG *log;
  if (!cpu->log) {
    log = malloc(sizeof(struct LOG));
    memset(log, 0, sizeof(struct LOG));
    cpu->log = log;
  } else {
    log = cpu->log;
  }

  if (log->file) {
    fclose(log->file);
  }

  if (log_file) {
    log->file = log_file;

    log->logging = true;
  } else {
    log->logging = false;
    log->file = NULL;
  }

}

void log_cleanup(struct CPU *cpu) {
  struct LOG *log = cpu->log;
  if (log) {
    fclose(log->file);
  }

  free(log);
}

/* write_log_entry(): Write the current register state to a log file
 */
void write_log_entry(struct CPU *cpu) {
  struct LOG *log = cpu->log;
  if (log->logging) {
    fprintf(log->file, "A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X\n", cpu->registers[a], cpu->registers[f], cpu->registers[b], cpu->registers[c], cpu->registers[d], cpu->registers[e], cpu->registers[h], cpu->registers[l], cpu_read_reg16(cpu, sp), cpu_read_reg16(cpu, pc), cpu_read_reg16(cpu, pc), cpu_read_reg16(cpu, pc) + 1, cpu_read_reg16(cpu, pc) + 2, cpu_read_reg16(cpu, pc) + 3);
    fflush(log->file);
  }
}
