#include "cpu.h"
#include "rom.h"
#include <cstdio>

int main(int argc, char **argv) {
  // Testing the CPU
  printf("main(): Initializing CPU\n");
  CPU cpu;
  cpu_init(&cpu);
  printf("main(): CPU initialized\n");

  const size_t dummy_rom_size = 5;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0x11, 0x04, 0xAD, 0x03, 0x04};
  ROM rom;

  printf("main(): Creating dummy rom to pass to CPU\n");
  dummy_rom(dummy_rom_data, dummy_rom_size, &rom);

  cpu_execute(&cpu, &rom, true, true);

  return 0;
}
