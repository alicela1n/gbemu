#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* The GameBoy has 8 8-bit registers
 * A -> Accumulator
 * B -> General Purpose
 * C -> General Purpose
 * D -> General Purpose
 * E -> General Purpose
 * F -> Flags
 * H -> General Purpose
 * L -> General Purpose
 */
enum registers { a = 0, b = 1, c = 2, d = 3, e = 4, f = 5, h = 6, l = 7 };

/* There are four flags, z, n, h, and c */
enum flags {
  flag_z = 0x80, // Zero bit (bit-7)
  flag_n = 0x40, // Subtraction flag (bit-6)
  flag_h = 0x20, // Half carry flag (bit-5)
  flag_c = 0x10  // Carry flag (bit-4)
};

struct CPU {
  uint8_t registers[8];

  /* Two 16-bit registers
   * SP -> Stack Pointer
   * PC -> Program Counter
   */
  uint16_t sp;
  uint16_t pc;

  /* Interrupt registers
   * IE -> Interrupt enable selection
   * IF -> Interrupt status and acknowledge
   * IME -> Interurpt master enable
   */
  uint16_t ie_r;
  uint16_t if_r;
  uint32_t ime_r;

  uint8_t mem[0xFFFF];

  uint32_t cycles;

  /* Keep track of CPU state */
  bool halt;
  bool ime; /* Interrupt master enable flag */
  bool ime_pending;
};

void cpu_init(struct CPU *cpu);
void cpu_toggle_halt(struct CPU *cpu);
void cpu_execute(struct CPU *cpu, bool dump, bool print_regs);

#endif
