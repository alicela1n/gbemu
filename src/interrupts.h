#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "cpu_instr.h"

enum interrupts {
  int_joypad = 0x10,  // Joypad flag (bit-4)
  int_serial = 0x08,  // Serial flag (bit-3)
  int_timer  = 0x04,  // Timer flag (bit-2)
  int_lcd    = 0x02,  // LCD flag (bit-1)
  int_vblank = 0x00   // Vblank flag (bit-0)
};

void cpu_set_ie_flag(struct CPU *cpu, uint8_t interrupt, bool set);
void cpu_set_if_flag(struct CPU *cpu, uint8_t interrupt, bool set);
void handle_interrupts(struct CPU *cpu);

#endif
