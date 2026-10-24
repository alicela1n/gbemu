#include "cpu.h"
#include "helper.h"
#include "memory.h"
#include <stddef.h>
#include <stdio.h>

void cpu_cb(struct CPU *cpu, bool dump);

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
  memset(cpu->mem, 0, 0x7FFF);

  // CPU is running so halt = false
  cpu->halt = false;

  // Allocate rom struct
  cpu->rom = calloc(1, sizeof(struct ROM));
}

void cpu_cleanup(struct CPU *cpu) {
  rom_cleanup(cpu->rom);
}

void cpu_halt_loop(struct CPU *cpu, bool dump) {
  if (cpu->halt) {

    if (dump) {
      printf("cpu_halt_loop(): CPU Halted!\n");
      printf("cpu_halt_loop(): Press enter to unhalt...\n");
    }

    while (cpu->halt) {
      getchar();

      if (dump) {
        printf("Continuing...\n");
      }

      cpu->halt = false;
    }
  }
}

/* cpu_execute(): What this does is decode each opcode and translate it into
 * an instruction to be cpu_execute by the virtual CPU. Optional dump flag
 * for disassembly upon execution.
 */
void cpu_execute(struct CPU *cpu, bool dump, bool print_regs, bool print_memory, bool stop_on_halt) {
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
      printf("cpu_execute(): pc: 0x%02X; 0x%02X\n", cpu->pc, read_memory8(cpu, cpu->pc));
      printf("cpu_execute(): Flags\n\n");
      printf("cpu_execute(): z: %02X, n: %02X, h: %02X, c: %02X\n",
             cpu_check_flag(cpu, flag_z), cpu_check_flag(cpu, flag_n),
             cpu_check_flag(cpu, flag_h), cpu_check_flag(cpu, flag_c));

      printf("\n");
      printf("cpu_execute(): Link port state: 0x%02X\n", read_memory8(cpu, 0xff02));
      printf("cpu_execute(): Link port output: %c\n", read_memory8(cpu, 0xff01));
    }

    if (print_memory) {
      for (uint16_t i = 0x8000; i < 0xFFFF; i++) {
        printf("%02X: 0x%02X ", i, read_memory8(cpu, i));
      }
      printf("\n");
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

      cpu_ld_mem8(cpu, cpu_read_reg16(cpu, bc),
                  cpu->registers[a]);

      cpu->pc++;
    }

    // INC BC
    else if (read_memory8(cpu, cpu->pc) == 0x03) {
      if (dump)
        printf("INC BC\n");

      cpu_inc_reg16(cpu, bc, 1);

      cpu->pc++;
    }

    // INC B
    else if (read_memory8(cpu, cpu->pc) == 0x04) {
      if (dump)
        printf("INC B\n");

      cpu_inc_reg8(cpu, b, 1);
      cpu->pc++;
    }

    // DEC B
    else if (read_memory8(cpu, cpu->pc) == 0x05) {
      if (dump)
        printf("DEC B\n");

      cpu_dec_reg8(cpu, b, 1);
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

      cpu_add_reg16(cpu, hl, bc);

      cpu->pc++;
    }

    // LD A, (BC)
    else if (read_memory8(cpu, cpu->pc) == 0x0A) {
      if (dump)
        printf("LD A, (BC)\n");

      cpu_ld_reg8(
          cpu, a,
          read_memory8(cpu, cpu_read_reg16(cpu, bc)));

      cpu->pc++;
    }

    // DEC BC
    else if (read_memory8(cpu, cpu->pc) == 0x0B) {
      if (dump)
        printf("DEC BC\n");

      cpu_dec_reg16(cpu, bc, 1);

      cpu->pc++;
    }

    // INC C
    else if (read_memory8(cpu, cpu->pc) == 0x0C) {
      if (dump)
        printf("INC C\n");

      cpu_inc_reg8(cpu, c, 1);

      cpu->pc++;
    }

    // DEC C
    else if (read_memory8(cpu, cpu->pc) == 0x0D) {
      if (dump)
        printf("DEC C\n");

      cpu_dec_reg8(cpu, c, 1);

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

      cpu_ld_reg16(cpu, de, uint8to16(read_memory16(cpu, cpu->pc + 1), read_memory16(cpu, cpu->pc + 2)));
      cpu->pc += 3;
    }

    // LD (DE), A
    else if (read_memory8(cpu, cpu->pc) == 0x12) {
      if (dump)
        printf("LD (DE), A\n");

      cpu_ld_mem8(cpu, cpu_read_reg16(cpu, de),
                  cpu->registers[a]);

      cpu->pc++;
    }

    // INC DE
    else if (read_memory8(cpu, cpu->pc) == 0x13) {
      if (dump)
        printf("INC DE\n");

      cpu_inc_reg16(cpu, de, 1);

      cpu->pc++;
    }

    // INC D
    else if (read_memory8(cpu, cpu->pc) == 0x14) {
      if (dump)
        printf("INC D\n");

      cpu_inc_reg16(cpu, d, 1);
      cpu->pc++;
    }

    // DEC D
    else if (read_memory8(cpu, cpu->pc) == 0x15) {
      if (dump)
        printf("DEC D\n");

      cpu_dec_reg8(cpu, d, 1);
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

      uint8_t offset = read_memory8(cpu, cpu->pc + 1);
      cpu->pc += 2;
      cpu_jr(cpu, read_memory8(cpu, offset));
    }

    // ADD HL, DE
    else if (read_memory8(cpu, cpu->pc) == 0x19) {
      if (dump)
        printf("ADD HL, DE\n");

      cpu_add_reg16(cpu, hl, de);

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

      cpu_dec_reg16(cpu, de, 1);

      cpu->pc++;
    }

    // INC E
    else if (read_memory8(cpu, cpu->pc) == 0x1C) {
      if (dump)
        printf("INC E\n");

      cpu_inc_reg16(cpu, e, 1);

      cpu->pc++;
    }

    // DEC E
    else if (read_memory8(cpu, cpu->pc) == 0x1D) {
      if (dump)
        printf("DEC E\n");

      cpu_dec_reg8(cpu, e, 1);

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
        cpu_jp(cpu, read_memory8(cpu, cpu->pc + 1));
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

      cpu_inc_reg16(cpu, hl, 1);

      cpu->pc++;
    }

    // INC H
    else if (read_memory8(cpu, cpu->pc) == 0x24) {
      if (dump)
        printf("INC H\n");
      
      cpu_inc_reg8(cpu, h, 1);

      cpu->pc++;
    }

    // DEC H
    else if (read_memory8(cpu, cpu->pc) == 0x25) {
      if (dump)
        printf("DEC H\n");

      cpu_dec_reg8(cpu, h, 1);
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

      if (cpu_check_flag(cpu, flag_z)) {
        uint8_t offset = read_memory8(cpu, cpu->pc + 1);
        cpu->pc += 2;
        cpu_jr(cpu, read_memory8(cpu, offset));
      } else {
        cpu->pc += 2;
      }
    }

    // ADD HL, HL
    else if (read_memory8(cpu, cpu->pc) == 0x29) {
      if (dump)
        printf("ADD HL, DE\n");

      cpu_add_reg16(cpu, hl, de);

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

      cpu_dec_reg16(cpu, de, 1);
      cpu->pc++;
    }

    // INC L
    else if (read_memory8(cpu, cpu->pc) == 0x2C) {
      if (dump)
        printf("INC L\n");

      cpu_inc_reg8(cpu, l, 1);

      cpu->pc++;
    }

    // DEC L
    else if (read_memory8(cpu, cpu->pc) == 0x2D) {
      if (dump)
        printf("DEC L\n");

      cpu_dec_reg8(cpu, l, 1);

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

      if (!cpu_check_flag(cpu, flag_c)) {
        uint8_t offset = read_memory8(cpu, cpu->pc + 1);
        cpu->pc += 2;
        cpu_jr(cpu, read_memory8(cpu, offset));
      }
      else {
        cpu->pc += 2;
      }
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

      cpu_inc_reg8(cpu, sp, 1);

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

      cpu_dec_reg8(cpu, h, 1);
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

      if (cpu_check_flag(cpu, flag_c)) {
        uint8_t offset = read_memory8(cpu, cpu->pc + 1);
        cpu->pc += 2;
        cpu_jr(cpu, read_memory8(cpu, offset));
      }
      else {
        cpu->pc += 2;
      }
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

      cpu_dec_reg16(cpu, sp, 1);

      cpu->pc++;
    }

    // INC A
    else if (read_memory8(cpu, cpu->pc) == 0x3C) {
      if (dump)
        printf("INC A\n");

      cpu_inc_reg8(cpu, a, 1);

      cpu->pc++;
    }

    // DEC A
    else if (read_memory8(cpu, cpu->pc) == 0x3D) {
      if (dump)
        printf("DEC A\n");

      cpu_dec_reg8(cpu, a, 1);

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

      cpu_ld_reg8(cpu, b,  cpu_read_reg16(cpu, hl));

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

      cpu_ld_reg8(cpu, c,  cpu_read_reg16(cpu, hl));
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

      cpu_ld_reg8(cpu, l,  cpu_read_reg16(cpu, hl));
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

      cpu_ld_mem8(cpu, cpu_read_reg16(cpu, hl),
                  cpu->registers[b]);
      cpu->pc++;
    }

    // LD (HL), C
    else if (read_memory8(cpu, cpu->pc) == 0x71) {
      if (dump)
        printf("LD (HL), C\n");

      cpu_ld_mem8(cpu, cpu_read_reg16(cpu, hl),
                  cpu->registers[c]);
      cpu->pc++;
    }

    // LD (HL), D
    else if (read_memory8(cpu, cpu->pc) == 0x72) {
      if (dump)
        printf("LD (HL), D\n");

      cpu_ld_mem8(cpu,  cpu_read_reg16(cpu, hl),
                  cpu->registers[d]);
      cpu->pc++;
    }

    // LD (HL), E
    else if (read_memory8(cpu, cpu->pc) == 0x73) {
      if (dump)
        printf("LD (HL), E\n");

      cpu_ld_mem8(cpu,  cpu_read_reg16(cpu, hl),
                  cpu->registers[e]);
      cpu->pc++;
    }

    // LD (HL), H
    else if (read_memory8(cpu, cpu->pc) == 0x74) {
      if (dump)
        printf("LD (HL), H\n");

      cpu_ld_mem8(cpu,  cpu_read_reg16(cpu, hl),
                  cpu->registers[h]);
      cpu->pc++;
    }

    // LD (HL), L
    else if (read_memory8(cpu, cpu->pc) == 0x75) {
      if (dump)
        printf("LD (HL), L\n");

      cpu_ld_mem8(cpu,  cpu_read_reg16(cpu, hl),
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

      cpu_ld_mem8(cpu,  cpu_read_reg16(cpu, hl),
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
          read_memory8(cpu,  cpu_read_reg16(cpu, hl)));
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
          read_memory8(cpu,  cpu_read_reg16(cpu, hl)));

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
               mem_ptr8(cpu, read_memory8(cpu, uint8to16(cpu->registers[l],
                                                         cpu->registers[h]))));

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
          read_memory8(cpu,  cpu_read_reg16(cpu, hl)));

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
               mem_ptr8(cpu,  cpu_read_reg16(cpu, hl)));

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
              mem_ptr8(cpu,  cpu_read_reg16(cpu, hl)));

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
              mem_ptr8(cpu,  cpu_read_reg16(cpu, hl)));

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
             mem_ptr8(cpu,  cpu_read_reg16(cpu, hl)));

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
              mem_ptr8(cpu,  cpu_read_reg16(cpu, hl)));

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

      cpu_cb(cpu, dump);

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

      cpu->halt = true;
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
      cpu->ime_r = true;

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

      cpu->halt = true;
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

      cpu->halt = true;
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

    // LD (FF00+u8), A
    else if (read_memory8(cpu, cpu->pc) == 0xE0) {
      if (dump)
        printf("LD (FF00+%02X), A\n", read_memory8(cpu, cpu->pc + 1));

      cpu_ld_mem8(cpu, 0xFF00 + cpu->pc + 1, cpu->registers[a]);
      cpu->pc += 2;
    }

    // POP HL
    else if (read_memory8(cpu, cpu->pc) == 0xE1) {
      if (dump)
        printf("POP HL\n");

      cpu_pop(cpu, &cpu->registers[h], &cpu->registers[l]);
      cpu->pc++;
    }

    // LD (FF00+C), A
    else if (read_memory8(cpu, cpu->pc) == 0xE2) {
      if (dump)
        printf("LD (FF00+C), A\n");

      cpu_ld_mem8(cpu, 0xFF00 + cpu->pc + 1, cpu->registers[a]);
      cpu->pc += 2;
    }

    // Invalid
    else if (read_memory8(cpu, cpu->pc) == 0xE3) {
      if (dump)
        printf("Invalid instruction: 0x%02X\n", read_memory8(cpu, cpu->pc));

      cpu->halt = true;
    }

    // Invalid
    else if (read_memory8(cpu, cpu->pc) == 0xE4) {
      if (dump)
        printf("Invalid instruction: 0x%02X\n", read_memory8(cpu, cpu->pc));

      cpu->halt = true;
    }

    // PUSH HL
    else if (read_memory8(cpu, cpu->pc) == 0xE5) {
      if (dump)
        printf("PUSH HL\n");

      cpu_push(cpu, &cpu->registers[h], &cpu->registers[l]);
      cpu->pc++;
    }

    // AND A, u8
    else if (read_memory8(cpu, cpu->pc) == 0xE6) {
      if (dump)
        printf("AND A, %020X\n", read_memory8(cpu, cpu->pc++));

      cpu_and(cpu, &cpu->registers[a], mem_ptr8(cpu, cpu->pc++));

      cpu->pc += 3;
    }

    // RST 20h
    else if (read_memory8(cpu, cpu->pc) == 0xE7) {
      if (dump)
        printf("RST 20h\n");

      cpu_rst(cpu, 0x20);
      cpu->pc++;
    }

    // ADD SP, i8
    else if (read_memory8(cpu, cpu->pc) == 0xE8) {
      if (dump)
        printf("ADD SP, %020X\n", read_memory8(cpu, cpu->pc + 1));

      cpu_add8(cpu, (uint8_t *)&cpu->sp, read_memory8(cpu, cpu->pc + 1));

      cpu->pc += 2;
    }

    // JP HL
    else if (read_memory8(cpu, cpu->pc) == 0xE9) {
      if (dump)
        printf("JP %04X\n", uint8to16(cpu->registers[h], cpu->registers[l]));

      cpu_jp(cpu, uint8to16(cpu->registers[h], cpu->registers[l]));
      cpu->pc += 3;
    }

    // LD (u16), A
    else if (read_memory8(cpu, cpu->pc) == 0xEA) {
      if (dump)
        printf("LD (u16), A\n");

      cpu_ld_mem8(cpu, uint8to16(cpu->pc++, cpu->pc + 2), cpu->registers[a]);

      cpu->pc += 3;
    }

    // Invalid
    else if (read_memory8(cpu, cpu->pc) == 0xEB) {
      if (dump)
        printf("Invalid instruction: 0x%02X\n", read_memory8(cpu, cpu->pc));

      cpu->halt = true;
    }

    // Invalid
    else if (read_memory8(cpu, cpu->pc) == 0xEC) {
      if (dump)
        printf("Invalid instruction: 0x%02X\n", read_memory8(cpu, cpu->pc));

      cpu->halt = true;
    }

    // Invalid
    else if (read_memory8(cpu, cpu->pc) == 0xED) {
      if (dump)
        printf("Invalid instruction: 0x%02X\n", read_memory8(cpu, cpu->pc));

      cpu->halt = true;
    }

    // XOR A, u8
    else if (read_memory8(cpu, cpu->pc) == 0xEE) {
      if (dump)
        printf("XOR A, %020X\n", read_memory8(cpu, cpu->pc++));

      cpu_xor(cpu, &cpu->registers[a], mem_ptr8(cpu, cpu->pc++));

      cpu->pc += 2;
    }

    // RST 28h
    else if (read_memory8(cpu, cpu->pc) == 0xEF) {
      if (dump)
        printf("RST 28h\n");

      cpu_rst(cpu, 0x28);
      cpu->pc++;
    }

    // LD A, (FF00+u8)
    else if (read_memory8(cpu, cpu->pc) == 0xF0) {
      if (dump)
        printf("LD A, (FF00+%02X)\n", read_memory8(cpu, cpu->pc + 1));

      cpu_ld_reg8(cpu, a, 0xFF00 + cpu->pc + 1);
      cpu->pc += 2;
    }

    // POP AF
    else if (read_memory8(cpu, cpu->pc) == 0xF1) {
      if (dump)
        printf("POP AF\n");

      cpu_pop(cpu, &cpu->registers[a], &cpu->registers[f]);
      cpu->pc++;
    }

    // LD A, (FF00+C)
    else if (read_memory8(cpu, cpu->pc) == 0xF2) {
      if (dump)
        printf("LD A, (FF00+C)\n");

      cpu_ld_reg8(cpu, a, 0xFF00 + cpu->registers[c]);
      cpu->pc += 2;
    }

    // DI
    else if (read_memory8(cpu, cpu->pc) == 0xF3) {
      if (dump)
        printf("DI\n");

      cpu->ime_r = false;
      cpu->pc++;
    }

    // Invalid
    else if (read_memory8(cpu, cpu->pc) == 0xF4) {
      if (dump)
        printf("Invalid instruction: 0x%02X\n", read_memory8(cpu, cpu->pc));

      cpu->halt = true;
    }

    // PUSH AF
    else if (read_memory8(cpu, cpu->pc) == 0xF5) {
      if (dump)
        printf("PUSH HL\n");

      cpu_push(cpu, &cpu->registers[a], &cpu->registers[f]);
      cpu->pc++;
    }

    // OR A, u8
    else if (read_memory8(cpu, cpu->pc) == 0xF6) {
      if (dump)
        printf("OR A, %020X\n", read_memory8(cpu, cpu->pc++));

      cpu_or(cpu, &cpu->registers[a], mem_ptr8(cpu, cpu->pc++));
      cpu->pc += 2;
    }

    // RST 30h
    else if (read_memory8(cpu, cpu->pc) == 0xF7) {
      if (dump)
        printf("RST 30h\n");

      cpu_rst(cpu, 0x30);
      cpu->pc++;
    }

    // LD HL,SP+i8
    else if (read_memory8(cpu, cpu->pc) == 0xF8) {
      if (dump)
        printf("LD HL, SP + %020X", read_memory8(cpu, cpu->pc + 1));

      cpu_ld_reg16(cpu, hl, cpu->sp + cpu->pc + 1);

      cpu->pc += 2;
    }

    // LD SP, HL
    else if (read_memory8(cpu, cpu->pc) == 0xF9) {
      if (dump)
        printf("LD SP, HL\n");

      cpu_ld_reg16(cpu, sp, uint8to16(cpu->registers[h], cpu->registers[l]));

      cpu->pc++;
    }

    // LD A, (u16)
    else if (read_memory8(cpu, cpu->pc) == 0xFA) {
      if (dump)
        printf("LD A, %040X\n", read_memory16(cpu, cpu->pc + 1));

      cpu_ld_reg8(cpu, a, (uint8_t)read_memory16(cpu, cpu->pc + 1));
      cpu->pc += 2;
    }

    // EI
    else if (read_memory8(cpu, cpu->pc) == 0xFB) {
      if (dump)
        printf("EI\n");

      cpu->ime_r = true;
      cpu->pc++;
    }

    // Invalid
    else if (read_memory8(cpu, cpu->pc) == 0xFC) {
      if (dump)
        printf("Invalid instruction: 0x%02X\n", read_memory8(cpu, cpu->pc));

      cpu->halt = true;
    }

    // Invalid
    else if (read_memory8(cpu, cpu->pc) == 0xFD) {
      if (dump)
        printf("Invalid instruction: 0x%02X\n", read_memory8(cpu, cpu->pc));

      cpu->halt = true;
    }

    // CP A, u8
    else if (read_memory8(cpu, cpu->pc) == 0xFE) {
      if (dump)
        printf("CP A, %020X\n", cpu->pc + 1);

      cpu_cp8(cpu, &cpu->registers[a], mem_ptr8(cpu, cpu->pc + 1));

      cpu->pc++;
    }

    // RST 38h
    else if (read_memory8(cpu, cpu->pc) == 0xFF) {
      if (dump)
        printf("RST 38h\n");

      cpu_rst(cpu, 0x38);

      cpu->pc++;
    }

    else {
      if (dump)
        printf("Invalid opcode: 0x%02X\n", read_memory8(cpu, cpu->pc));

      cpu->halt = true;
    }

    if (read_memory8(cpu, 0xFF02) == 0x81) {
      char c = read_memory8(cpu, 0xFF01);
      printf("%c", c);
      write_memory8(cpu, 0x0, 0xFF02);
    }

    if (print_regs) {
      printf("cpu_execute(): --End--\n\n");
      sleep(1);
    }

  } 

  if (cpu->halt) {
    if (!stop_on_halt)
      cpu_halt_loop(cpu, dump);
    else
     return;
  }
}

