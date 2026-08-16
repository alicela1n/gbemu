#include "cpu.h"
#include "rom.h"
#include <stdio.h>

int main(int argc, char **argv) {
  printf("main(): Initializing CPU\n");
  struct CPU cpu;
  cpu_init(&cpu);
  printf("main(): CPU initialized\n");

  const size_t dummy_rom_size = 1;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0x80};

  printf("main(): Creating dummy rom to pass to CPU\n");
  dummy_rom(dummy_rom_data, dummy_rom_size, cpu.rom);

  memcpy(cpu.mem, cpu.rom->data, 0x3FFF);

  cpu_execute(&cpu, true, true);

  return 0;
}
