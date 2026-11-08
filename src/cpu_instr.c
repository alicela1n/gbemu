#include "cpu.h"
#include "memory.h"
#include "cpu_instr.h"
#include "helper.h"

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
  return (cpu->registers[f] & flag) != 0;
}

/* cpu_set_flag(): Set a flag in the flags register,
 * passing 1 to set sets the flag, passing 0 unsets it
 */
void cpu_set_flag(struct CPU *cpu, uint8_t flag, bool set) {
  if (set)
    cpu->registers[f] |= flag;
  else
    cpu->registers[f] &= ~(flag);
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

/* cpu_bit_r(): Test a bit at the CPU register specified and set the
 * z flag to true if a bit is not set, or false if it is.
 */
void cpu_bit_r(struct CPU *cpu, uint8_t r, uint8_t b) {
  if ((cpu->registers[r] & (1 << b)) == 0)
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);

  cpu_set_flag(cpu, flag_n, false);
  cpu_set_flag(cpu, flag_h, true);
}

/* cpu_bit_mem8(): Test a bit in memory at an address and set the
 * z flag to true if a bit is not set, or false if it is.
 */
void cpu_bit_mem8(struct CPU *cpu, uint16_t addr, uint8_t b) {
  if ((read_memory8(cpu, addr) & (1 << b)) == 0)
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);

  cpu_set_flag(cpu, flag_n, false);
  cpu_set_flag(cpu, flag_h, true);
}

/* cpu_res_r(): Reset a bit at the CPU register specified
 */
void cpu_res_r(struct CPU *cpu, uint8_t r, uint8_t b) {
  cpu->registers[r] &= ~((uint8_t)1 << b);
}

/* cpu_res_mem8(): Reset a bit at the memory address specified
 */
void cpu_res_mem8(struct CPU *cpu, uint16_t addr, uint8_t b) {
  uint8_t val = read_memory8(cpu, addr);
  write_memory8(cpu, val & ~((uint8_t)1 << b), addr);
}

/* cpu_set_mem8(): Set a bit at the memory address specified
 */
void cpu_set_r(struct CPU *cpu, uint8_t r, uint8_t b) {
  cpu->registers[r] |= ((uint8_t)1 << b);
}

/* cpu_set_mem8(): Set a bit at the memory address specified
 */
void cpu_set_mem8(struct CPU *cpu, uint16_t addr, uint8_t b) {
  uint8_t val = read_memory8(cpu, addr);
  write_memory8(cpu, val | ((uint8_t)1 << b), addr);
}

/* cpu_ld_reg8(): Load data (n) into a register (r), 8 bit
 */
void cpu_ld_reg8(struct CPU *cpu, uint8_t r, uint8_t n) {
  cpu->registers[r] = n;
}

/* cpu_ld_reg16(): Load data (n) into 16 bit register or register pair
 */
void cpu_ld_reg16(struct CPU *cpu, uint16_t r, uint16_t n) {
  switch(r) {
    case sp:
      cpu->sp = n;
      break;
    case pc:
      cpu->pc = n;
      break;
    case af:
      cpu_ld_reg8(cpu, f, n & 0xFF);
      cpu_ld_reg8(cpu, a, (n >> 8) & 0xFF);
      break;
    case bc:
      cpu_ld_reg8(cpu, c, n & 0xff);
      cpu_ld_reg8(cpu, b, (n >> 8) & 0xff);
    break;
    case de:
      cpu_ld_reg8(cpu, e, n & 0xff);
      cpu_ld_reg8(cpu, d, (n >> 8) & 0xff);
      break;
    case hl:
      cpu_ld_reg8(cpu, l, n & 0xff);
      cpu_ld_reg8(cpu, h, (n >> 8) & 0xff);
      break;
  }
}

/* cpu_inc_reg8(): Increment 8 bit register 
 */
void cpu_inc_reg8(struct CPU *cpu, uint8_t r, uint8_t n) {
  cpu->registers[r] += n;
}

void cpu_inc_mem8(struct CPU *cpu, uint8_t addr, uint8_t n) {
  uint8_t val = read_memory8(cpu, addr);
  ++val;
  write_memory8(cpu, val, addr);
}

void cpu_dec_mem8(struct CPU *cpu, uint8_t addr, uint8_t n) {
  uint8_t val = read_memory8(cpu, addr);
  --val;
  write_memory8(cpu, val, addr);
}



/* cpu_inc_reg16(): Increment 16 bit register or register pair
 */
void cpu_inc_reg16(struct CPU *cpu, uint16_t r, uint16_t n) {
  uint16_t val = cpu_read_reg16(cpu, r);
  val += n;

  cpu_ld_reg16(cpu, r, val);
}

/* cpu_dec_reg8(): Decrement 8 bit register
 */
void cpu_dec_reg8(struct CPU *cpu, uint8_t r, uint8_t n) {
  cpu->registers[r] -= n;
}

/* cpu_dec_reg16(): Decrement 16 bit register or register pair
 */
