#include "memory.h"
#include "helper.h"
#include "rom.h"

uint8_t read_memory8(struct CPU *cpu, uint16_t addr) {
  uint8_t data = 0;
  if (addr <= 0x3FFF) {
    data = cpu->rom->data[addr];
  } else if (addr > 0x3FFF && addr <= 0x7FFF) {
    // TODO: Implement bank switching
    return 0;
  } else {
    data = cpu->mem[addr - 0x8000];
  }

  return data;
}

uint8_t *mem_ptr8(struct CPU *cpu, uint16_t addr) {
  if (addr <= 0x3FFF) {
    return &cpu->rom->data[addr - 0x8000];
  } else if (addr > 0x3FFF && addr <= 0x7FFF) {
    // TODO: Implement bank switching
    return NULL;
  } else {
    return &cpu->mem[addr - 0x8000];
  }
}

uint16_t read_memory16(struct CPU *cpu, uint16_t addr) {
  uint8_t low = read_memory8(cpu, addr);
  uint8_t high = read_memory8(cpu, addr + 1);
  return uint8to16(low, high);
}

void write_memory8(struct CPU *cpu, uint8_t value, uint16_t addr) {
  if (addr <= 0x7FFF) {
    return; // Cartridge data is read only
  } else {
    cpu->mem[addr - 0x8000] = value;
  }
}

void write_memory16(struct CPU *cpu, uint16_t value, uint16_t addr) {
  write_memory8(cpu, (uint8_t)value & 0xFF, addr);
  write_memory8(cpu, (uint8_t)(value >> 8) & 0xFF, ++addr);
}
