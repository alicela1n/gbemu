#include "cpu.h"
#include "memory.h"
#include "cpu_instr.h"

/* cpu_toggle_halt(): Toggle halt flag on CPU
 */
void cpu_toggle_halt(struct CPU *cpu) {
  if (!cpu->halt)
    cpu->halt = true;
  else
    cpu->halt = false;
}

/* cpu_check_flag(): Check if a flag in the flags register is set
 */
bool cpu_check_flag(struct CPU *cpu, uint8_t flag) {
  if (cpu->registers[f] & (1 << flag)) {
    return true;
  } else {
    return false;
  }
}

/* cpu_set_flag(): Set a flag in the flags register,
 * passing 1 to set sets the flag, passing 0 unsets it
 */
void cpu_set_flag(struct CPU *cpu, uint8_t flag, bool set) {
  if (set)
    cpu->registers[f] = cpu->registers[f] | (1 << flag);
  else
    cpu->registers[f] = cpu->registers[f] & ~(1 << flag);
}

/* cpu_toggle_flag(): Toggle a flag in the flag register
 * If set unset it, if not set set it
 */
void cpu_toggle_flag(struct CPU *cpu, uint8_t flag) {
  if (cpu_check_flag(cpu, flag))
    // Unset
    cpu_set_flag(cpu, flag, 0);
  else
    // Set
    cpu_set_flag(cpu, flag, 1);
}

/* cpu_ld_reg8(): Load data (n) into a register (r), 8 bit
 */
void cpu_ld_reg8(struct CPU *cpu, uint8_t r, uint8_t n) {
  cpu->registers[r] = n;
}

void cpu_ld_reg16(struct CPU *cpu, uint16_t r, uint16_t n) {
  if (r == sp) {
    cpu->sp = n;
  } else if (r == pc) {
    cpu->pc = n;
  } else if (r == af) {
    cpu_ld_reg8(cpu, a, n & 0xFF);
    cpu_ld_reg8(cpu, f, (n >> 8) & 0xFF);
  } else if (r == bc) {
    cpu_ld_reg8(cpu, b, n & 0xff);
    cpu_ld_reg8(cpu, c, (n >> 8) & 0xff);
  } else if (r == de) {
    cpu_ld_reg8(cpu, d, n & 0xff);
    cpu_ld_reg8(cpu, e, (n >> 8) & 0xff);
  } else if (r == hl) {
    cpu_ld_reg8(cpu, h, n & 0xff);
    cpu_ld_reg8(cpu, l, (n >> 8) & 0xff);
  }
}

/* cpu_ld_mem8(): Load data (a) into memory at addr
 */
void cpu_ld_mem8(struct CPU *cpu, uint16_t addr, uint8_t a) {
  write_memory8(cpu, a, addr);
}

/* cpu_ld_mem16(): Same thing as cpu_ld_mem8() but deals with 16 bit integers
 */
void cpu_ld_mem16(struct CPU *cpu, uint16_t addr, uint16_t b) {
  cpu_ld_mem8(cpu, addr, (b | 0xFF));
  cpu_ld_mem8(cpu, addr + 1, b);
}

/* cpu_jp(): Jump to a location in memory
 */
void cpu_jp(struct CPU *cpu, uint16_t addr) { cpu->pc = addr; }

/* cpu_jr(): Jump to a relative location
 */
void cpu_jr(struct CPU *cpu, uint8_t addr) { cpu->pc += addr; }

/* cpu_add8(): 8-bit adding function which deals with carries and stuff,
 *             takes in two variables a and b, and stores the result in a.
 */
void cpu_add8(struct CPU *cpu, uint8_t *a, uint8_t b) {
  /*printf("%i\n", *a);
  printf("%i\n", *b); */
  *a = *a + b;

  if ((*a & 0x0F) + (b & 0x0F) == 0x10)
    cpu_set_flag(cpu, flag_h, 1);
  else
    cpu_set_flag(cpu, flag_h, 0);

  if (*a > 0xFF)
    cpu_set_flag(cpu, flag_c, 1);
  else
    cpu_set_flag(cpu, flag_c, 0);

  if (*a == 0)
    cpu_set_flag(cpu, flag_z, 1);
  else
    cpu_set_flag(cpu, flag_z, 0);
}

/* cpu_adc8(): Same as cpu_add() but adds the carry
 */
void cpu_adc8(struct CPU *cpu, uint8_t *a, uint8_t *b) {
  if (cpu_check_flag(cpu, flag_c))
    (*b)++;

  cpu_add8(cpu, a, *b);
}

/* cpu_sub8(): 8-bit subtraction function which deals with carries and stuff,
 *             takes in two variables a and b, and stores the result in a.
 */