/* cpu_cb(): Handle CB prefixed instructions
 */
void cpu_cb(struct CPU *cpu, bool dump) {
  if (read_memory8(cpu, cpu->pc) == 0x00) {
    if (dump)
      printf("RLC B\n");

    cpu_rlc_r(cpu, b);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x01) {
    if (dump)
      printf("RLC C\n");

    cpu_rlc_r(cpu, c);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x02) {
    if (dump)
      printf("RLC D\n");

    cpu_rlc_r(cpu, d);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x03) {
    if (dump)
      printf("RLC E\n");

    cpu_rlc_r(cpu, e);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x04) {
    if (dump)
      printf("RLC H\n");

    cpu_rlc_r(cpu, h);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x05) {
    if (dump)
      printf("RLC L\n");

    cpu_rlc_r(cpu, l);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x06) {
    if (dump)
      printf("RLC (HL)\n");
    cpu_rlc_mem8(cpu, ( cpu_read_reg16(cpu, hl))); 
  }

  else if (read_memory8(cpu, cpu->pc) == 0x07) {
    if (dump)
      printf("RLC A\n");

    cpu_rlc_r(cpu, a);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x08) {
    if (dump)
      printf("RRC B\n");

    cpu_rrc_r(cpu, b);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x09) {
    if (dump)
      printf("RRC C\n");

    cpu_rrc_r(cpu, c);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x0A) {
    if (dump)
      printf("RRC D\n");

    cpu_rrc_r(cpu, d);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x0B) {
    if (dump)
      printf("RRC E\n");

    cpu_rrc_r(cpu, e);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x0C) {
    if (dump)
      printf("RRC H\n");

    cpu_rrc_r(cpu, h);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x0D) {
    if (dump)
      printf("RRC L\n");

    cpu_rrc_r(cpu, l);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x0E) {
    if (dump)
      printf("RRC (HL)\n");
    cpu_rrc_mem8(cpu, ( cpu_read_reg16(cpu, hl))); 
  }

  else if (read_memory8(cpu, cpu->pc) == 0x0F) {
    if (dump)
      printf("RRC A\n");

    cpu_rrc_r(cpu, a);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x10) {
    if (dump)
      printf("RL B\n");

    cpu_rl_r(cpu, b);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x11) {
    if (dump)
      printf("RL C\n");

    cpu_rl_r(cpu, c);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x12) {
    if (dump)
      printf("RL D\n");

    cpu_rl_r(cpu, d);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x13) {
    if (dump)
      printf("RL E\n");

    cpu_rl_r(cpu, e);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x14) {
    if (dump)
      printf("RL H\n");

    cpu_rl_r(cpu, h);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x15) {
    if (dump)
      printf("RL L\n");

    cpu_rl_r(cpu, l);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x16) {
    if (dump)
      printf("RL (HL)\n");
    cpu_rl_mem8(cpu, ( cpu_read_reg16(cpu, hl))); 
  }

  else if (read_memory8(cpu, cpu->pc) == 0x17) {
    if (dump)
      printf("RL A\n");

    cpu_rl_r(cpu, a);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x18) {
    if (dump)
      printf("RR B\n");

    cpu_rr_r(cpu, b);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x19) {
    if (dump)
      printf("RR C\n");

    cpu_rr_r(cpu, c);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x1A) {
    if (dump)
      printf("RR D\n");

    cpu_rr_r(cpu, d);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x1B) {
    if (dump)
      printf("RR E\n");

    cpu_rr_r(cpu, e);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x1C) {
    if (dump)
      printf("RR H\n");

    cpu_rr_r(cpu, h);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x1D) {
    if (dump)
      printf("RR L\n");

    cpu_rr_r(cpu, l);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x1E) {
    if (dump)
      printf("RR (HL)\n");
    cpu_rr_mem8(cpu, ( cpu_read_reg16(cpu, hl))); 
  }

  else if (read_memory8(cpu, cpu->pc) == 0x1F) {
    if (dump)
      printf("RR A\n");

    cpu_rr_r(cpu, a);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x20) {
    if (dump)
      printf("SLA B\n");

    cpu_sla_r(cpu, b);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x21) {
    if (dump)
      printf("SLA C\n");

    cpu_sla_r(cpu, c);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x22) {
    if (dump)
      printf("SLA D\n");

    cpu_sla_r(cpu, d);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x23) {
    if (dump)
      printf("SLA E\n");

    cpu_sla_r(cpu, e);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x24) {
    if (dump)
      printf("SLA H\n");

    cpu_sla_r(cpu, h);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x25) {
    if (dump)
      printf("SLA L\n");

    cpu_sla_r(cpu, l);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x26) {
    if (dump)
      printf("SLA (HL)\n");
    cpu_sla_mem8(cpu, ( cpu_read_reg16(cpu, hl))); 
  }

  else if (read_memory8(cpu, cpu->pc) == 0x27) {
    if (dump)
      printf("SLA A\n");

    cpu_sla_r(cpu, a);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x28) {
    if (dump)
      printf("SRA B\n");

    cpu_sra_r(cpu, b);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x29) {
    if (dump)
      printf("SRA C\n");

    cpu_sra_r(cpu, c);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x2A) {
    if (dump)
      printf("SRA D\n");

    cpu_sra_r(cpu, d);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x2B) {
    if (dump)
      printf("SRA E\n");

    cpu_sra_r(cpu, e);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x2C) {
    if (dump)
      printf("SRA H\n");

    cpu_sra_r(cpu, h);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x2D) {
    if (dump)
      printf("SRA L\n");

    cpu_sra_r(cpu, l);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x2E) {
    if (dump)
      printf("SRA (HL)\n");
    cpu_sra_mem8(cpu, ( cpu_read_reg16(cpu, hl))); 
  }

  else if (read_memory8(cpu, cpu->pc) == 0x2F) {
    if (dump)
      printf("SRA A\n");

    cpu_sra_r(cpu, a);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x30) {
    if (dump)
      printf("SWAP B\n");

    cpu_swap_r(cpu, b);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x31) {
    if (dump)
      printf("SWAP C\n");

    cpu_swap_r(cpu, c);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x32) {
    if (dump)
      printf("SWAP D\n");

    cpu_swap_r(cpu, d);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x33) {
    if (dump)
      printf("SWAP E\n");

    cpu_swap_r(cpu, e);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x34) {
    if (dump)
      printf("SWAP H\n");

    cpu_swap_r(cpu, h);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x35) {
    if (dump)
      printf("SWAP L\n");

    cpu_swap_r(cpu, l);
  }


  else if (read_memory8(cpu, cpu->pc) == 0x36) {
    if (dump)
      printf("SWAP (HL)");

    cpu_swap_m(cpu, (cpu_read_reg16(cpu, hl)));
  }

  else if (read_memory8(cpu, cpu->pc) == 0x37) {
    if (dump)
      printf("SWAP A\n");

    cpu_swap_r(cpu, a);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x38) {
    if (dump)
      printf("SRL B\n");

    cpu_srl_r(cpu, b);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x39) {
    if (dump)
      printf("SRL C\n");

    cpu_srl_r(cpu, c);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x3A) {
    if (dump)
      printf("SRL D\n");

    cpu_srl_r(cpu, d);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x3B) {
    if (dump)
      printf("SRL E\n");

    cpu_srl_r(cpu, e);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x3C) {
    if (dump)
      printf("SRL H\n");

    cpu_srl_r(cpu, h);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x3D) {
    if (dump)
      printf("SRL L\n");

    cpu_srl_r(cpu, l);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x3E) {
    if (dump)
      printf("SRL (HL)\n");

    cpu_srl_mem8(cpu, cpu_read_reg16(cpu, hl));
  }

  else if (read_memory8(cpu, cpu->pc) == 0x3F) {
    if (dump)
      printf("SRL A\n");

    cpu_srl_r(cpu, a);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x40) {
    if (dump)
      printf("BIT 0,B\n");

    cpu_bit_r(cpu, b, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x41) {
    if (dump)
      printf("BIT 0,C\n");

    cpu_bit_r(cpu, c, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x42) {
    if (dump)
      printf("BIT 0,D\n");

    cpu_bit_r(cpu, d, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x43) {
    if (dump)
      printf("BIT 0,E\n");

    cpu_bit_r(cpu, e, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x44) {
    if (dump)
      printf("BIT 0,H\n");

    cpu_bit_r(cpu, h, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x45) {
    if (dump)
      printf("BIT 0,L\n");

    cpu_bit_r(cpu, l, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x46) {
    if (dump)
      printf("BIT 0,(HL)\n");

    cpu_bit_mem8(cpu, cpu_read_reg16(cpu, hl), 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x47) {
    if (dump)
      printf("BIT 0,A\n");

    cpu_bit_r(cpu, a, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x48) {
    if (dump)
      printf("BIT 1,B\n");

    cpu_bit_r(cpu, b, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x49) {
    if (dump)
      printf("BIT 1,C\n");

    cpu_bit_r(cpu,c, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x4A) {
    if (dump)
      printf("BIT 1,D\n");

    cpu_bit_r(cpu, d, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x4B) {
    if (dump)
      printf("BIT 1,E\n");

    cpu_bit_r(cpu, e, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x4C) {
    if (dump)
      printf("BIT 1,H\n");

    cpu_bit_r(cpu, h, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x4D) {
    if (dump)
      printf("BIT 1,L\n");

    cpu_bit_r(cpu, l, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x4E) {
    if (dump)
      printf("BIT 1,(HL)\n");

    cpu_bit_mem8(cpu, cpu_read_reg16(cpu, hl), 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x4F) {
    if (dump)
      printf("BIT 1,A\n");

    cpu_bit_r(cpu, a, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x50) {
    if (dump)
      printf("BIT 2,B\n");

    cpu_bit_r(cpu, b, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x51) {
    if (dump)
      printf("BIT 2,C\n");

    cpu_bit_r(cpu, c, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x52) {
    if (dump)
      printf("BIT 2,D\n");

    cpu_bit_r(cpu, d, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x53) {
    if (dump)
      printf("BIT 2,E\n");

    cpu_bit_r(cpu, e, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x54) {
    if (dump)
      printf("BIT 2,H\n");

    cpu_bit_r(cpu, h, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x55) {
    if (dump)
      printf("BIT 2,L\n");

    cpu_bit_r(cpu, l, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x56) {
    if (dump)
      printf("BIT 2,(HL)\n");

    cpu_bit_mem8(cpu, cpu_read_reg16(cpu, hl), 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x57) {
    if (dump)
      printf("BIT 2,A\n");

    cpu_bit_r(cpu, a, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x58) {
    if (dump)
      printf("BIT 3,B\n");

    cpu_bit_r(cpu, b, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x59) {
    if (dump)
      printf("BIT 3,C\n");

    cpu_bit_r(cpu, c, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x5A) {
    if (dump)
      printf("BIT 3,D\n");

    cpu_bit_r(cpu, d, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x5B) {
    if (dump)
      printf("BIT 3,E\n");

    cpu_bit_r(cpu, e, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x5C) {
    if (dump)
      printf("BIT 3,H\n");

    cpu_bit_r(cpu, h, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x5D) {
    if (dump)
      printf("BIT 3,L\n");

    cpu_bit_r(cpu, l, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x5E) {
    if (dump)
      printf("BIT 3,(HL)\n");

    cpu_bit_mem8(cpu, cpu_read_reg16(cpu, hl), 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x5F) {
    if (dump)
      printf("BIT 3,A\n");

    cpu_bit_r(cpu, a, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x60) {
    if (dump)
      printf("BIT 4,B\n");

    cpu_bit_r(cpu, b, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x61) {
    if (dump)
      printf("BIT 4,C\n");

    cpu_bit_r(cpu, c, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x62) {
    if (dump)
      printf("BIT 4,D\n");

    cpu_bit_r(cpu, d, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x63) {
    if (dump)
      printf("BIT 4,E\n");

    cpu_bit_r(cpu, e, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x64) {
    if (dump)
      printf("BIT 4,H\n");

    cpu_bit_r(cpu, h, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x65) {
    if (dump)
      printf("BIT 4,L\n");

    cpu_bit_r(cpu, l, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x66) {
    if (dump)
      printf("BIT 4,(HL)\n");

    cpu_bit_mem8(cpu, cpu_read_reg16(cpu, hl), 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x67) {
    if (dump)
      printf("BIT 4,A\n");

    cpu_bit_r(cpu, a, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x68) {
    if (dump)
      printf("BIT 5,B\n");

    cpu_bit_r(cpu, b, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x69) {
    if (dump)
      printf("BIT 5,C\n");

    cpu_bit_r(cpu, c, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x6A) {
    if (dump)
      printf("BIT 5,D\n");

    cpu_bit_r(cpu, d, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x6B) {
    if (dump)
      printf("BIT 5,E\n");

    cpu_bit_r(cpu, e, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x6C) {
    if (dump)
      printf("BIT 5,L\n");

    cpu_bit_r(cpu, l, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x6D) {
    if (dump)
      printf("BIT 5,H\n");

    cpu_bit_r(cpu, h, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x6E) {
    if (dump)
      printf("BIT 5,(HL)\n");

    cpu_bit_mem8(cpu, cpu_read_reg16(cpu, hl), 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x6F) {
    if (dump)
      printf("BIT 5,A\n");

    cpu_bit_r(cpu, a, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x70) {
    if (dump)
      printf("BIT 6,B\n");

    cpu_bit_r(cpu, b, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x71) {
    if (dump)
      printf("BIT 6,C\n");

    cpu_bit_r(cpu, c, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x72) {
    if (dump)
      printf("BIT 6,D\n");

    cpu_bit_r(cpu, d, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x73) {
    if (dump)
      printf("BIT 6,E\n");

    cpu_bit_r(cpu, e, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x74) {
    if (dump)
      printf("BIT 6,H\n");

    cpu_bit_r(cpu, h, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x75) {
    if (dump)
      printf("BIT 6,L\n");

    cpu_bit_r(cpu, l, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x76) {
    if (dump)
      printf("BIT 6,(HL)\n");

    cpu_bit_mem8(cpu, cpu_read_reg16(cpu, hl), 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x77) {
    if (dump)
      printf("BIT 6,A\n");

    cpu_bit_r(cpu, a, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x78) {
    if (dump)
      printf("BIT 7,B\n");

    cpu_bit_r(cpu, b, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x79) {
    if (dump)
      printf("BIT 7,C\n");

    cpu_bit_r(cpu, c, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x7A) {
    if (dump)
      printf("BIT 7,D\n");

    cpu_bit_r(cpu, d, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x7B) {
    if (dump)
      printf("BIT 7,E\n");

    cpu_bit_r(cpu, e, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x7C) {
    if (dump)
      printf("BIT 7,H\n");

    cpu_bit_r(cpu, h, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x7D) {
    if (dump)
      printf("BIT 7,L\n");

    cpu_bit_r(cpu, l, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x7E) {
    if (dump)
      printf("BIT 7,(HL)\n");

    cpu_bit_mem8(cpu, cpu_read_reg16(cpu, hl), 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x7F) {
    if (dump)
      printf("BIT 7,A\n");

    cpu_bit_r(cpu, a, 7);
  }
  
  else if (read_memory8(cpu, cpu->pc) == 0x80) {
    if (dump)
      printf("RES 0,B\n");

    cpu_res_r(cpu, b, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x81) {
    if (dump)
      printf("RES 0,C\n");

    cpu_res_r(cpu, c, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x82) {
    if (dump)
      printf("RES 0,D\n");

    cpu_res_r(cpu, d, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x83) {
    if (dump)
      printf("RES 0,E\n");

    cpu_res_r(cpu, e, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x84) {
    if (dump)
      printf("RES 0,H\n");

    cpu_res_r(cpu, h, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x85) {
    if (dump)
      printf("RES 0,L\n");

    cpu_res_r(cpu, l, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x86) {
    if (dump)
      printf("RES 0,(HL)\n");

    cpu_res_mem8(cpu, cpu_read_reg16(cpu, hl), 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x87) {
    if (dump)
      printf("RES 0,A\n");

    cpu_res_r(cpu, a, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x88) {
    if (dump)
      printf("RES 1,B\n");

    cpu_res_r(cpu, b, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x89) {
    if (dump)
      printf("RES 1,C\n");

    cpu_res_r(cpu, c, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x8A) {
    if (dump)
      printf("RES 1,D\n");

    cpu_res_r(cpu, d, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x8B) {
    if (dump)
      printf("RES 1,E\n");

    cpu_res_r(cpu, e, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x8C) {
    if (dump)
      printf("RES 1,H\n");

    cpu_res_r(cpu, h, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x8D) {
    if (dump)
      printf("RES 1,L\n");

    cpu_res_r(cpu, l, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x8E) {
    if (dump)
      printf("RES 1,(HL)\n");

    cpu_res_mem8(cpu, cpu_read_reg16(cpu, hl), 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x8F) {
    if (dump)
      printf("RES 1,A\n");

    cpu_res_r(cpu, a, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x90) {
    if (dump)
      printf("RES 2,B\n");

    cpu_res_r(cpu, b, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x91) {
    if (dump)
      printf("RES 2,C\n");

    cpu_res_r(cpu, c, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x92) {
    if (dump)
      printf("RES 2,D\n");

    cpu_res_r(cpu, d, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x93) {
    if (dump)
      printf("RES 2,E\n");

    cpu_res_r(cpu, e, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x94) {
    if (dump)
      printf("RES 2,H\n");

    cpu_res_r(cpu, h, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x95) {
    if (dump)
      printf("RES 2,L\n");

    cpu_res_r(cpu, l, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x96) {
    if (dump)
      printf("RES 2,(HL)\n");

    cpu_res_mem8(cpu, cpu_read_reg16(cpu, hl), 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x97) {
    if (dump)
      printf("RES 2,A\n");

    cpu_res_r(cpu, a, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x98) {
    if (dump)
      printf("RES 3,B\n");

    cpu_res_r(cpu, b, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x99) {
    if (dump)
      printf("RES 3,C\n");

    cpu_res_r(cpu, c, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x9A) {
    if (dump)
      printf("RES 3,D\n");

    cpu_res_r(cpu, d, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x9B) {
    if (dump)
      printf("RES 3,E\n");

    cpu_res_r(cpu, e, 3);
  }


  else if (read_memory8(cpu, cpu->pc) == 0x9C) {
    if (dump)
      printf("RES 3,H\n");

    cpu_res_r(cpu, h, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x9D) {
    if (dump)
      printf("RES 3,L\n");

    cpu_res_r(cpu, l, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x9E) {
    if (dump)
      printf("RES 3,(HL)\n");

    cpu_res_mem8(cpu, cpu_read_reg16(cpu, hl), 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x9F) {
    if (dump)
      printf("RES 3,A\n");

    cpu_res_r(cpu, a, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xA0) {
    if (dump)
      printf("RES 4,B\n");

    cpu_res_r(cpu, b, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xA1) {
    if (dump)
      printf("RES 4,C\n");

    cpu_res_r(cpu, c, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xA2) {
    if (dump)
      printf("RES 4,D\n");

    cpu_res_r(cpu, d, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x43) {
    if (dump)
      printf("RES 4,E\n");

    cpu_res_r(cpu, e, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x44) {
    if (dump)
      printf("RES 4,H\n");

    cpu_res_r(cpu, h, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x45) {
    if (dump)
      printf("RES 4,L\n");

    cpu_res_r(cpu, l, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x46) {
    if (dump)
      printf("RES 4,(HL)\n");

    cpu_res_mem8(cpu, cpu_read_reg16(cpu, hl), 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x47) {
    if (dump)
      printf("RES 4,A\n");

    cpu_res_r(cpu, a, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x48) {
    if (dump)
      printf("RES 5,B\n");

    cpu_res_r(cpu, b, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x49) {
    if (dump)
      printf("RES 5,C\n");

    cpu_res_r(cpu, c, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x4A) {
    if (dump)
      printf("RES 5,D\n");

    cpu_res_r(cpu, d, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x4B) {
    if (dump)
      printf("RES 5,E\n");

    cpu_res_r(cpu, e, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x4C) {
    if (dump)
      printf("RES 5,H\n");

    cpu_res_r(cpu, h, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x4D) {
    if (dump)
      printf("RES 5,L\n");

    cpu_res_r(cpu, l, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x4E) {
    if (dump)
      printf("RES 5,(HL)\n");

    cpu_res_mem8(cpu, cpu_read_reg16(cpu, hl), 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0x4F) {
    if (dump)
      printf("RES 5,A\n");

    cpu_res_r(cpu, a, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xB0) {
    if (dump)
      printf("RES 6,B\n");

    cpu_res_r(cpu, b, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xB1) {
    if (dump)
      printf("RES 6,C\n");

    cpu_res_r(cpu, c, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xB2) {
    if (dump)
      printf("RES 6,D\n");

    cpu_res_r(cpu, d, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xB3) {
    if (dump)
      printf("RES 6,E\n");

    cpu_res_r(cpu, e, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xB4) {
    if (dump)
      printf("RES 6,H\n");

    cpu_res_r(cpu, h, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xB5) {
    if (dump)
      printf("RES 6,L\n");

    cpu_res_r(cpu, l, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xB6) {
    if (dump)
      printf("RES 6,(HL)\n");

    cpu_res_mem8(cpu, cpu_read_reg16(cpu, hl), 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xB7) {
    if (dump)
      printf("RES 6,A\n");

    cpu_res_r(cpu, a, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xB8) {
    if (dump)
      printf("RES 7,B\n");

    cpu_res_r(cpu, b, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xB9) {
    if (dump)
      printf("RES 7,C\n");

    cpu_res_r(cpu, c, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xBA) {
    if (dump)
      printf("RES 7,D\n");

    cpu_res_r(cpu, d, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xBB) {
    if (dump)
      printf("RES 7,E\n");

    cpu_res_r(cpu, e, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xBC) {
    if (dump)
      printf("RES 7,H\n");

    cpu_res_r(cpu, h, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xBD) {
    if (dump)
      printf("RES 7,L\n");

    cpu_res_r(cpu, l, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xBE) {
    if (dump)
      printf("RES 7,(HL)\n");

    cpu_res_mem8(cpu, cpu_read_reg16(cpu, hl), 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xBF) {
    if (dump)
      printf("RES 7,A\n");

    cpu_res_r(cpu, a, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xC0) {
    if (dump)
      printf("SET 0,B\n");

    cpu_set_r(cpu, b, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xC1) {
    if (dump)
      printf("SET 0,C\n");

    cpu_set_r(cpu, c, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xC2) {
    if (dump)
      printf("SET 0,D\n");

    cpu_set_r(cpu, d, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xC3) {
    if (dump)
      printf("SET 0,E\n");

    cpu_set_r(cpu, e, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xC4) {
    if (dump)
      printf("SET 0,H\n");

    cpu_set_r(cpu, h, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xC5) {
    if (dump)
      printf("SET 0,L\n");

    cpu_set_r(cpu, l, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xC6) {
    if (dump)
      printf("SET 0,(HL)\n");

    cpu_set_mem8(cpu, cpu_read_reg16(cpu, hl), 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xC7) {
    if (dump)
      printf("SET 0,A\n");

    cpu_set_r(cpu, a, 0);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xC8) {
    if (dump)
      printf("SET 1,B\n");

    cpu_set_r(cpu, b, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xC9) {
    if (dump)
      printf("SET 1,C\n");

    cpu_set_r(cpu, c, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xCA) {
    if (dump)
      printf("SET 1,D\n");

    cpu_set_r(cpu, d, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xCB) {
    if (dump)
      printf("SET 1,E\n");

    cpu_set_r(cpu, e, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xCC) {
    if (dump)
      printf("SET 1,H\n");

    cpu_set_r(cpu, h, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xCD) {
    if (dump)
      printf("SET 1,L\n");

    cpu_set_r(cpu, l, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xCE) {
    if (dump)
      printf("SET 1,(HL)\n");

    cpu_set_mem8(cpu, cpu_read_reg16(cpu, hl), 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xCF) {
    if (dump)
      printf("SET 1,A\n");

    cpu_set_r(cpu, a, 1);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xD0) {
    if (dump)
      printf("SET 2,B\n");

    cpu_set_r(cpu, b, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xD1) {
    if (dump)
      printf("SET 2,C\n");

    cpu_set_r(cpu, c, 2);
  }


  else if (read_memory8(cpu, cpu->pc) == 0xD2) {
    if (dump)
      printf("SET 2,D\n");

    cpu_set_r(cpu, d, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xD3) {
    if (dump)
      printf("SET 2,E\n");

    cpu_set_r(cpu, e, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xD4) {
    if (dump)
      printf("SET 2,H\n");

    cpu_set_r(cpu, h, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xD5) {
    if (dump)
      printf("SET 2,L\n");

    cpu_set_r(cpu, l, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xD6) {
    if (dump)
      printf("SET 2,(HL)\n");

    cpu_set_mem8(cpu, cpu_read_reg16(cpu, hl), 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xD7) {
    if (dump)
      printf("SET 2,A\n");

    cpu_set_r(cpu, a, 2);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xD8) {
    if (dump)
      printf("SET 3,B\n");

    cpu_set_r(cpu, b, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xD9) {
    if (dump)
      printf("SET 3,C\n");

    cpu_set_r(cpu, c, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xDA) {
    if (dump)
      printf("SET 3,D\n");

    cpu_set_r(cpu, d, 3);
  }
  
  else if (read_memory8(cpu, cpu->pc) == 0xDB) {
    if (dump)
      printf("SET 3,E\n");

    cpu_set_r(cpu, e, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xDC) {
    if (dump)
      printf("SET 3,H\n");

    cpu_set_r(cpu, h, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xDD) {
    if (dump)
      printf("SET 3,L\n");

    cpu_set_r(cpu, l, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xDE) {
    if (dump)
      printf("SET 3,(HL)\n");

    cpu_set_mem8(cpu, cpu_read_reg16(cpu, hl), 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xDF) {
    if (dump)
      printf("SET 3,A\n");

    cpu_set_r(cpu, a, 3);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xE0) {
    if (dump)
      printf("SET 4,B\n");

    cpu_set_r(cpu, b, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xE1) {
    if (dump)
      printf("SET 4,C\n");

    cpu_set_r(cpu, c, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xE2) {
    if (dump)
      printf("SET 4,D\n");

    cpu_set_r(cpu, d, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xE3) {
    if (dump)
      printf("SET 4,E\n");

    cpu_set_r(cpu, e, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xE4) {
    if (dump)
      printf("SET 4,H\n");

    cpu_set_r(cpu, h, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xE5) {
    if (dump)
      printf("SET 4,L\n");

    cpu_set_r(cpu, l, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xE6) {
    if (dump)
      printf("SET 4,(HL)\n");

    cpu_set_mem8(cpu, cpu_read_reg16(cpu, hl), 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xE7) {
    if (dump)
      printf("SET 4,A\n");

    cpu_set_r(cpu, a, 4);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xE8) {
    if (dump)
      printf("SET 5,B\n");

    cpu_set_r(cpu, b, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xE9) {
    if (dump)
      printf("SET 5,C\n");

    cpu_set_r(cpu, c, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xEA) {
    if (dump)
      printf("SET 5,D\n");

    cpu_set_r(cpu, d, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xEB) {
    if (dump)
      printf("SET 5,E\n");

    cpu_set_r(cpu, e, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xEC) {
    if (dump)
      printf("SET 5,H\n");

    cpu_set_r(cpu, h, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xED) {
    if (dump)
      printf("SET 5,L\n");

    cpu_set_r(cpu, l, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xEE) {
    if (dump)
      printf("SET 5,(HL)\n");

    cpu_set_mem8(cpu, cpu_read_reg16(cpu, hl), 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xEF) {
    if (dump)
      printf("SET 5,A\n");

    cpu_set_r(cpu, a, 5);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xF0) {
    if (dump)
      printf("SET 6,B\n");

    cpu_set_r(cpu, b, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xF1) {
    if (dump)
      printf("SET 6,C\n");

    cpu_set_r(cpu, c, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xF2) {
    if (dump)
      printf("SET 6,D\n");

    cpu_set_r(cpu, d, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xF3) {
    if (dump)
      printf("SET 6,E\n");

    cpu_set_r(cpu, e, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xF4) {
    if (dump)
      printf("SET 6,H\n");

    cpu_set_r(cpu, h, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xF5) {
    if (dump)
      printf("SET 6,L\n");

    cpu_set_r(cpu, l, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xF6) {
    if (dump)
      printf("SET 6,(HL)\n");

    cpu_set_mem8(cpu, cpu_read_reg16(cpu, hl), 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xF7) {
    if (dump)
      printf("SET 6,A\n");

    cpu_set_r(cpu, a, 6);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xF8) {
    if (dump)
      printf("SET 7,B\n");

    cpu_set_r(cpu, b, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xF9) {
    if (dump)
      printf("SET 7,C\n");

    cpu_set_r(cpu, c, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xFA) {
    if (dump)
      printf("SET 7,D\n");

    cpu_set_r(cpu, d, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xFB) {
    if (dump)
      printf("SET 7,E\n");

    cpu_set_r(cpu, e, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xFC) {
    if (dump)
      printf("SET 7,H\n");

    cpu_set_r(cpu, h, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xFD) {
    if (dump)
      printf("SET 7,L\n");

    cpu_set_r(cpu, l, 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xFE) {
    if (dump)
      printf("SET 7,(HL)\n");

    cpu_set_mem8(cpu, cpu_read_reg16(cpu, hl), 7);
  }

  else if (read_memory8(cpu, cpu->pc) == 0xFF) {
    if (dump)
      printf("SET 7,A\n");

    cpu_set_r(cpu, a, 7);
  }
}
