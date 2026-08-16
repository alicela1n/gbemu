#ifndef MEMORY_H
#define MEMORY_H

#include "cpu.h"
#include "rom.h"
#include <stdint.h>
#include <stdio.h>

uint8_t read_memory8(struct CPU *cpu, uint16_t addr);
void write_memory8(struct CPU *cpu, uint8_t value, uint16_t addr);
uint16_t read_memory16(struct CPU *cpu, uint16_t addr);
uint8_t *mem_ptr8(struct CPU *cpu, uint16_t addr);

#endif
