#include "cpu.h"
#include "rom.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  printf("main(): Initializing CPU\n");
  struct CPU cpu;
  cpu_init(&cpu);
  printf("main(): CPU initialized\n");

  /*
const size_t dummy_rom_size = 3;
const uint8_t dummy_rom_data[dummy_rom_size] = {0x80, 0x81, 0x82};

printf("main(): Creating dummy rom to pass to CPU\n");
dummy_rom(dummy_rom_data, dummy_rom_size, cpu.rom);
  */

  FILE *rom_file = fopen(argv[1], "rb");

  if (!rom_file) {
    perror("Error opening file");
    return 0;
  }

  load_rom(rom_file, cpu.rom);
  cpu_execute(&cpu, true, true);

  cpu_cleanup(&cpu);

  return 0;
}
