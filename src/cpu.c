#include "cpu.h"
#include "helper.h"
#include "memory.h"
#include <_inttypes.h>
#include <math.h>

/* cpu_init(): Initialize CPU
 */
void cpu_init(struct CPU *cpu) {
  // Initialize CPU registers
  cpu->registers[a] = 0x01;
  cpu->registers[b] = 0xff;
  cpu->registers[c] = 0x13;
  cpu->registers[d] = 0x00;
  cpu->registers[e] = 0xC1;
  cpu->registers[f] = 0x00;
  cpu->registers[d] = 0x00;
  cpu->registers[h] = 0x84;
  cpu->registers[l] = 0x03;

  // On the GameBoy the program starts executing at 0x0100
  cpu->pc = 0x0100;
  // Stack pointer grows downwards
  cpu->sp = 0xFFFE;

  // Initialize the memory by setting it to all 0s
  memset(cpu->mem, 0, 0xFFFF);

  // CPU is running so halt = false
  cpu->halt = false;
}

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

void cpu_jp(struct CPU *cpu, uint16_t addr) {
  cpu->pc = read_memory16(cpu, addr);
}

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
void cpu_sub8(struct CPU *cpu, uint8_t *a, uint8_t b) {
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

/* cpu_execute(): What this does is decode each opcode and translate it into
 * an instruction to be cpu_execute by the virtual CPU. Optional dump flag
 * for disassembly upon execution.
 */
void cpu_execute(struct CPU *cpu, bool dump, bool print_regs) {
  while (!cpu->halt) {
    if (print_regs) {
      printf("cpu_execute(): --Begin--\n");
      printf("cpu_execute(): read_memory8(cpu, cpu->pc): 0x%02X\n",
             read_memory8(cpu, cpu->pc));
      printf("cpu_execute(): 8 bit registers a-l\n");
      printf("cpu_execute(): a: 0x%02X\n", cpu->registers[a]);
      printf("cpu_execute(): b: 0x%02X\n", cpu->registers[b]);
      printf("cpu_execute(): c: 0x%02X\n", cpu->registers[c]);
      printf("cpu_execute(): d: 0x%02X\n", cpu->registers[d]);
      printf("cpu_execute(): e: 0x%02X\n", cpu->registers[e]);
      printf("cpu_execute(): f: 0x%02X\n", cpu->registers[f]);
      printf("cpu_execute(): h: 0x%02X\n", cpu->registers[h]);
      printf("cpu_execute(): l: 0x%02X\n", cpu->registers[l]);
      printf("\n");
      printf("cpu_execute(): 16 bit registers sp and pc\n");
      printf("cpu_execute(): sp: 0x%02X\n", cpu->sp);
      printf("cpu_execute(): pc: 0x%02X\n", cpu->pc);
      printf("cpu_execute(): Flags\n\n");
      printf("cpu_execute(): z: %02X, n: %02X, h: %02X, c: %02X\n",
             cpu_check_flag(cpu, flag_z), cpu_check_flag(cpu, flag_n),
             cpu_check_flag(cpu, flag_h), cpu_check_flag(cpu, flag_c));
    }

    // NOP
    if (read_memory8(cpu, cpu->pc) == 0x00) {
      if (dump)
        printf("NOP\n");
      cpu->pc++;
    }

    // LD BC, d16
    else if (read_memory8(cpu, cpu->pc) == 0x01) {
      if (dump)
        printf("LD BC, %02X%02X\n", read_memory8(cpu, cpu->pc + 1),
               read_memory8(cpu, cpu->pc + 2));

      // C and B are reversed due to endianness
      cpu_ld_reg8(cpu, c, read_memory8(cpu, (cpu->pc + 1)));
      cpu_ld_reg8(cpu, b, read_memory8(cpu, (cpu->pc + 2)));

      cpu->pc += 3;
    }

    // LD (BC), A
    else if (read_memory8(cpu, cpu->pc) == 0x02) {
      if (dump)
        printf("LD (BC), A\n");

      cpu_ld_mem8(cpu, uint8to16(cpu->registers[b], cpu->registers[c]),
                  cpu->registers[a]);

      cpu->pc++;
    }

    // INC BC
    else if (read_memory8(cpu, cpu->pc) == 0x03) {
      if (dump)
        printf("INC BC\n");

      cpu->registers[b]++;
      cpu->registers[c]++;

      cpu->pc++;
    }

    // INC B
    else if (read_memory8(cpu, cpu->pc) == 0x04) {
      if (dump)
        printf("INC B\n");

      cpu->registers[b]++;
      cpu->pc++;
    }

    // DEC B
    else if (read_memory8(cpu, cpu->pc) == 0x05) {
      if (dump)
        printf("DEC B\n");

      cpu->registers[b]--;
      cpu->pc++;
    }

    // LD B, d8
    else if (read_memory8(cpu, cpu->pc) == 0x06) {
      if (dump)
        printf("LD B, %02X\n", read_memory8(cpu, cpu->pc + 1));

      cpu_ld_reg8(cpu, b, read_memory8(cpu, (cpu->pc + 1)));

      cpu->pc += 2;
    }

    // RLCA
    else if (read_memory8(cpu, cpu->pc) == 0x07) {
      if (dump)
        printf("RLCA\n");

      cpu->registers[a] =
          (cpu->registers[a] << 1) | ((cpu->registers[a] >> 7) & 1);
      cpu->registers[c] = cpu->registers[a] & (1 << 0);

      cpu->pc++;
    }

    // LD (a16), SP
    else if (read_memory8(cpu, cpu->pc) == 0x08) {
      if (dump)
        printf("LD %02X%02X\n, SP\n", read_memory8(cpu, cpu->pc + 1),
               read_memory8(cpu, cpu->pc + 2));

      cpu_ld_mem16(cpu, cpu->pc + 1, cpu->sp);

      cpu->pc += 3;
    }

    // ADD HL, BC
    else if (read_memory8(cpu, cpu->pc) == 0x09) {
      if (dump)
        printf("ADD HL, BC\n");

      cpu_add8(cpu, &cpu->registers[h], cpu->registers[b]);
      cpu_add8(cpu, &cpu->registers[l], cpu->registers[c]);

      cpu->pc++;
    }

    // LD A, (BC)
    else if (read_memory8(cpu, cpu->pc) == 0x0A) {
      if (dump)
        printf("LD A, (BC)\n");

      cpu_ld_reg8(
          cpu, a,
          read_memory8(cpu, uint8to16(cpu->registers[b], cpu->registers[c])));

      cpu->pc++;
    }

    // DEC BC
    else if (read_memory8(cpu, cpu->pc) == 0x0B) {
      if (dump)
        printf("DEC BC\n");

      cpu->registers[b]--;
      cpu->registers[c]--;

      cpu->pc++;
    }

    // INC C
    else if (read_memory8(cpu, cpu->pc) == 0x0C) {
      if (dump)
        printf("INC C\n");

      cpu->registers[c]++;

      cpu->pc++;
    }

    // DEC C
    else if (read_memory8(cpu, cpu->pc) == 0x0D) {
      if (dump)
        printf("DEC C\n");

      cpu->registers[c]--;

      cpu->pc++;
    }

    // LD C, d8
    else if (read_memory8(cpu, cpu->pc) == 0x0E) {
      if (dump)
        printf("LD C, %02X\n", read_memory8(cpu, cpu->pc + 1));

      cpu_ld_reg8(cpu, c, read_memory8(cpu, cpu->pc + 1));

      cpu->pc += 2;
    }

    // RRCA
    else if (read_memory8(cpu, cpu->pc) == 0x0F) {
      if (dump)
        printf("RRCA\n");

      cpu->registers[a] =
          (cpu->registers[a] >> 1) | ((cpu->registers[a] << 7) & 1);
      cpu->registers[c] = cpu->registers[a] & (1 >> 0);

      cpu->pc++;
    }

    // STOP
    else if (read_memory8(cpu, cpu->pc) == 0x10) {
      if (dump)
        printf("STOP\n");

      cpu->halt = true;
      cpu->pc++;
    }

    // LD DE,u16
    else if (read_memory8(cpu, cpu->pc) == 0x11) {
      if (dump)
        printf("LD DE, %02X%02X\n", read_memory8(cpu, cpu->pc + 1),
               read_memory8(cpu, cpu->pc + 2));

      cpu_ld_reg8(cpu, e, read_memory8(cpu, cpu->pc + 1));
      cpu_ld_reg8(cpu, d, read_memory8(cpu, cpu->pc + 2));
      cpu->pc += 3;
    }

    // LD (DE), A
    else if (read_memory8(cpu, cpu->pc) == 0x12) {
      if (dump)
        printf("LD (DE), A\n");

      cpu_ld_mem8(cpu, uint8to16(cpu->registers[d], cpu->registers[e]),
                  cpu->registers[a]);

      cpu->pc++;
    }

    // INC DE
    else if (read_memory8(cpu, cpu->pc) == 0x13) {
      if (dump)
        printf("INC DE\n");

      cpu->registers[d]++;
      cpu->registers[e]++;

      cpu->pc++;
    }

    // INC D
    else if (read_memory8(cpu, cpu->pc) == 0x14) {
      if (dump)
        printf("INC D\n");

      cpu->registers[d]++;
      cpu->pc++;
    }

    // DEC D
    else if (read_memory8(cpu, cpu->pc) == 0x15) {
      if (dump)
        printf("DEC D\n");

      cpu->registers[d]--;
      cpu->pc++;
    }

    // LD D, d8
    else if (read_memory8(cpu, cpu->pc) == 0x16) {
      if (dump)
        printf("LD D, %02X\n", read_memory8(cpu, cpu->pc + 1));

      cpu_ld_reg8(cpu, d, read_memory8(cpu, (cpu->pc + 1)));

      cpu->pc += 2;
    }

    // RLA
    else if (read_memory8(cpu, cpu->pc) == 0x17) {
      if (dump)
        printf("RLA\n");

      cpu->registers[a] =
          (cpu->registers[a] << 1) | ((cpu->registers[a] >> 7) & 1);
      cpu->registers[c] = cpu->registers[a] & (1 << 0);

      cpu->pc++;
    }

    // JR i8
    else if (read_memory8(cpu, cpu->pc) == 0x18) {
      if (dump)
        printf("JR %02X\n", read_memory8(cpu, cpu->pc + 1));

      cpu->pc += 2 + read_memory8(cpu, cpu->pc + 1);
    }

    // ADD HL, DE
    else if (read_memory8(cpu, cpu->pc) == 0x19) {
      if (dump)
        printf("ADD HL, DE\n");

      cpu_add8(cpu, &cpu->registers[h], cpu->registers[d]);
      cpu_add8(cpu, &cpu->registers[l], cpu->registers[e]);

      cpu->pc++;
    }

    // LD A, (DE)
    else if (read_memory8(cpu, cpu->pc) == 0x1A) {
      if (dump)
        printf("LD A, (DE)\n");

      cpu_ld_reg8(
          cpu, a,
          read_memory8(cpu, uint8to16(cpu->registers[d], cpu->registers[e])));

      cpu->pc++;
    }

    // DEC DE
    else if (read_memory8(cpu, cpu->pc) == 0x1B) {
      if (dump)
        printf("DEC DE\n");

      cpu->registers[d]--;
      cpu->registers[e]--;

      cpu->pc++;
    }

    // INC E
    else if (read_memory8(cpu, cpu->pc) == 0x1C) {
      if (dump)
        printf("INC E\n");

      cpu->registers[e]++;

      cpu->pc++;
    }

    // DEC E
    else if (read_memory8(cpu, cpu->pc) == 0x1D) {
      if (dump)
        printf("DEC E\n");

      cpu->registers[e]--;

      cpu->pc++;
    }

    // LD E, d8
    else if (read_memory8(cpu, cpu->pc) == 0x1E) {
      if (dump)
        printf("LD E, %02X\n", read_memory8(cpu, cpu->pc + 1));

      cpu_ld_reg8(cpu, e, read_memory8(cpu, (cpu->pc + 1)));

      cpu->pc += 2;
    }

    // RRA
    else if (read_memory8(cpu, cpu->pc) == 0x1F) {
      if (dump)
        printf("RRA\n");

      cpu->registers[a] =
          (cpu->registers[a] >> 1) | ((cpu->registers[a] << 7) & 1);
      cpu->registers[c] = cpu->registers[a] & (1 >> 0);

      cpu->pc++;
    }

    // JP NZ, i8
    else if (read_memory8(cpu, cpu->pc) == 0x20) {
      if (dump)
        printf("JP NZ, %02X\n", read_memory8(cpu, cpu->pc + 1));

      if (!cpu_check_flag(cpu, flag_z))
        cpu->pc += 2 + read_memory8(cpu, cpu->pc + 1);
      else
        cpu->pc += 2;
    }

    // LD HL, d16
    else if (read_memory8(cpu, cpu->pc) == 0x21) {
      if (dump)
        printf("LD HL, %02X%02X\n", read_memory8(cpu, cpu->pc + 1),
               read_memory8(cpu, cpu->pc + 2));

      cpu_ld_reg8(cpu, l, read_memory8(cpu, (cpu->pc + 1)));
      cpu_ld_reg8(cpu, h, read_memory8(cpu, (cpu->pc + 2)));

      cpu->pc += 3;
    }

    // LD (HL+), A
    else if (read_memory8(cpu, cpu->pc) == 0x22) {
      if (dump)
        printf("LD (HL+), A\n");

      cpu_ld_mem8(cpu, uint8to16(cpu->registers[h], cpu->registers[l] + 1),
                  cpu->registers[a]);

      cpu->registers[h]++;
      cpu->registers[l]++;
      cpu->pc++;
    }

    // INC HL
    else if (read_memory8(cpu, cpu->pc) == 0x23) {
      if (dump)
        printf("INC HL\n");

      cpu->registers[h]++;
      cpu->registers[l]++;

      cpu->pc++;
    }

    // INC H
    else if (read_memory8(cpu, cpu->pc) == 0x24) {
      if (dump)
        printf("INC H\n");

      cpu->registers[h]++;
      cpu->pc++;
    }

    // DEC H
    else if (read_memory8(cpu, cpu->pc) == 0x25) {
      if (dump)
        printf("DEC H\n");

      cpu->registers[h]--;
      cpu->pc++;
    }

    // LD H, d8
    else if (read_memory8(cpu, cpu->pc) == 0x26) {
      if (dump)
        printf("LD H, %02X\n", read_memory8(cpu, cpu->pc + 1));

      cpu_ld_reg8(cpu, h, read_memory8(cpu, (cpu->pc + 1)));

      cpu->pc += 2;
    }

    // DAA
    else if (read_memory8(cpu, cpu->pc) == 0x27) {
      if (dump)
        printf("DAA\n");

      if (cpu->registers[a] > 0x09 || cpu_check_flag(cpu, flag_z)) {
        cpu->registers[a] += 0x06;
      } else if (cpu->registers[a] > 0x99 || cpu_check_flag(cpu, flag_c)) {
        cpu->registers[a] += 0x60;
      }

      cpu->pc++;
    }

    // JR Z, i8
    else if (read_memory8(cpu, cpu->pc) == 0x28) {
      if (dump)
        printf("JP NZ, %02X\n", read_memory8(cpu, cpu->pc + 1));

      if (cpu_check_flag(cpu, flag_z))
        cpu->pc += 2 + read_memory8(cpu, cpu->pc + 1);
      else
        cpu->pc += 2;
    }

    // ADD HL, HL
    else if (read_memory8(cpu, cpu->pc) == 0x29) {
      if (dump)
        printf("ADD HL, DE\n");

      cpu_add8(cpu, &cpu->registers[h], cpu->registers[h]);
      cpu_add8(cpu, &cpu->registers[l], cpu->registers[l]);

      cpu->pc++;
    }

    // LD A, (HL+)
    else if (read_memory8(cpu, cpu->pc) == 0x2A) {
      if (dump)
        printf("LOAD A, (HL+)\n");

      cpu_ld_reg8(cpu, a,
                  read_memory8(cpu, uint8to16(cpu->registers[l],
                                              cpu->registers[h] + 1)));

      cpu->pc++;
    }

    // DEC HL
    else if (read_memory8(cpu, cpu->pc) == 0x2B) {
      if (dump)
        printf("DEC DE\n");

      cpu->registers[h]--;
      cpu->registers[l]--;

      cpu->pc++;
    }

    // INC L
    else if (read_memory8(cpu, cpu->pc) == 0x2C) {
      if (dump)
        printf("INC L\n");

      cpu->registers[l]++;

      cpu->pc++;
    }

    // DEC L
    else if (read_memory8(cpu, cpu->pc) == 0x2D) {
      if (dump)
        printf("DEC L\n");

      cpu->registers[l]--;

      cpu->pc++;
    }

    // LD L, d8
    else if (read_memory8(cpu, cpu->pc) == 0x2E) {
      if (dump)
        printf("LD L, %02X\n", read_memory8(cpu, cpu->pc + 1));

      cpu_ld_reg8(cpu, l, read_memory8(cpu, (cpu->pc + 1)));

      cpu->pc += 2;
    }

    // CPL
    else if (read_memory8(cpu, cpu->pc) == 0x2F) {
      if (dump)
        printf("CPL\n");

      cpu->registers[a] = ~(cpu->registers[a] & 0xFF);
      cpu->pc++;
    }

    // JR NC, i8
    else if (read_memory8(cpu, cpu->pc) == 0x30) {
      if (dump)
        printf("JR NZ, %02X\n", read_memory8(cpu, cpu->pc + 1));

      if (!cpu_check_flag(cpu, flag_c))
        cpu->pc += 2 + read_memory8(cpu, cpu->pc + 1);
      else
        cpu->pc += 2;
    }

    // LD SP, d16
    else if (read_memory8(cpu, cpu->pc) == 0x31) {
      if (dump)
        printf("LD SP, %02X%02X\n", read_memory8(cpu, cpu->pc + 1),
               read_memory8(cpu, cpu->pc + 2));

      cpu_ld_reg16(cpu, sp, read_memory16(cpu, (cpu->pc + 1)));
      cpu->pc += 3;
    }

    // LD (HL-), A
    else if (read_memory8(cpu, cpu->pc) == 0x32) {
      if (dump)
        printf("LD (HL-), A\n");

      cpu_ld_mem8(cpu, (cpu->registers[l] << 8) | cpu->registers[h],
                  cpu->registers[a]);
      cpu->registers[h]--;
      cpu->registers[l]--;
      cpu->pc++;
    }

    // INC SP
    else if (read_memory8(cpu, cpu->pc) == 0x33) {
      if (dump)
        printf("INC SP\n");

      cpu->sp++;

      cpu->pc++;
    }

    // INC (HL)
    else if (read_memory8(cpu, cpu->pc) == 0x34) {
      if (dump)
        printf("INC (HL)\n");

      cpu->registers[h]++;
      cpu->registers[l]++;
      cpu->pc++;
    }

    // DEC H
    else if (read_memory8(cpu, cpu->pc) == 0x35) {
      if (dump)
        printf("DEC H\n");

      cpu->registers[h]--;
      cpu->pc++;
    }

    // LD H, d8
    else if (read_memory8(cpu, cpu->pc) == 0x36) {
      if (dump)
        printf("ld h, %02x\n", read_memory8(cpu, cpu->pc + 1));

      cpu_ld_reg8(cpu, h, read_memory8(cpu, (cpu->pc + 1)));

      cpu->pc += 2;
    }

    // SCF
    else if (read_memory8(cpu, cpu->pc) == 0x37) {
      if (dump)
        printf("SCF\n");

      cpu_set_flag(cpu, flag_c, 1);

      cpu->pc += 1;
    }

    // JR C, i8
    else if (read_memory8(cpu, cpu->pc) == 0x38) {
      if (dump)
        printf("JR C, %02X\n", read_memory8(cpu, cpu->pc + 1));

      if (cpu_check_flag(cpu, flag_c))
        cpu->pc += 2 + read_memory8(cpu, cpu->pc + 1);
      else
        cpu->pc += 2;
    }

    // ADD HL, SP
    else if (read_memory8(cpu, cpu->pc) == 0x39) {
      if (dump)
        printf("ADD HL, SP\n");

      cpu->registers[h] += (cpu->sp >> 8);
      cpu->registers[l] += (cpu->sp << 8);

      cpu->pc++;
    }

    // LD A, (HL+)
    else if (read_memory8(cpu, cpu->pc) == 0x3A) {
      if (dump)
        printf("LD A, (HL-)\n");

      cpu_ld_reg8(
          cpu, a,
          read_memory8(cpu, ((cpu->registers[l] << 8) | cpu->registers[h]--)));

      cpu->pc++;
    }

    // DEC SP
    else if (read_memory8(cpu, cpu->pc) == 0x3B) {
      if (dump)
        printf("DEC SP\n");

      cpu->sp--;

      cpu->pc++;
    }

    // INC A
    else if (read_memory8(cpu, cpu->pc) == 0x3C) {
      if (dump)
        printf("INC A\n");

      cpu->registers[a]++;

      cpu->pc++;
    }

    // DEC A
    else if (read_memory8(cpu, cpu->pc) == 0x3D) {
      if (dump)
        printf("DEC A\n");

      cpu->registers[a]--;

      cpu->pc++;
    }

    // LD A, d8
    else if (read_memory8(cpu, cpu->pc) == 0x3E) {
      if (dump)
        printf("LD A, %02X\n", read_memory8(cpu, cpu->pc + 1));

      cpu_ld_reg8(cpu, a, read_memory8(cpu, (cpu->pc + 1)));

      cpu->pc += 2;
    }

    // CCF
    else if (read_memory8(cpu, cpu->pc) == 0x3F) {
      if (dump)
        printf("CCF\n");

      cpu_toggle_flag(cpu, flag_c);

      cpu->pc++;
    }

    // LD B, B
    else if (read_memory8(cpu, cpu->pc) == 0x40) {
      if (dump)
        printf("LD B, B\n");

      cpu_ld_reg8(cpu, b, cpu->registers[b]);

      cpu->pc++;
    }

    // LD B, C
    else if (read_memory8(cpu, cpu->pc) == 0x41) {
      if (dump)
        printf("LD B, C\n");

      cpu_ld_reg8(cpu, b, cpu->registers[c]);
      cpu->pc++;
    }

    // LD B, D
    else if (read_memory8(cpu, cpu->pc) == 0x42) {
      if (dump)
        printf("LD B, D\n");

      cpu_ld_reg8(cpu, b, cpu->registers[d]);
      cpu->pc++;
    }

    // LD B, E
    else if (read_memory8(cpu, cpu->pc) == 0x43) {
      if (dump)
        printf("LD B, E\n");

      cpu_ld_reg8(cpu, b, cpu->registers[e]);
      cpu->pc++;
    }

    // LD B, H
    else if (read_memory8(cpu, cpu->pc) == 0x44) {
      if (dump)
        printf("LD B, H\n");

      cpu_ld_reg8(cpu, b, cpu->registers[h]);

      cpu->pc++;
    }

    // LD B, L
    else if (read_memory8(cpu, cpu->pc) == 0x45) {
      if (dump)
        printf("LD B, L\n");

      cpu_ld_reg8(cpu, b, cpu->registers[l]);
      cpu->pc++;
    }

    // LD B, (HL)
    else if (read_memory8(cpu, cpu->pc) == 0x46) {
      if (dump)
        printf("LD B, (HL)\n");

      cpu_ld_reg8(cpu, b, ((cpu->registers[l] << 8) | cpu->registers[h]));

      cpu->pc++;
    }

    // LD B, A
    else if (read_memory8(cpu, cpu->pc) == 0x47) {
      if (dump)
        printf("LD B, A\n");

      cpu_ld_reg8(cpu, b, cpu->registers[a]);
      cpu->pc++;
    }

    // LD C, B
    else if (read_memory8(cpu, cpu->pc) == 0x48) {
      if (dump)
        printf("LD C, B\n");

      cpu_ld_reg8(cpu, c, cpu->registers[b]);

      cpu->pc++;
    }

    // LD C, C
    else if (read_memory8(cpu, cpu->pc) == 0x49) {
      if (dump)
        printf("LD C, C\n");

      cpu_ld_reg8(cpu, c, cpu->registers[c]);
      cpu->pc++;
    }

    // LD C, D
    else if (read_memory8(cpu, cpu->pc) == 0x4A) {
      if (dump)
        printf("LD C, D\n");

      cpu_ld_reg8(cpu, c, cpu->registers[d]);
      cpu->pc++;
    }

    // LD C, E
    else if (read_memory8(cpu, cpu->pc) == 0x4B) {
      if (dump)
        printf("LD C, E\n");

      cpu_ld_reg8(cpu, c, cpu->registers[e]);

      cpu->pc++;
    }

    // LD C, H
    else if (read_memory8(cpu, cpu->pc) == 0x4C) {
      if (dump)
        printf("LD C, H\n");

      cpu_ld_reg8(cpu, c, cpu->registers[h]);
      cpu->pc++;
    }

    // LD C, L
    else if (read_memory8(cpu, cpu->pc) == 0x4D) {
      if (dump)
        printf("LD C, L\n");

      cpu_ld_reg8(cpu, c, cpu->registers[l]);
      cpu->pc++;
    }

    // LD C, (HL)
    else if (read_memory8(cpu, cpu->pc) == 0x4E) {
      if (dump)
        printf("LD C, (HL)\n");

      cpu_ld_reg8(cpu, c, ((cpu->registers[l] << 8) | cpu->registers[h]));
      cpu->pc++;
    }

    // LD C, A
    else if (read_memory8(cpu, cpu->pc) == 0x4F) {
      if (dump)
        printf("LD C, A\n");

      cpu_ld_reg8(cpu, c, cpu->registers[a]);
      cpu->pc++;
    }

    // LD D, B
    else if (read_memory8(cpu, cpu->pc) == 0x50) {
      if (dump)
        printf("LD D, B\n");

      cpu_ld_reg8(cpu, d, cpu->registers[b]);
      cpu->pc++;
    }

    // LD D, C
    else if (read_memory8(cpu, cpu->pc) == 0x51) {
      if (dump)
        printf("LD D, C\n");

      cpu_ld_reg8(cpu, d, cpu->registers[c]);
      cpu->pc++;
    }

    // LD D, D
    else if (read_memory8(cpu, cpu->pc) == 0x52) {
      if (dump)
        printf("LD D, D\n");

      cpu_ld_reg8(cpu, d, cpu->registers[d]);
      cpu->pc++;
    }

    // LD D, E
    else if (read_memory8(cpu, cpu->pc) == 0x53) {
      if (dump)
        printf("LD D, E\n");

      cpu_ld_reg8(cpu, d, cpu->registers[e]);
      cpu->pc++;
    }

    // LD D, H
    else if (read_memory8(cpu, cpu->pc) == 0x54) {
      if (dump)
        printf("LD D, H\n");

      cpu_ld_reg8(cpu, d, cpu->registers[h]);
      cpu->pc++;
    }

    // LD D, L
    else if (read_memory8(cpu, cpu->pc) == 0x55) {
      if (dump)
        printf("LD D, L\n");

      cpu_ld_reg8(cpu, d, cpu->registers[l]);
      cpu->pc++;
    }

    // LD D, (HL)
    else if (read_memory8(cpu, cpu->pc) == 0x56) {
      if (dump)
        printf("LD D, (HL)\n");

      cpu_ld_reg8(cpu, d, uint8to16(cpu->registers[h], cpu->registers[l]));
      cpu->pc++;
    }

    // LD D, A
    else if (read_memory8(cpu, cpu->pc) == 0x57) {
      if (dump)
        printf("LD D, A\n");

      cpu_ld_reg8(cpu, d, cpu->registers[a]);
      cpu->pc++;
    }

    // LD E, B
    else if (read_memory8(cpu, cpu->pc) == 0x58) {
      if (dump)
        printf("LD E, B\n");

      cpu_ld_reg8(cpu, e, cpu->registers[b]);
      cpu->pc++;
    }

    // LD E, C
    else if (read_memory8(cpu, cpu->pc) == 0x59) {
      if (dump)
        printf("LD E, C\n");

      cpu_ld_reg8(cpu, e, cpu->registers[c]);
      cpu->pc++;
    }

    // LD E, D
    else if (read_memory8(cpu, cpu->pc) == 0x5A) {
      if (dump)
        printf("LD E, D\n");

      cpu_ld_reg8(cpu, e, cpu->registers[d]);
      cpu->pc++;
    }

    // LD E, E
    else if (read_memory8(cpu, cpu->pc) == 0x5B) {
      if (dump)
        printf("LD E, E\n");

      cpu_ld_reg8(cpu, e, cpu->registers[e]);
      cpu->pc++;
    }

    // LD E, H
    else if (read_memory8(cpu, cpu->pc) == 0x5C) {
      if (dump)
        printf("LD E, H\n");

      cpu_ld_reg8(cpu, e, cpu->registers[h]);
      cpu->pc++;
    }

    // LD E, L
    else if (read_memory8(cpu, cpu->pc) == 0x5D) {
      if (dump)
        printf("LD E, L\n");

      cpu_ld_reg8(cpu, e, cpu->registers[l]);
      cpu->pc++;
    }

    // LD E, (HL)
    else if (read_memory8(cpu, cpu->pc) == 0x5E) {
      if (dump)
        printf("LD E, (HL)\n");

      cpu_ld_reg8(cpu, e, uint8to16(cpu->registers[h], cpu->registers[l]));
      cpu->pc++;
    }

    // LD E, A
    else if (read_memory8(cpu, cpu->pc) == 0x5D) {
      if (dump)
        printf("LD E, A\n");

      cpu_ld_reg8(cpu, e, cpu->registers[a]);
      cpu->pc++;
    }

    // LD H, B
    else if (read_memory8(cpu, cpu->pc) == 0x60) {
      if (dump)
        printf("LD H, B\n");

      cpu_ld_reg8(cpu, h, cpu->registers[b]);
      cpu->pc++;
    }

    // LD H, C
    else if (read_memory8(cpu, cpu->pc) == 0x61) {
      if (dump)
        printf("LD H, C\n");

      cpu_ld_reg8(cpu, h, cpu->registers[c]);
      cpu->pc++;
    }

    // LD H, D
    else if (read_memory8(cpu, cpu->pc) == 0x62) {
      if (dump)
        printf("LD H, D\n");

      cpu_ld_reg8(cpu, h, cpu->registers[d]);
      cpu->pc++;
    }

    // LD H, E
    else if (read_memory8(cpu, cpu->pc) == 0x63) {
      if (dump)
        printf("LD H, E\n");

      cpu_ld_reg8(cpu, h, cpu->registers[e]);
      cpu->pc++;
    }

    // LD H, H
    else if (read_memory8(cpu, cpu->pc) == 0x64) {
      if (dump)
        printf("LD H, H\n");

      cpu_ld_reg8(cpu, h, cpu->registers[h]);
      cpu->pc++;
    }

    // LD H, L
    else if (read_memory8(cpu, cpu->pc) == 0x65) {
      if (dump)
        printf("LD H, L\n");

      cpu_ld_reg8(cpu, h, cpu->registers[l]);
      cpu->pc++;
    }

    // LD H, (HL)
    else if (read_memory8(cpu, cpu->pc) == 0x66) {
      if (dump)
        printf("LD H, (HL)\n");

      cpu_ld_reg8(cpu, h, uint8to16(cpu->registers[h], cpu->registers[l]));

      cpu->pc++;
    }

    // LD H, A
    else if (read_memory8(cpu, cpu->pc) == 0x67) {
      if (dump)
        printf("LD H, A\n");

      cpu_ld_reg8(cpu, h, cpu->registers[a]);
      cpu->pc++;
    }

    // LD L, B
    else if (read_memory8(cpu, cpu->pc) == 0x68) {
      if (dump)
        printf("LD L, B\n");

      cpu_ld_reg8(cpu, l, cpu->registers[b]);
      cpu->pc++;
    }

    // LD L, C
    else if (read_memory8(cpu, cpu->pc) == 0x69) {
      if (dump)
        printf("LD L, C\n");

      cpu_ld_reg8(cpu, l, cpu->registers[c]);
      cpu->pc++;
    }

    // LD L, D
    else if (read_memory8(cpu, cpu->pc) == 0x6A) {
      if (dump)
        printf("LD L, D\n");

      cpu_ld_reg8(cpu, l, cpu->registers[d]);
      cpu->pc++;
    }

    // LD L, E
    else if (read_memory8(cpu, cpu->pc) == 0x6B) {
      if (dump)
        printf("LD L, E\n");

      cpu_ld_reg8(cpu, l, cpu->registers[e]);
      cpu->pc++;
    }

    // LD L, H
    else if (read_memory8(cpu, cpu->pc) == 0x6C) {
      if (dump)
        printf("LD L, H\n");

      cpu_ld_reg8(cpu, l, cpu->registers[h]);
      cpu->pc++;
    }

    // LD L, L
    else if (read_memory8(cpu, cpu->pc) == 0x6D) {
      if (dump)
        printf("LD L, L\n");

      cpu_ld_reg8(cpu, l, cpu->registers[l]);
      cpu->pc++;
    }

    // LD L, (HL)
    else if (read_memory8(cpu, cpu->pc) == 0x6E) {
      if (dump)
        printf("LD L, (HL)\n");

      cpu_ld_reg8(cpu, l, uint8to16(cpu->registers[h], cpu->registers[l]));
      cpu->pc++;
    }

    // LD L, A
    else if (read_memory8(cpu, cpu->pc) == 0x6F) {
      if (dump)
        printf("LD L, A\n");

      cpu_ld_reg8(cpu, l, cpu->registers[a]);
      cpu->pc++;
    }

    // LD (HL), B
    else if (read_memory8(cpu, cpu->pc) == 0x70) {
      if (dump)
        printf("LD (HL), B\n");

      cpu_ld_mem8(cpu, uint8to16(cpu->registers[h], cpu->registers[l]),
                  cpu->registers[b]);
      cpu->pc++;
    }

    // LD (HL), C
    else if (read_memory8(cpu, cpu->pc) == 0x71) {
      if (dump)
        printf("LD (HL), C\n");

      cpu_ld_mem8(cpu, uint8to16(cpu->registers[h], cpu->registers[l]),
                  cpu->registers[c]);
      cpu->pc++;
    }

    // LD (HL), D
    else if (read_memory8(cpu, cpu->pc) == 0x72) {
      if (dump)
        printf("LD (HL), D\n");

      cpu_ld_mem8(cpu, uint8to16(cpu->registers[h], cpu->registers[l]),
                  cpu->registers[d]);
      cpu->pc++;
    }

    // LD (HL), E
    else if (read_memory8(cpu, cpu->pc) == 0x73) {
      if (dump)
        printf("LD (HL), E\n");

      cpu_ld_mem8(cpu, uint8to16(cpu->registers[h], cpu->registers[l]),
                  cpu->registers[e]);
      cpu->pc++;
    }

    // LD (HL), H
    else if (read_memory8(cpu, cpu->pc) == 0x74) {
      if (dump)
        printf("LD (HL), H\n");

      cpu_ld_mem8(cpu, uint8to16(cpu->registers[h], cpu->registers[l]),
                  cpu->registers[h]);
      cpu->pc++;
    }

    // LD (HL), L
    else if (read_memory8(cpu, cpu->pc) == 0x75) {
      if (dump)
        printf("LD (HL), L\n");

      cpu_ld_mem8(cpu, uint8to16(cpu->registers[h], cpu->registers[l]),
                  cpu->registers[l]);
      cpu->pc++;
    }

    // HALT
    else if (read_memory8(cpu, cpu->pc) == 0x76) {
      if (dump)
        printf("HALT\n");

      cpu->halt = true;
    }

    // LD (HL), A
    else if (read_memory8(cpu, cpu->pc) == 0x77) {
      if (dump)
        printf("LD (HL), A\n");

      cpu_ld_mem8(cpu, uint8to16(cpu->registers[h], cpu->registers[l]),
                  cpu->registers[a]);
      cpu->pc++;
    }

    // LD A, B
    else if (read_memory8(cpu, cpu->pc) == 0x78) {
      if (dump)
        printf("LD A, B\n");

      cpu_ld_reg8(cpu, a, cpu->registers[b]);
      cpu->pc++;
    }

    // LD A, C
    else if (read_memory8(cpu, cpu->pc) == 0x79) {
      if (dump)
        printf("LD A, C\n");

      cpu_ld_reg8(cpu, a, cpu->registers[c]);
      cpu->pc++;
    }

    // LD A, D
    else if (read_memory8(cpu, cpu->pc) == 0x7A) {
      if (dump)
        printf("LD A, D\n");

      cpu_ld_reg8(cpu, a, cpu->registers[d]);
      cpu->pc++;
    }

    // LD A, E
    else if (read_memory8(cpu, cpu->pc) == 0x7B) {
      if (dump)
        printf("LD A, E\n");

      cpu_ld_reg8(cpu, a, cpu->registers[e]);
      cpu->pc++;
    }

    // LD A, H
    else if (read_memory8(cpu, cpu->pc) == 0x7C) {
      if (dump)
        printf("LD A, H\n");

      cpu_ld_reg8(cpu, a, cpu->registers[h]);
      cpu->pc++;
    }

    // LD A, L
    else if (read_memory8(cpu, cpu->pc) == 0x7D) {
      if (dump)
        printf("LD A, L\n");

      cpu_ld_reg8(cpu, a, cpu->registers[l]);
      cpu->pc++;
    }

    // LD A, (HL)
    else if (read_memory8(cpu, cpu->pc) == 0x7E) {
      if (dump)
        printf("LD H, (HL)\n");

      cpu_ld_reg8(
          cpu, h,
          read_memory8(cpu, uint8to16(cpu->registers[h], cpu->registers[l])));
      cpu->pc++;
    }

    // LD A, A
    else if (read_memory8(cpu, cpu->pc) == 0x7F) {
      if (dump)
        printf("LD A, A\n");

      cpu_ld_reg8(cpu, a, cpu->registers[a]);
      cpu->pc++;
    }

    // ADD A, B
    else if (read_memory8(cpu, cpu->pc) == 0x80) {
      if (dump)
        printf("ADD A, B\n");

      cpu_add8(cpu, &cpu->registers[a], cpu->registers[b]);

      cpu->pc++;
    }

    // ADD A, C
    else if (read_memory8(cpu, cpu->pc) == 0x81) {
      if (dump)
        printf("ADD A, C\n");

      cpu_add8(cpu, &cpu->registers[a], cpu->registers[c]);

      cpu->pc++;
    }

    // ADD A, D
    else if (read_memory8(cpu, cpu->pc) == 0x82) {
      if (dump)
        printf("ADD A, D\n");

      cpu_add8(cpu, &cpu->registers[a], cpu->registers[d]);

      cpu->pc++;
    }

    // ADD A, E
    else if (read_memory8(cpu, cpu->pc) == 0x83) {
      if (dump)
        printf("ADD A, E\n");

      cpu_add8(cpu, &cpu->registers[a], cpu->registers[e]);

      cpu->pc++;
    }

    // ADD A, H
    else if (read_memory8(cpu, cpu->pc) == 0x84) {
      if (dump)
        printf("ADD A, H\n");

      cpu_add8(cpu, &cpu->registers[a], cpu->registers[h]);

      cpu->pc++;
    }

    // ADD A, L
    else if (read_memory8(cpu, cpu->pc) == 0x85) {
      if (dump)
        printf("ADD A, L\n");

      cpu_add8(cpu, &cpu->registers[a], cpu->registers[l]);

      cpu->pc++;
    }

    // ADD A, (HL)
    else if (read_memory8(cpu, cpu->pc) == 0x86) {
      if (dump)
        printf("ADD A, (HL)\n");

      cpu_add8(
          cpu, &cpu->registers[a],
          read_memory8(cpu, uint8to16(cpu->registers[h], cpu->registers[l])));

      cpu->pc++;
    }

    // ADD A, B
    else if (read_memory8(cpu, cpu->pc) == 0x87) {
      if (dump)
        printf("ADD A, B\n");

      cpu_add8(cpu, &cpu->registers[a], cpu->registers[a]);

      cpu->pc++;
    }

    // ADC A, B
    else if (read_memory8(cpu, cpu->pc) == 0x88) {
      if (dump)
        printf("ADC A, B\n");

      cpu_adc8(cpu, &cpu->registers[a], &cpu->registers[b]);

      cpu->pc++;
    }

    // ADC A, C
    else if (read_memory8(cpu, cpu->pc) == 0x89) {
      if (dump)
        printf("ADC A, C\n");

      cpu_adc8(cpu, &cpu->registers[a], &cpu->registers[c]);

      cpu->pc++;
    }

    // ADC A, D
    else if (read_memory8(cpu, cpu->pc) == 0x8A) {
      if (dump)
        printf("ADC A, D\n");

      cpu_adc8(cpu, &cpu->registers[a], &cpu->registers[d]);

      cpu->pc++;
    }

    // ADC A, E
    else if (read_memory8(cpu, cpu->pc) == 0x8B) {
      if (dump)
        printf("ADC A, E\n");

      cpu_adc8(cpu, &cpu->registers[a], &cpu->registers[e]);

      cpu->pc++;
    }

    // ADC A, H
    else if (read_memory8(cpu, cpu->pc) == 0x8C) {
      if (dump)
        printf("ADC A, H\n");

      cpu_adc8(cpu, &cpu->registers[a], &cpu->registers[h]);

      cpu->pc++;
    }

    // ADC A, L
    else if (read_memory8(cpu, cpu->pc) == 0x8D) {
      if (dump)
        printf("ADC A, L\n");

      cpu_adc8(cpu, &cpu->registers[a], &cpu->registers[l]);

      cpu->pc++;
    }

    // ADC A, (HL)
    else if (read_memory8(cpu, cpu->pc) == 0x8E) {
      if (dump)
        printf("ADC A, (HL)\n");

      cpu_adc8(cpu, &cpu->registers[a],
               mem_ptr8(cpu, read_memory8(cpu, uint8to16(cpu->registers[h],
                                                         cpu->registers[l]))));

      cpu->pc++;
    }

    // ADC A, A
    else if (read_memory8(cpu, cpu->pc) == 0x8F) {
      if (dump)
        printf("ADC A, A\n");

      cpu_adc8(cpu, &cpu->registers[a], &cpu->registers[a]);

      cpu->pc++;
    }

    // SUB A, B
    else if (read_memory8(cpu, cpu->pc) == 0x90) {
      if (dump)
        printf("SUB A, B\n");

      cpu_sub8(cpu, &cpu->registers[a], cpu->registers[b]);

      cpu->pc++;
    }

    // SUB A, C
    else if (read_memory8(cpu, cpu->pc) == 0x91) {
      if (dump)
        printf("SUB A, C\n");

      cpu_sub8(cpu, &cpu->registers[a], cpu->registers[c]);

      cpu->pc++;
    }

    // SUB A, D
    else if (read_memory8(cpu, cpu->pc) == 0x92) {
      if (dump)
        printf("SUB A, D\n");

      cpu_sub8(cpu, &cpu->registers[a], cpu->registers[d]);

      cpu->pc++;
    }

    // SUB A, E
    else if (read_memory8(cpu, cpu->pc) == 0x93) {
      if (dump)
        printf("SUB A, E\n");

      cpu_sub8(cpu, &cpu->registers[a], cpu->registers[e]);

      cpu->pc++;
    }

    // SUB A, H
    else if (read_memory8(cpu, cpu->pc) == 0x94) {
      if (dump)
        printf("SUB A, H\n");

      cpu_sub8(cpu, &cpu->registers[a], cpu->registers[h]);

      cpu->pc++;
    }

    // SUB A, L
    else if (read_memory8(cpu, cpu->pc) == 0x95) {
      if (dump)
        printf("SUB A, L\n");

      cpu_sub8(cpu, &cpu->registers[a], cpu->registers[l]);

      cpu->pc++;
    }

    // SUB A, L
    else if (read_memory8(cpu, cpu->pc) == 0x95) {
      if (dump)
        printf("SUB A, L\n");

      cpu_sub8(cpu, &cpu->registers[a], cpu->registers[l]);

      cpu->pc++;
    }

    // SUB A, (HL)
    else if (read_memory8(cpu, cpu->pc) == 0x96) {
      if (dump)
        printf("SUB A, (HL)\n");

      cpu_sub8(
          cpu, &cpu->registers[a],
          read_memory8(cpu, uint8to16(cpu->registers[h], cpu->registers[l])));

      cpu->pc++;
    }

    // SUB A, A
    else if (read_memory8(cpu, cpu->pc) == 0x97) {
      if (dump)
        printf("SUB A, A\n");

      cpu_sub8(cpu, &cpu->registers[a], cpu->registers[a]);

      cpu->pc++;
    }

    // SBC A, B
    else if (read_memory8(cpu, cpu->pc) == 0x98) {
      if (dump)
        printf("SBC A, B\n");

      cpu_sbc8(cpu, &cpu->registers[a], &cpu->registers[b]);

      cpu->pc++;
    }

    // SBC A, C
    else if (read_memory8(cpu, cpu->pc) == 0x99) {
      if (dump)
        printf("SBC A, C\n");

      cpu_sbc8(cpu, &cpu->registers[a], &cpu->registers[c]);

      cpu->pc++;
    }

    // SBC A, D
    else if (read_memory8(cpu, cpu->pc) == 0x9A) {
      if (dump)
        printf("SBC A, D\n");

      cpu_sbc8(cpu, &cpu->registers[a], &cpu->registers[d]);

      cpu->pc++;
    }

    // SBC A, E
    else if (read_memory8(cpu, cpu->pc) == 0x9B) {
      if (dump)
        printf("SBC A, E\n");

      cpu_sbc8(cpu, &cpu->registers[a], &cpu->registers[e]);

      cpu->pc++;
    }

    // SBC A, H
    else if (read_memory8(cpu, cpu->pc) == 0x9C) {
      if (dump)
        printf("SBC A, C\n");

      cpu_sbc8(cpu, &cpu->registers[a], &cpu->registers[c]);

      cpu->pc++;
    }

    // SBC A, L
    else if (read_memory8(cpu, cpu->pc) == 0x9D) {
      if (dump)
        printf("SBC A, L\n");

      cpu_sbc8(cpu, &cpu->registers[a], &cpu->registers[l]);

      cpu->pc++;
    }

    // SBC A, (HL)
    else if (read_memory8(cpu, cpu->pc) == 0x9E) {
      if (dump)
        printf("SBC A, (HL)\n");

      cpu_sbc8(cpu, &cpu->registers[a],
               mem_ptr8(cpu, uint8to16(cpu->registers[h], cpu->registers[l])));

      cpu->pc++;
    }

    // SBC A, A
    else if (read_memory8(cpu, cpu->pc) == 0x9F) {
      if (dump)
        printf("SBC A, A\n");

      cpu_sbc8(cpu, &cpu->registers[a], &cpu->registers[a]);

      cpu->pc++;
    }

    // AND A, B
    else if (read_memory8(cpu, cpu->pc) == 0xA0) {
      if (dump)
        printf("AND A, B\n");

      cpu_and(cpu, &cpu->registers[a], &cpu->registers[b]);

      cpu->pc++;
    }

    // AND A, C
    else if (read_memory8(cpu, cpu->pc) == 0xA1) {
      if (dump)
        printf("AND A, C\n");

      cpu_and(cpu, &cpu->registers[a], &cpu->registers[c]);

      cpu->pc++;
    }

    // AND A, D
    else if (read_memory8(cpu, cpu->pc) == 0xA2) {
      if (dump)
        printf("AND A, D\n");

      cpu_and(cpu, &cpu->registers[a], &cpu->registers[d]);

      cpu->pc++;
    }

    // AND A, E
    else if (read_memory8(cpu, cpu->pc) == 0xA3) {
      if (dump)
        printf("AND A, E\n");

      cpu_and(cpu, &cpu->registers[a], &cpu->registers[e]);

      cpu->pc++;
    }

    // AND A, H
    else if (read_memory8(cpu, cpu->pc) == 0xA4) {
      if (dump)
        printf("AND A, H\n");

      cpu_and(cpu, &cpu->registers[a], &cpu->registers[h]);

      cpu->pc++;
    }

    // AND A, L
    else if (read_memory8(cpu, cpu->pc) == 0xA5) {
      if (dump)
        printf("AND A, L\n");

      cpu_and(cpu, &cpu->registers[a], &cpu->registers[l]);

      cpu->pc++;
    }

    // AND A, (HL)
    else if (read_memory8(cpu, cpu->pc) == 0xA6) {
      if (dump)
        printf("AND A, (HL)\n");

      cpu_and(cpu, &cpu->registers[a],
              mem_ptr8(cpu, uint8to16(cpu->registers[h], cpu->registers[l])));

      cpu->pc++;
    }

    // AND A, A
    else if (read_memory8(cpu, cpu->pc) == 0xA7) {
      if (dump)
        printf("AND A, A\n");

      cpu_and(cpu, &cpu->registers[a], &cpu->registers[a]);

      cpu->pc++;
    }

    // XOR A, B
    else if (read_memory8(cpu, cpu->pc) == 0xA8) {
      if (dump)
        printf("XOR A, B\n");

      cpu_xor(cpu, &cpu->registers[a], &cpu->registers[b]);

      cpu->pc++;
    }

    // XOR A, C
    else if (read_memory8(cpu, cpu->pc) == 0xA9) {
      if (dump)
        printf("XOR A, C\n");

      cpu_xor(cpu, &cpu->registers[a], &cpu->registers[c]);

      cpu->pc++;
    }

    // XOR A, D
    else if (read_memory8(cpu, cpu->pc) == 0xAA) {
      if (dump)
        printf("XOR A, D\n");

      cpu_xor(cpu, &cpu->registers[a], &cpu->registers[d]);

      cpu->pc++;
    }

    // XOR A, E
    else if (read_memory8(cpu, cpu->pc) == 0xAB) {
      if (dump)
        printf("XOR A, E\n");

      cpu_xor(cpu, &cpu->registers[a], &cpu->registers[e]);

      cpu->pc++;
    }

    // XOR A, H
    else if (read_memory8(cpu, cpu->pc) == 0xAC) {
      if (dump)
        printf("XOR A, H\n");

      cpu_xor(cpu, &cpu->registers[a], &cpu->registers[h]);

      cpu->pc++;
    }

    // XOR A, L
    else if (read_memory8(cpu, cpu->pc) == 0xAD) {
      if (dump)
        printf("XOR A, H\n");

      cpu_xor(cpu, &cpu->registers[a], &cpu->registers[l]);

      cpu->pc++;
    }

    // XOR A, (HL)
    else if (read_memory8(cpu, cpu->pc) == 0xAE) {
      if (dump)
        printf("XOR A, (HL)\n");

      cpu_xor(cpu, &cpu->registers[a],
              mem_ptr8(cpu, uint8to16(cpu->registers[h], cpu->registers[l])));

      cpu->pc++;
    }

    // XOR A, A
    else if (read_memory8(cpu, cpu->pc) == 0xAF) {
      if (dump)
        printf("XOR A, A\n");

      cpu_xor(cpu, &cpu->registers[a], &cpu->registers[a]);

      cpu->pc++;
    }

    // OR A, B
    else if (read_memory8(cpu, cpu->pc) == 0xB0) {
      if (dump)
        printf("OR A, B\n");

      cpu_or(cpu, &cpu->registers[a], &cpu->registers[b]);

      cpu->pc++;
    }

    // OR A, C
    else if (read_memory8(cpu, cpu->pc) == 0xB1) {
      if (dump)
        printf("OR A, C\n");

      cpu_or(cpu, &cpu->registers[a], &cpu->registers[c]);

      cpu->pc++;
    }

    // OR A, D
    else if (read_memory8(cpu, cpu->pc) == 0xB2) {
      if (dump)
        printf("OR A, D\n");

      cpu_or(cpu, &cpu->registers[a], &cpu->registers[d]);

      cpu->pc++;
    }

    // OR A, E
    else if (read_memory8(cpu, cpu->pc) == 0xB3) {
      if (dump)
        printf("OR A, E\n");

      cpu_or(cpu, &cpu->registers[a], &cpu->registers[e]);

      cpu->pc++;
    }

    // OR A, H
    else if (read_memory8(cpu, cpu->pc) == 0xB4) {
      if (dump)
        printf("OR A, H\n");

      cpu_or(cpu, &cpu->registers[a], &cpu->registers[h]);

      cpu->pc++;
    }

    // OR A, L
    else if (read_memory8(cpu, cpu->pc) == 0xB5) {
      if (dump)
        printf("OR A, L\n");

      cpu_or(cpu, &cpu->registers[a], &cpu->registers[l]);

      cpu->pc++;
    }

    // OR A, (HL)
    else if (read_memory8(cpu, cpu->pc) == 0xB6) {
      if (dump)
        printf("OR A, (HL)\n");

      cpu_or(cpu, &cpu->registers[a],
             mem_ptr8(cpu, uint8to16(cpu->registers[h], cpu->registers[l])));

      cpu->pc++;
    }

    // OR A, A
    else if (read_memory8(cpu, cpu->pc) == 0xB7) {
      if (dump)
        printf("OR A, A\n");

      cpu_or(cpu, &cpu->registers[a], &cpu->registers[a]);

      cpu->pc++;
    }

    // CP A, B
    else if (read_memory8(cpu, cpu->pc) == 0xB8) {
      if (dump)
        printf("CP A, B\n");

      cpu_cp8(cpu, &cpu->registers[a], &cpu->registers[b]);

      cpu->pc++;
    }

    // CP A, C
    else if (read_memory8(cpu, cpu->pc) == 0xB9) {
      if (dump)
        printf("CP A, C\n");

      cpu_cp8(cpu, &cpu->registers[a], &cpu->registers[c]);

      cpu->pc++;
    }

    // CP A, D
    else if (read_memory8(cpu, cpu->pc) == 0xBA) {
      if (dump)
        printf("CP A, D\n");

      cpu_cp8(cpu, &cpu->registers[a], &cpu->registers[d]);

      cpu->pc++;
    }

    // CP A, E
    else if (read_memory8(cpu, cpu->pc) == 0xBB) {
      if (dump)
        printf("CP A, E\n");

      cpu_cp8(cpu, &cpu->registers[a], &cpu->registers[e]);

      cpu->pc++;
    }

    // CP A, H
    else if (read_memory8(cpu, cpu->pc) == 0xBC) {
      if (dump)
        printf("CP A, H\n");

      cpu_cp8(cpu, &cpu->registers[a], &cpu->registers[h]);

      cpu->pc++;
    }

    // CP A, L
    else if (read_memory8(cpu, cpu->pc) == 0xBD) {
      if (dump)
        printf("CP A, L\n");

      cpu_cp8(cpu, &cpu->registers[a], &cpu->registers[l]);

      cpu->pc++;
    }

    // CP A, (HL)
    else if (read_memory8(cpu, cpu->pc) == 0xBE) {
      if (dump)
        printf("CP A, (HL)\n");

      cpu_cp8(cpu, &cpu->registers[a],
              mem_ptr8(cpu, uint8to16(cpu->registers[h], cpu->registers[l])));

      cpu->pc++;
    }

    // CP A, A
    else if (read_memory8(cpu, cpu->pc) == 0xBF) {
      if (dump)
        printf("CP A, A\n");

      cpu_cp8(cpu, &cpu->registers[a], &cpu->registers[a]);

      cpu->pc++;
    }

    // RET NZ
    else if (read_memory8(cpu, cpu->pc) == 0xC0) {
      if (dump)
        printf("RET NZ\n");
      if (!cpu_check_flag(cpu, flag_z)) {
        cpu_ret(cpu);

      } else {
        cpu->pc++;
      }
    }

    // POP BC
    else if (read_memory8(cpu, cpu->pc) == 0xC1) {
      if (dump)
        printf("POP BC\n");

      cpu_pop(cpu, &cpu->registers[a], &cpu->registers[b]);
      cpu->pc++;
    }

    // JP NZ, u16
    else if (read_memory8(cpu, cpu->pc) == 0xC2) {
      if (dump)
        printf("JP NZ, %04X\n", uint8to16(cpu->pc, cpu->pc + 2));

      if (!cpu_check_flag(cpu, flag_z)) {
        cpu_jp(cpu, cpu->pc++);
        cpu->pc += 3;
      } else {
        cpu->pc += 3;
      }
    }

    // JP u16
    else if (read_memory8(cpu, cpu->pc) == 0xC3) {
      if (dump)
        printf("JP %04X\n", uint8to16(cpu->pc, cpu->pc + 2));

      cpu_jp(cpu, cpu->pc++);
      cpu->pc += 3;
    }

    // CALL NZ, u16
    else if (read_memory8(cpu, cpu->pc) == 0xC4) {
      if (dump)
        printf("CALL NZ, %04X\n", uint8to16(cpu->pc, cpu->pc + 2));

      if (!cpu_check_flag(cpu, flag_z))
        cpu_call(cpu, uint8to16(cpu->pc, cpu->pc + 2));
      else
        cpu->pc += 3;
    }

    // PUSH BC
    else if (read_memory8(cpu, cpu->pc) == 0xC5) {
      if (dump)
        printf("PUSH BC\n");

      cpu_push(cpu, &cpu->registers[a], &cpu->registers[b]);
      cpu->pc++;
    }

    // ADD A, u8
    else if (read_memory8(cpu, cpu->pc) == 0xC6) {
      if (dump)
        printf("ADD A, u8\n");

      cpu_add8(cpu, &cpu->registers[a], read_memory8(cpu, cpu->pc + 1));

      cpu->pc += 2;
    }

    // RST 00h
    else if (read_memory8(cpu, cpu->pc) == 0xC7) {
      if (dump)
        printf("RST 00h\n");

      cpu_rst(cpu, 0x00);
      cpu->pc++;
    }

    // RET Z
    else if (read_memory8(cpu, cpu->pc) == 0xC8) {
      if (dump)
        printf("RET Z\n");
      if (cpu_check_flag(cpu, flag_z)) {
        cpu_ret(cpu);

      } else {
        cpu->pc++;
      }
    }

    // RET
    else if (read_memory8(cpu, cpu->pc) == 0xC9) {
      if (dump)
        printf("RET\n");
      cpu_ret(cpu);
      cpu->pc++;
    }

    // JP Z, u16
    else if (read_memory8(cpu, cpu->pc) == 0xCA) {
      if (dump)
        printf("JP Z, %04X\n", uint8to16(cpu->pc, cpu->pc + 2));

      if (cpu_check_flag(cpu, flag_z)) {
        cpu_jp(cpu, cpu->pc++);
        cpu->pc += 3;
      } else {
        cpu->pc += 3;
      }
    }

    // PREFIX CB
    // TODO: Implement
    else if (read_memory8(cpu, cpu->pc) == 0xCB) {
      if (dump)
        printf("PREFIX CB (NOT IMPLEMENTED!!!)\n");

      cpu->pc += 2;
    }

    // CALL Z, u16
    else if (read_memory8(cpu, cpu->pc) == 0xCC) {
      if (dump)
        printf("CALL Z, %04X\n", uint8to16(cpu->pc, cpu->pc + 2));

      if (cpu_check_flag(cpu, flag_z))
        cpu_call(cpu, uint8to16(cpu->pc, cpu->pc + 2));
      else
        cpu->pc += 3;
    }

    // CALL, u16
    else if (read_memory8(cpu, cpu->pc) == 0xCC) {
      if (dump)
        printf("CALL, %04X\n", uint8to16(cpu->pc, cpu->pc + 2));

      cpu_call(cpu, uint8to16(cpu->pc, cpu->pc + 2));
    }

    // ADC A, u8
    else if (read_memory8(cpu, cpu->pc) == 0xCD) {
      if (dump)
        printf("ADC A, %02X\n", cpu->pc + 1);

      cpu_adc8(cpu, &cpu->registers[a], (uint8_t *)&cpu->pc + 1);
      cpu->pc++;
    }

    // RST 08h
    else if (read_memory8(cpu, cpu->pc) == 0xCE) {
      if (dump)
        printf("RST 08h\n");

      cpu_rst(cpu, 0x08);
      cpu->pc++;
    }

    // RET NZ
    else if (read_memory8(cpu, cpu->pc) == 0xD0) {
      if (dump)
        printf("RET NZ\n");

      if (!cpu_check_flag(cpu, flag_z))
        cpu_ret(cpu);

      cpu->pc++;
    }

    // POP DE
    else if (read_memory8(cpu, cpu->pc) == 0xD1) {
      if (dump)
        printf("POP DE\n");

      cpu_pop(cpu, &cpu->registers[d], &cpu->registers[e]);
      cpu->pc++;
    }

    // JP NZ, u16
    else if (read_memory8(cpu, cpu->pc) == 0xD2) {
      if (dump)
        printf("JP NZ, %04X\n", uint8to16(cpu->pc, cpu->pc + 2));

      if (!cpu_check_flag(cpu, flag_z)) {
        cpu_jp(cpu, cpu->pc++);
        cpu->pc += 3;
      } else {
        cpu->pc += 3;
      }
    }

    // Invalid
    else if (read_memory8(cpu, cpu->pc) == 0xD3) {
      if (dump)
        printf("Invalid instruction: 0x%02X\n", read_memory8(cpu, cpu->pc));

      cpu->pc += 1;
    }

    // CALL NZ, u16
    else if (read_memory8(cpu, cpu->pc) == 0xD4) {
      if (dump)
        printf("CALL NZ, %04X\n", uint8to16(cpu->pc, cpu->pc + 2));

      if (!cpu_check_flag(cpu, flag_z))
        cpu_call(cpu, uint8to16(cpu->pc, cpu->pc + 2));
      else
        cpu->pc += 3;
    }

    // PUSH DE
    else if (read_memory8(cpu, cpu->pc) == 0xD5) {
      if (dump)
        printf("PUSH DE\n");

      cpu_push(cpu, &cpu->registers[d], &cpu->registers[e]);
      cpu->pc++;
    }

    // SUB A, u8
    else if (read_memory8(cpu, cpu->pc) == 0xD6) {
      if (dump)
        printf("SUB A, u8\n");

      cpu_sub8(cpu, &cpu->registers[a], read_memory8(cpu, cpu->pc + 1));

      cpu->pc += 2;
    }

    // RST 10h
    else if (read_memory8(cpu, cpu->pc) == 0xD7) {
      if (dump)
        printf("RST 10h\n");

      cpu_rst(cpu, 0x10);
      cpu->pc++;
    }

    // RET C
    else if (read_memory8(cpu, cpu->pc) == 0xD8) {
      if (dump)
        printf("RET C\n");

      if (cpu_check_flag(cpu, flag_c))
        cpu_ret(cpu);

      cpu->pc++;
    }

    // RETI
    else if (read_memory8(cpu, cpu->pc) == 0xD9) {
      if (dump)
        printf("RETI\n");

      cpu_ret(cpu);
      cpu->ime = true;

      cpu->pc++;
    }

    // JP C, u16
    else if (read_memory8(cpu, cpu->pc) == 0xDA) {
      if (dump)
        printf("JP C, %04X\n", uint8to16(cpu->pc, cpu->pc + 2));

      if (cpu_check_flag(cpu, flag_c)) {
        cpu_jp(cpu, cpu->pc++);
        cpu->pc += 3;
      } else {
        cpu->pc += 3;
      }
    }

    // Invalid
    else if (read_memory8(cpu, cpu->pc) == 0xDB) {
      if (dump)
        printf("Invalid instruction: 0x%02X\n", read_memory8(cpu, cpu->pc));

      cpu->pc += 1;
    }

    // CALL C, u16
    else if (read_memory8(cpu, cpu->pc) == 0xDC) {
      if (dump)
        printf("CALL C, %04X\n", uint8to16(cpu->pc, cpu->pc + 2));

      if (cpu_check_flag(cpu, flag_c))
        cpu_call(cpu, uint8to16(cpu->pc, cpu->pc + 2));
      else
        cpu->pc += 3;
    }

    // Invalid
    else if (read_memory8(cpu, cpu->pc) == 0xDD) {
      if (dump)
        printf("Invalid instruction: 0x%02X\n", read_memory8(cpu, cpu->pc));

      cpu->pc += 1;
    }

    // SBC A, u8
    else if (read_memory8(cpu, cpu->pc) == 0xDE) {
      if (dump)
        printf("SBC A, %02X\n", cpu->pc + 1);

      cpu_sbc8(cpu, &cpu->registers[a], (uint8_t *)&cpu->pc + 1);
      cpu->pc++;
    }

    // RST 18h
    else if (read_memory8(cpu, cpu->pc) == 0xDF) {
      if (dump)
        printf("RST 18h\n");

      cpu_rst(cpu, 0x18);
      cpu->pc++;
    }

    else {
      if (dump)
        printf("Invalid opcode: 0x%02X\n", read_memory8(cpu, cpu->pc));

      cpu->halt = true;
    }

    if (print_regs) {
      printf("cpu_execute(): --End--\n\n");
      sleep(3);
    }
  }
}
