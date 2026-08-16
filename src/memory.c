#include "memory.h"
#include "rom.h"

uint8_t read_memory8(struct CPU *cpu, uint16_t addr) {
  uint8_t data = 0;
  if (addr <= 0x3FFF) {
    data = cpu->rom->data[addr];
  } else if (addr > 0x3FFF && addr <= 0x7FFF) {
    // TODO: Implement bank switching
    return 0;
  } else {
    data = cpu->mem[addr];
  }

  return data;
}

uint8_t *mem_ptr8(struct CPU *cpu, uint16_t addr) {
  if (addr <= 0x3FFF) {
    return &cpu->rom->data[addr];
  } else if (addr > 0x3FFF && addr <= 0x7FFF) {
    // TODO: Implement bank switching
    return NULL;
  } else {
    return &cpu->mem[addr];
  }
}

uint16_t read_memory16(struct CPU *cpu, uint16_t addr) {
  uint8_t a = read_memory8(cpu, addr + 1);
  uint8_t b = read_memory8(cpu, addr);
  return (a << 8) | b;
}

void write_memory8(struct CPU *cpu, uint8_t value, uint16_t addr) {
  if (addr <= 0x7FFF) {
    return; // Cartridge data is read only
  } else {
    cpu->mem[addr] = value;
  }
}
