#include "cpu.h"
#include "rom.h"

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
  memset(cpu->mem, 0, 0x10000);

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

/* cpu_execute(): What this does is decode each opcode and translate it into an
 * instruction to be cpu_execute by the virtual CPU. Optional dump flag for
 * disassembly upon execution.
 */
void cpu_execute(struct CPU *cpu, const struct ROM *rom, bool dump,
                 bool print_regs) {
  while (!cpu->halt) {
    if (print_regs) {
      printf("cpu_execute(): --Begin--\n");
      printf("cpu_execute(): rom->data[cpu->pc]: 0x%02X\n", rom->data[cpu->pc]);
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
    }

    // NOP
    if (rom->data[cpu->pc] == 0x00) {
      if (dump)
        printf("NOP\n");
      cpu->pc++;
    }

    // LD BC, d16
    else if (rom->data[cpu->pc] == 0x01) {
      if (dump)
        printf("LD BC, %02X%02X\n", rom->data[cpu->pc + 1],
               rom->data[cpu->pc + 2]);

      // C and B are reversed due to endianness
      cpu->registers[c] = rom->data[cpu->pc + 1];
      cpu->registers[b] = rom->data[cpu->pc + 2];
      cpu->pc += 3;
    }

    // LD (BC), A
    else if (rom->data[cpu->pc] == 0x02) {
      if (dump)
        printf("LD (BC), A\n");

      cpu->mem[(cpu->registers[c] << 8) | cpu->registers[b]] =
          cpu->registers[a];
      cpu->pc++;
    }

    // INC BC
    else if (rom->data[cpu->pc] == 0x03) {
      if (dump)
        printf("INC BC\n");

      cpu->registers[b]++;
      cpu->registers[c]++;

      cpu->pc++;
    }

    // INC B
    else if (rom->data[cpu->pc] == 0x04) {
      if (dump)
        printf("INC B\n");

      cpu->registers[b]++;
      cpu->pc++;
    }

    // DEC B
    else if (rom->data[cpu->pc] == 0x05) {
      if (dump)
        printf("DEC B\n");

      cpu->registers[b]--;
      cpu->pc++;
    }

    // LD B, d8
    else if (rom->data[cpu->pc] == 0x06) {
      if (dump)
        printf("LD B, %02X\n", rom->data[cpu->pc + 1]);

      cpu->registers[b] = rom->data[cpu->pc + 1];

      cpu->pc += 2;
    }

    // RLCA
    else if (rom->data[cpu->pc] == 0x07) {
      if (dump)
        printf("RLCA\n");

      cpu->registers[a] =
          (cpu->registers[a] << 1) | ((cpu->registers[a] >> 7) & 1);
      cpu->registers[c] = cpu->registers[a] & (1 << 0);

      cpu->pc++;
    }

    // LD (a16), SP
    else if (rom->data[cpu->pc] == 0x08) {
      if (dump)
        printf("LD %02X%02X\n, SP\n", rom->data[cpu->pc + 1],
               rom->data[cpu->pc + 2]);

      cpu->mem[cpu->pc + 2] = (cpu->sp);
      cpu->mem[cpu->pc + 1] = (cpu->sp & 0xFF);

      cpu->pc += 3;
    }

    // ADD HL, BC
    else if (rom->data[cpu->pc] == 0x09) {
      if (dump)
        printf("ADD HL, BC\n");

      cpu->registers[h] += cpu->registers[b];
      cpu->registers[l] += cpu->registers[c];

      cpu->pc++;
    }

    // LD A, (BC)
    else if (rom->data[cpu->pc] == 0x0A) {
      if (dump)
        printf("LOAD A, (BC)\n");

      cpu->registers[a] =
          cpu->mem[(cpu->registers[b] << 8) | cpu->registers[c]];

      cpu->pc++;
    }

    // DEC BC
    else if (rom->data[cpu->pc] == 0x0B) {
      if (dump)
        printf("DEC BC\n");

      cpu->registers[b]--;
      cpu->registers[c]--;

      cpu->pc++;
    }

    // INC C
    else if (rom->data[cpu->pc] == 0x0C) {
      if (dump)
        printf("INC C\n");

      cpu->registers[c]++;

      cpu->pc++;
    }

    // DEC C
    else if (rom->data[cpu->pc] == 0x0D) {
      if (dump)
        printf("DEC C\n");

      cpu->registers[c]--;

      cpu->pc++;
    }

    // LD C, d8
    else if (rom->data[cpu->pc] == 0x0E) {
      if (dump)
        printf("LD C, %02X\n", rom->data[cpu->pc + 1]);

      cpu->registers[c] = rom->data[cpu->pc + 1];

      cpu->pc += 2;
    }

    // RRCA
    else if (rom->data[cpu->pc] == 0x0F) {
      if (dump)
        printf("RRCA\n");

      cpu->registers[a] =
          (cpu->registers[a] >> 1) | ((cpu->registers[a] << 7) & 1);
      cpu->registers[c] = cpu->registers[a] & (1 >> 0);

      cpu->pc++;
    }

    // STOP
    else if (rom->data[cpu->pc] == 0x10) {
      if (dump)
        printf("STOP\n");

      cpu->halt = true;
      cpu->pc++;
    }

    // LD DE,u16
    else if (rom->data[cpu->pc] == 0x11) {
      if (dump)
        printf("LD DE, %02X%02X\n", rom->data[cpu->pc + 1],
               rom->data[cpu->pc + 2]);

      cpu->registers[e] = rom->data[cpu->pc + 1];
      cpu->registers[d] = rom->data[cpu->pc + 2];
      cpu->pc += 3;
    }

    // LD (DE), A
    else if (rom->data[cpu->pc] == 0x12) {
      if (dump)
        printf("LD (DE), A\n");

      cpu->mem[(cpu->registers[d] << 8) | cpu->registers[e]] =
          cpu->registers[a];
      cpu->pc++;
    }

    // INC DE
    else if (rom->data[cpu->pc] == 0x13) {
      if (dump)
        printf("INC DE\n");

      cpu->registers[d]++;
      cpu->registers[e]++;

      cpu->pc++;
    }

    // INC D
    else if (rom->data[cpu->pc] == 0x14) {
      if (dump)
        printf("INC D\n");

      cpu->registers[d]++;
      cpu->pc++;
    }

    // DEC D
    else if (rom->data[cpu->pc] == 0x15) {
      if (dump)
        printf("DEC D\n");

      cpu->registers[d]--;
      cpu->pc++;
    }

    // LD D, d8
    else if (rom->data[cpu->pc] == 0x16) {
      if (dump)
        printf("LD D, %02X\n", rom->data[cpu->pc + 1]);

      cpu->registers[d] = rom->data[cpu->pc + 1];

      cpu->pc += 2;
    }

    // RLA
    else if (rom->data[cpu->pc] == 0x17) {
      if (dump)
        printf("RLA\n");

      cpu->registers[a] =
          (cpu->registers[a] << 1) | ((cpu->registers[a] >> 7) & 1);
      cpu->registers[c] = cpu->registers[a] & (1 << 0);

      cpu->pc++;
    }

    // JR i8
    else if (rom->data[cpu->pc] == 0x18) {
      if (dump)
        printf("JR %02X\n", rom->data[cpu->pc + 1]);

      cpu->pc += 2 + rom->data[cpu->pc + 1];
    }

    // ADD HL, DE
    else if (rom->data[cpu->pc] == 0x19) {
      if (dump)
        printf("ADD HL, DE\n");

      cpu->registers[h] += cpu->registers[d];
      cpu->registers[l] += cpu->registers[e];

      cpu->pc++;
    }

    // LD A, (DE)
    else if (rom->data[cpu->pc] == 0x1A) {
      if (dump)
        printf("LOAD A, (DE)\n");

      cpu->registers[a] =
          cpu->mem[(cpu->registers[d] << 8) | cpu->registers[e]];

      cpu->pc++;
    }

    // DEC DE
    else if (rom->data[cpu->pc] == 0x1B) {
      if (dump)
        printf("DEC DE\n");

      cpu->registers[d]--;
      cpu->registers[e]--;

      cpu->pc++;
    }

    // INC E
    else if (rom->data[cpu->pc] == 0x1C) {
      if (dump)
        printf("INC E\n");

      cpu->registers[e]++;

      cpu->pc++;
    }

    // DEC E
    else if (rom->data[cpu->pc] == 0x1D) {
      if (dump)
        printf("DEC E\n");

      cpu->registers[e]--;

      cpu->pc++;
    }

    // LD E, d8
    else if (rom->data[cpu->pc] == 0x1E) {
      if (dump)
        printf("LD E, %02X\n", rom->data[cpu->pc + 1]);

      cpu->registers[e] = rom->data[cpu->pc + 1];

      cpu->pc += 2;
    }

    // RRA
    else if (rom->data[cpu->pc] == 0x1F) {
      if (dump)
        printf("RRA\n");

      cpu->registers[a] =
          (cpu->registers[a] >> 1) | ((cpu->registers[a] << 7) & 1);
      cpu->registers[c] = cpu->registers[a] & (1 >> 0);

      cpu->pc++;
    }

    // JP NZ, i8
    else if (rom->data[cpu->pc] == 0x20) {
      if (dump)
        printf("JP NZ, %02X\n", rom->data[cpu->pc + 1]);

      if (cpu->registers[f] != (cpu->registers[f] & flag_z))
        cpu->pc += 2 + rom->data[cpu->pc + 1];
      else
        cpu->pc += 2;
    }

    // LD HL, d16
    else if (rom->data[cpu->pc] == 0x21) {
      if (dump)
        printf("LD HL, %02X%02X\n", rom->data[cpu->pc + 1],
               rom->data[cpu->pc + 2]);

      // C and B are reversed due to endianness
      cpu->registers[l] = rom->data[cpu->pc + 1];
      cpu->registers[h] = rom->data[cpu->pc + 2];
      cpu->pc += 3;
    }

    // LD (HL+), A
    else if (rom->data[cpu->pc] == 0x22) {
      if (dump)
        printf("LD (HL+), A\n");

      cpu->mem[(cpu->registers[h] << 8) | cpu->registers[l]] =
          cpu->registers[a];
      cpu->registers[h]++;
      cpu->registers[l]++;
      cpu->pc++;
    }

    // INC HL
    else if (rom->data[cpu->pc] == 0x23) {
      if (dump)
        printf("INC HL\n");

      cpu->registers[h]++;
      cpu->registers[l]++;

      cpu->pc++;
    }

    // INC H
    else if (rom->data[cpu->pc] == 0x24) {
      if (dump)
        printf("INC H\n");

      cpu->registers[h]++;
      cpu->pc++;
    }

    // DEC H
    else if (rom->data[cpu->pc] == 0x25) {
      if (dump)
        printf("DEC H\n");

      cpu->registers[h]--;
      cpu->pc++;
    }

    // LD H, d8
    else if (rom->data[cpu->pc] == 0x26) {
      if (dump)
        printf("LD H, %02X\n", rom->data[cpu->pc + 1]);

      cpu->registers[h] = rom->data[cpu->pc + 1];

      cpu->pc += 2;
    }

    // DAA
    else if (rom->data[cpu->pc] == 0x27) {
      if (dump)
        printf("DAA\n");

      if (cpu->registers[a] > 0x09 || cpu->registers[f] & flag_h) {
        cpu->registers[a] += 0x06;
      } else if (cpu->registers[a] > 0x99 ||
                 cpu->registers[c] == (cpu->registers[f] & flag_c)) {
        cpu->registers[a] += 0x60;
      }

      cpu->pc++;
    }

    // JR Z, i8
    else if (rom->data[cpu->pc] == 0x28) {
      if (dump)
        printf("JP NZ, %02X\n", rom->data[cpu->pc + 1]);

      if (cpu->registers[f] == (cpu->registers[f] & flag_z))
        cpu->pc += 2 + rom->data[cpu->pc + 1];
      else
        cpu->pc += 2;
    }

    // ADD HL, HL
    else if (rom->data[cpu->pc] == 0x29) {
      if (dump)
        printf("ADD HL, DE\n");

      cpu->registers[h] += cpu->registers[h];
      cpu->registers[l] += cpu->registers[l];

      cpu->pc++;
    }

    // LD A, (HL+)
    else if (rom->data[cpu->pc] == 0x2A) {
      if (dump)
        printf("LOAD A, (HL+)\n");

      cpu->registers[a] =
          cpu->mem[(cpu->registers[h] << 8) | cpu->registers[l]]++;

      cpu->pc++;
    }

    // DEC HL
    else if (rom->data[cpu->pc] == 0x2B) {
      if (dump)
        printf("DEC DE\n");

      cpu->registers[h]--;
      cpu->registers[l]--;

      cpu->pc++;
    }

    // INC L
    else if (rom->data[cpu->pc] == 0x2C) {
      if (dump)
        printf("INC L\n");

      cpu->registers[l]++;

      cpu->pc++;
    }

    // DEC L
    else if (rom->data[cpu->pc] == 0x2D) {
      if (dump)
        printf("DEC L\n");

      cpu->registers[l]--;

      cpu->pc++;
    }

    // LD L, d8
    else if (rom->data[cpu->pc] == 0x2E) {
      if (dump)
        printf("LD L, %02X\n", rom->data[cpu->pc + 1]);

      cpu->registers[l] = rom->data[cpu->pc + 1];

      cpu->pc += 2;
    }

    // CPL
    else if (rom->data[cpu->pc] == 0x2F) {
      if (dump)
        printf("CPL\n");

      cpu->registers[a] = ~(cpu->registers[a] & 0xFF);
      cpu->pc++;
    }

    // JR NC, i8
    else if (rom->data[cpu->pc] == 0x30) {
      if (dump)
        printf("JR NZ, %02X\n", rom->data[cpu->pc + 1]);

      if (cpu->registers[f] != (cpu->registers[f] & flag_c))
        cpu->pc += 2 + rom->data[cpu->pc + 1];
      else
        cpu->pc += 2;
    }

    // LD SP, d16
    else if (rom->data[cpu->pc] == 0x31) {
      if (dump)
        printf("LD SP, %02X%02X\n", rom->data[cpu->pc + 1],
               rom->data[cpu->pc + 2]);

      // C and B are reversed due to endianness
      cpu->registers[l] = rom->data[cpu->pc + 1];
      cpu->registers[h] = rom->data[cpu->pc + 2];
      cpu->pc += 3;
    }

    // LD (HL-), A
    else if (rom->data[cpu->pc] == 0x32) {
      if (dump)
        printf("LD (HL+), A\n");

      cpu->mem[(cpu->registers[h] << 8) | cpu->registers[l]] =
          cpu->registers[a];
      cpu->registers[h]--;
      cpu->registers[l]--;
      cpu->pc++;
    }

    // INC SP
    else if (rom->data[cpu->pc] == 0x33) {
      if (dump)
        printf("INC SP\n");

      cpu->sp++;

      cpu->pc++;
    }

    // INC (HL)
    else if (rom->data[cpu->pc] == 0x34) {
      if (dump)
        printf("INC (HL)\n");

      cpu->registers[h]++;
      cpu->registers[l]++;
      cpu->pc++;
    }

    // DEC H
    else if (rom->data[cpu->pc] == 0x35) {
      if (dump)
        printf("DEC H\n");

      cpu->registers[h]--;
      cpu->pc++;
    }

    // ld h, d8
    else if (rom->data[cpu->pc] == 0x36) {
      if (dump)
        printf("ld h, %02x\n", rom->data[cpu->pc + 1]);

      cpu->registers[h] = rom->data[cpu->pc + 1];

      cpu->pc += 2;
    }

    // SCF
    else if (rom->data[cpu->pc] == 0x37) {
      if (dump)
        printf("SCF\n");

      cpu->registers[f] = cpu->registers[f] & flag_c;

      cpu->pc += 1;
    }

    // JR C, i8
    else if (rom->data[cpu->pc] == 0x38) {
      if (dump)
        printf("JP NZ, %02X\n", rom->data[cpu->pc + 1]);

      if (cpu->registers[f] == (cpu->registers[f] & flag_c))
        cpu->pc += 2 + rom->data[cpu->pc + 1];
      else
        cpu->pc += 2;
    }

    // ADD HL, SP
    else if (rom->data[cpu->pc] == 0x39) {
      if (dump)
        printf("ADD HL, SP\n");

      cpu->registers[h] += (cpu->sp >> 8);
      cpu->registers[l] += (cpu->sp << 8);

      cpu->pc++;
    }

    // LD A, (HL+)
    else if (rom->data[cpu->pc] == 0x3A) {
      if (dump)
        printf("LOAD A, (HL-)\n");

      cpu->registers[a] =
          cpu->mem[(cpu->registers[h] << 8) | cpu->registers[l]]--;

      cpu->pc++;
    }

    // DEC SP
    else if (rom->data[cpu->pc] == 0x3B) {
      if (dump)
        printf("DEC SP\n");

      cpu->sp--;

      cpu->pc++;
    }

    // INC A
    else if (rom->data[cpu->pc] == 0x3C) {
      if (dump)
        printf("INC A\n");

      cpu->registers[a]++;

      cpu->pc++;
    }

    // DEC A
    else if (rom->data[cpu->pc] == 0x3D) {
      if (dump)
        printf("DEC A\n");

      cpu->registers[a]--;

      cpu->pc++;
    }

    // LD A, d8
    else if (rom->data[cpu->pc] == 0x3E) {
      if (dump)
        printf("LD A, %02X\n", rom->data[cpu->pc + 1]);

      cpu->registers[a] = rom->data[cpu->pc + 1];

      cpu->pc += 2;
    }

    // SCF
    else if (rom->data[cpu->pc] == 0x3F) {
      if (dump)
        printf("SCF\n");

      if (cpu->registers[f] & flag_c)
        cpu->registers[f] = cpu->registers[f] & (~flag_c);
      else if (cpu->registers[f] & (~flag_c))
        cpu->registers[f] = cpu->registers[f] & flag_c;
      cpu->pc++;
    }

    else {
      if (dump)
        printf("Invalid opcode: 0x%02X\n", rom->data[cpu->pc]);

      cpu->halt = true;
    }

    if (print_regs) {
      printf("cpu_execute(): --End--\n\n");
      sleep(3);
    }
  }
}