void cpu_sub8(struct cpu *cpu, uint8_t *a, uint8_t b) {
  *a = b - *a;

  if ((*a & 0x0F) + (b & 0x0F) == 0x10)
    cpu_set_flag(cpu, flag_h, 1);
  else
    cpu_set_flag(cpu, flag_h, 0);

  if (*a > 0xFF)
    cpu_set_flag(cpu, flag_c, 1);
  else
    cpu_set_flag(cpu, flag_c, 0);

  if (*a == 0)
    cpu_set_flag(cpu, flag_z, 1);
  else
    cpu_set_flag(cpu, flag_z, 0);
}

/* cpu_sbc8(): Same as cpu_sub8() but adds the carry to the value being
 * subtracted
 */
void cpu_sbc8(struct CPU *cpu, uint8_t *a, uint8_t *b) {
  if (cpu_check_flag(cpu, flag_c))
    (*b)++;

  cpu_sub8(cpu, a, *b);
}

/* cpu_cp8(): Same as cpu_sub8(): except it doesn't modify the accumulator
 */
void cpu_cp8(struct CPU *cpu, uint8_t *a, uint8_t *b) {
  uint8_t tmp = *a; // Temporary variable to use instead of accumulator
  cpu_sub8(cpu, &tmp, *b);
}

/* cpu_and(): AND operation
 */
void cpu_and(struct CPU *cpu, uint8_t *a, uint8_t *b) {
  *a = *a & *b;
  if (*a == 0)
    cpu_set_flag(cpu, flag_z, 1);
  else
    cpu_set_flag(cpu, flag_z, 0);

  // Set half carry flag?
  cpu_set_flag(cpu, flag_h, 1);
}

/* cpu_xor(): XOR operation
 */
void cpu_xor(struct CPU *cpu, uint8_t *a, uint8_t *b) {
  *a = *a ^ *b;
  if (*a == 0)
    cpu_set_flag(cpu, flag_z, 1);
  else
    cpu_set_flag(cpu, flag_z, 0);
}

/* cpu_or(): OR operation
 */
void cpu_or(struct CPU *cpu, uint8_t *a, uint8_t *b) {
  *a = *a | *b;
  if (*a == 0)
    cpu_set_flag(cpu, flag_z, 1);
  else
    cpu_set_flag(cpu, flag_z, 0);
}

/* cpu_pop(): Pop from stack to register a
 */
void cpu_pop(struct CPU *cpu, uint8_t *a, uint8_t *b) {
  *a = read_memory8(cpu, cpu->sp + 1);
  *b = read_memory8(cpu, cpu->sp);
  cpu->sp += 2;
}

/* cpu_push(): Push from register a onto stack
 */
void cpu_push(struct CPU *cpu, uint8_t *a, uint8_t *b) {
  write_memory8(cpu, *a, cpu->sp - 1);
  write_memory8(cpu, *b, cpu->sp);
  cpu->sp -= 2;
}

/* cpu_call(): Calls from an address
 */
void cpu_call(struct CPU *cpu, uint16_t addr) {
  uint8_t high = (cpu->pc >> 8) & 0xFF;
  uint8_t low = cpu->pc & 0xFF;
  cpu_push(cpu, &high, &low);
  cpu->pc = addr;
}

/* cpu_rst(): Restart / unconditionally call function
 */
void cpu_rst(struct CPU *cpu, uint16_t n) {
  cpu->sp--;
  write_memory8(cpu, (cpu->pc >> 8) & 0xFF, cpu->sp);
  cpu->sp--;
  write_memory8(cpu, (cpu->pc & 0xFF), cpu->sp);
  cpu->pc = n & 0xFF;
}

/* cpu_ret(): Return
 */
void cpu_ret(struct CPU *cpu) { cpu->pc = read_memory8(cpu, cpu->sp++); }

/* cpu_rl_r(): Rotate register left
 */
