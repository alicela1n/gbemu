#include "cpu.h"
#include "rom.h"
#include "log.h"
#include <stdio.h>
#include <signal.h>

/* main(): Main function
 */
int main(int argc, char *argv[]) {
  printf("main(): Initializing CPU\n");
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);
  printf("main(): CPU initialized\n");

  if (argc < 2) {
    printf("main(): Not enough arguments!\nmain(): Exiting now!\n");
    return -1;
  }

  FILE *rom_file = fopen(argv[1], "rb");

  if (argc == 3) {
    FILE *log_file = fopen(argv[2], "w+");

    if (!log_file) {
      perror("Error opening log file!");
      return 0;
    }

    init_logs(cpu, log_file);
  } else {
    init_logs(cpu, NULL);
  }

  if (!rom_file) {
    perror("Error opening file");
    return 0;
  }

  load_rom(rom_file, cpu->rom);

  if (
  cpu_execute(cpu, true, true, false, false);
  cpu_cleanup(cpu);

  return 0;
}
