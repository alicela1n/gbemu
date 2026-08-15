#include "cpu.h"
#include "rom.h"
#include <stdio.h>

int main(int argc, char **argv) {
  // Testing the CPU
  printf("main(): Initializing CPU\n");
  struct CPU cpu;
  cpu_init(&cpu);
  printf("main(): CPU initialized\n");

  const size_t dummy_rom_size = 1;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0x80};
  struct ROM rom;

  printf("main(): Creating dummy rom to pass to CPU\n");
  dummy_rom(dummy_rom_data, dummy_rom_size, &rom);

  memcpy(cpu.mem, rom.data, 0x3FFF);

  cpu_execute(&cpu, &rom, true, true);

  return 0;
}