void cpu_rl_r(struct CPU *cpu, uint8_t r) {
  cpu->registers[r] = circshift(cpu->registers[r], 1);
  
  if (cpu->registers[r] & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
    cpu_set_flag(cpu, flag_c, false);

  if (cpu->registers[r] & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);
 
}

/* cpu_rl_mem8(): Rotate value at memory address left
 */ 
void cpu_rl_mem8(struct CPU *cpu, uint16_t addr) {
  write_memory8(cpu, circshift(read_memory8(cpu, addr), 1), addr);

  if (read_memory8(cpu, addr) & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
   cpu_set_flag(cpu, flag_c, false);

  if (read_memory8(cpu, addr) & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);

}

/* cpu_rl_r(): Rotate register right
 */
void cpu_rr_r(struct CPU *cpu, uint8_t r) {
  cpu->registers[r] = circshift(cpu->registers[r], 1);
  
  if (cpu->registers[r] & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
    cpu_set_flag(cpu, flag_c, false);

  if (cpu->registers[r] & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);
 
}

/* cpu_rr_mem8(): Rotate value at memory address left
 */ 
void cpu_rr_mem8(struct CPU *cpu, uint16_t addr) {
  write_memory8(cpu, circshift(read_memory8(cpu, addr), -1), addr);

  if (read_memory8(cpu, addr) & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
   cpu_set_flag(cpu, flag_c, false);

  if (read_memory8(cpu, addr) & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);

}

/* cpu_rlc_r(): Rotate register left circular
 */
void cpu_rlc_r(struct CPU *cpu, uint8_t r) {
  if (cpu->registers[r] & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
    cpu_set_flag(cpu, flag_c, false);

  cpu->registers[r] = circshift(cpu->registers[r], 1);

  if (cpu->registers[r] & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);
}

/* cpu_rlc_mem8(): Rotate value at memory address left circular
 */ 
void cpu_rlc_mem8(struct CPU *cpu, uint16_t addr) {
  if (read_memory8(cpu, addr) & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
   cpu_set_flag(cpu, flag_c, false);

  write_memory8(cpu, circshift(read_memory8(cpu, addr), 1), addr);

  if (read_memory8(cpu, addr) & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);

}

/* cpu_rrc_r(): Rotate register right circular
 */
void cpu_rrc_r(struct CPU *cpu, uint8_t r) {
  if (cpu->registers[r] & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
   cpu_set_flag(cpu, flag_c, false);

  cpu->registers[r] = circshift(cpu->registers[r], -1);

  if (cpu->registers[r] & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);
}

/* cpu_rrc_mem8(): Rotate value at memory address left circular
 */ 
void cpu_rrc_mem8(struct CPU *cpu, uint16_t addr) {
  if (read_memory8(cpu, addr) & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
   cpu_set_flag(cpu, flag_c, false);

  write_memory8(cpu, circshift(read_memory8(cpu, addr), -1), addr);

  if (read_memory8(cpu, addr) & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);
}

/* cpu_sl_r(): Shift register left
 */
void cpu_sl_r(struct CPU *cpu, uint8_t r) {
  cpu->registers[r] = cpu->registers[r] << 1;
  
  if (cpu->registers[r] & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
    cpu_set_flag(cpu, flag_c, false);

  if (cpu->registers[r] & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);
 
}

/* cpu_sl_mem8(): Shift value at memory address left
 */ 
void cpu_sl_mem8(struct CPU *cpu, uint16_t addr) {
  write_memory8(cpu, (read_memory8(cpu, addr) << 1), addr);

  if (read_memory8(cpu, addr) & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
   cpu_set_flag(cpu, flag_c, false);

  if (read_memory8(cpu, addr) & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);

}

/* cpu_sr_r(): Shift register right
 */
void cpu_sr_r(struct CPU *cpu, uint8_t r) {
  cpu->registers[r] = cpu->registers[r] >> 1;
  
  if (cpu->registers[r] & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
    cpu_set_flag(cpu, flag_c, false);

  if (cpu->registers[r] & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);
 
}

/* cpu_sr_mem8(): Shift value at memory address left
 */ 
void cpu_sr_mem8(struct CPU *cpu, uint16_t addr) {
  write_memory8(cpu, (read_memory8(cpu, addr) >> 1), addr);

  if (read_memory8(cpu, addr) & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
   cpu_set_flag(cpu, flag_c, false);

  if (read_memory8(cpu, addr) & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);

}

/* cpu_sla_r(): Shift register left arithmetic
 */
void cpu_sla_r(struct CPU *cpu, uint8_t r) {
  if (cpu->registers[r] & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
    cpu_set_flag(cpu, flag_c, false);

  cpu->registers[r] = cpu->registers[r] << 1;

  if (cpu->registers[r] & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);
}

/* cpu_sla_mem8(): Shift value at memory address left arithmetic
 */ 
void cpu_sla_mem8(struct CPU *cpu, uint16_t addr) {
  if (read_memory8(cpu, addr) & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
   cpu_set_flag(cpu, flag_c, false);

  write_memory8(cpu, (read_memory8(cpu, addr) << 1), addr);

  if (read_memory8(cpu, addr) & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);

}

/* cpu_sra_r(): Shift register right arithmetic
 */
void cpu_sra_r(struct CPU *cpu, uint8_t r) {
  if (cpu->registers[r] & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
   cpu_set_flag(cpu, flag_c, false);

  cpu->registers[r] = cpu->registers[r] >> 1;

  if (cpu->registers[r] & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);
}

void cpu_swap_r(struct CPU *cpu, uint8_t r) {
  cpu->registers[r] = (cpu->registers[r] & 0x0F) << 4 | (cpu->registers[r] & 0xF0) >> 4;
}

void cpu_swap_m(struct CPU *cpu, uint8_t addr) {
  uint8_t val = read_memory8(cpu, addr);
  write_memory8(cpu, ((val & 0x0F) << 4 | (val & 0xF0) >> 4), addr);
}

/* cpu_sra_mem8(): Shift value at memory address right arithmetic
 */ 
void cpu_sra_mem8(struct CPU *cpu, uint16_t addr) {
  if (read_memory8(cpu, addr) & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
   cpu_set_flag(cpu, flag_c, false);

  write_memory8(cpu, (read_memory8(cpu, addr) >> 1), addr);

  if (read_memory8(cpu, addr) & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);
}


