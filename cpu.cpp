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

      cpu->registers[c] = rom->data[cpu->pc + 1];
      cpu->registers[b] = rom->data[cpu->pc + 2];
      cpu->pc += 3;
    }

    // LD (BC), A
    else if (rom->data[cpu->pc] == 0x02) {
      if (dump)
        printf("LD (BC), A\n");

      cpu->mem[(cpu->registers[b] << 8) | cpu->registers[c]] = a;
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

    // RCLA
    else if (rom->data[cpu->pc] == 0x07) {
      if (dump)
        printf("RCLA\n");

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

      cpu->mem[cpu->pc + 1] = (cpu->sp & 0xFF);
      cpu->mem[cpu->pc + 2] = (cpu->sp);

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

			cpu->registers[a] = cpu->mem[(cpu->registers[b] << 8) | cpu->registers[c]];
			cpu->pc++;
		}

    else {
      if (dump)
        printf("Invalid opcode: 0x%02X\n", rom->data[cpu->pc]);
    }

    if (print_regs) {
      printf("cpu_execute(): --End--\n\n");
      sleep(3);
    }
  }
}
