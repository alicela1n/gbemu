#include "memory.h"
#include "cpu.h"
#include "rom.h"

uint8_t read_memory8(struct CPU *cpu, struct ROM *rom, uint16_t addr) {
  uint8_t data = 0;
  if (addr <= 0x3FFF) {
    data = rom->data[addr];
  } else if (addr > 0x3FFF && addr <= 0x7FFF) {
    // TODO: Implement bank switching
    return 0;
  } else {
    data = cpu->mem[addr];
  }

  return data;
}

void write_memory8(struct CPU *cpu, uint8_t value, uint16_t addr) {
  if (addr <= 0x7FFF) {
    return; // Cartridge data is read only
  } else {
    cpu->mem[addr] = value;
  }
}