void cpu_dec_reg16(struct CPU *cpu, uint16_t r, uint16_t n) {
  uint16_t val = cpu_read_reg16(cpu, r);

  val -= n;
  cpu_ld_reg16(cpu, r, val);
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
  uint16_t r = (uint16_t)*a + b;

  cpu_set_flag(cpu, flag_h, (*a & 0x0F) + (b & 0x0F) > 0x0F);

  cpu_set_flag(cpu, flag_c, (r > 0xFF));
  cpu_set_flag(cpu, flag_z, (*a == 0));

  *a = (uint8_t)r & 0xFF;
}

/* cpu_adc8(): Same as cpu_add() but adds the carry
 */
void cpu_adc8(struct CPU *cpu, uint8_t *a, uint8_t *b) {
  cpu_add8(cpu, a, *b);
  *a += cpu_check_flag(cpu, flag_c);
}

/* cpu_add_reg16(): 16-bit adding function for two register pairs 
 */
void cpu_add_reg16(struct CPU *cpu, uint16_t rp1, uint16_t rp2) {
  uint16_t r = cpu_read_reg16(cpu, rp1) + cpu_read_reg16(cpu, rp2);

  cpu_set_flag(cpu, flag_h, (rp1 & 0x0FFF) + (rp2 & 0x0FFF) > 0x0FFF);
  cpu_set_flag(cpu, flag_c, (rp1 & 0xFFFF) + (rp2 & 0xFFFF) > 0xFFFF);
  cpu_set_flag(cpu, flag_z, (r == 0));


  cpu_ld_reg16(cpu, rp1, r);
}

/* cpu_read_reg16(): Read 16 bit register or register pair
 */
uint16_t cpu_read_reg16(struct CPU *cpu, uint16_t r) {
  switch(r) {
    case af:
      return uint8to16(cpu->registers[f], cpu->registers[a]);
      break;
    case bc:
      return uint8to16(cpu->registers[c], cpu->registers[b]);
      break;
    case de:
      return uint8to16(cpu->registers[e], cpu->registers[d]);
      break;
    case hl:
      return uint8to16(cpu->registers[l], cpu->registers[h]);
      break;
    case sp:
      return cpu->sp;
      break;
    case pc:
      return cpu->pc;
      break;
    }

    return 0;
}


/* cpu_sub8(): 8-bit subtraction function which deals with carries and stuff,
 *             takes in two variables a and b, and stores the result in a.
 */
void cpu_sub8(struct CPU *cpu, uint8_t *a, uint8_t b) {
  uint8_t r = *a;
  r -= b;

  cpu_set_flag(cpu, flag_h, ((*a & 0x0F) < (b & 0x0F)));
  cpu_set_flag(cpu, flag_c, (*a < b));

  *a = r;

  cpu_set_flag(cpu, flag_z, *a == 0);
  cpu_set_flag(cpu, flag_n, true);
}

/* cpu_sbc8(): Same as cpu_sub8() but adds the carry to the value being
 * subtracted
 */
void cpu_sbc8(struct CPU *cpu, uint8_t *a, uint8_t *b) {
  cpu_sub8(cpu, a, *b);
  *a -= cpu_check_flag(cpu, flag_c);
}

/* cpu_cp8(): Same as cpu_sub8() except it doesn't modify the accumulator
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

  cpu->registers[r] = (cpu->registers[r] << 1) | ((cpu->registers[r] & 0x80) << 7);

  if (cpu->registers[r] & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);

  cpu_set_flag(cpu, flag_h, false);
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

  cpu_set_flag(cpu, flag_h, false);
}

/* cpu_sra_r(): Shift register right arithmetic
 */
void cpu_sra_r(struct CPU *cpu, uint8_t r) {
  if (cpu->registers[r] & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
   cpu_set_flag(cpu, flag_c, false);

  cpu->registers[r] = (cpu->registers[r] >> 1) | ((cpu->registers[r] & 0x80) >> 7);

  if (cpu->registers[r] & (1 << 0))
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);

  cpu_set_flag(cpu, flag_h, false);
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

  cpu_set_flag(cpu, flag_h, false);
}

/* cpu_srl_r(): Shift register right logical
 */
void cpu_srl_r(struct CPU *cpu, uint8_t r) {
  if (cpu->registers[r] & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
   cpu_set_flag(cpu, flag_c, false);

  cpu->registers[r] = cpu->registers[r] >> 1;

  if (cpu->registers[r] == 0)
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);

  cpu_set_flag(cpu, flag_h, false);
}

/* cpu_srl_mem8(): Shift register right logical
 */
void cpu_srl_mem8(struct CPU *cpu, uint16_t addr) {
  if (read_memory8(cpu, addr) & (1 << 7))
    cpu_set_flag(cpu, flag_c, true);
  else
   cpu_set_flag(cpu, flag_c, false);

  write_memory8(cpu, addr, (read_memory8(cpu, addr) >> 1));

  if (read_memory8(cpu, addr) == 0)
    cpu_set_flag(cpu, flag_z, true);
  else
    cpu_set_flag(cpu, flag_z, false);

  cpu_set_flag(cpu, flag_h, false);
}

/* cpu_sll_r(): Shift register left arithmetic
 */
void cpu_sll_r(struct CPU *cpu, uint8_t r) {
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

