#include "cpu.h"

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

/* cpu_execute(): What this does is decode each opcode and translate it into an instruction 
 *                to be cpu_execute by the virtual CPU. Optional dump flag for disassembly upon execution.
 */
void cpu_execute(struct CPU *cpu, const struct ROM *rom, bool dump) {
    while(!cpu->halt) {

			// NOP
       if (rom->data[cpu->pc] == 0x00) {
         if (dump)
       	  printf("NOP\n");
			 	cpu->pc++;
       }

			 // LD BC, d16
			 if (rom->data[pc] == 0x01) {
			 	if (dump)
			 		printf("LD BC, %02X%02X\n", rom->data[cpu->pc + 1], rom->data[cpu->pc + 2]);

			 	c = rom->data[cpu->pc + 1];
			 	b = rom->data[cpu->pc + 2];
			 	cpu->pc += 3;
			 }

			 // LD (BC), A
			 if (rom->data[pc] == 0x02) {
			 	if (dump)
			 		printf("LD (BC), A\n");

				cpu->mem[(cpu->b << 8) | cpu->c] = a;
				cpu->pc++;
			 }

			 // INC BC
			 if (rom->data[pc] == 0x03) {
				 if (dump)
					printf("INC BC\n");

				cpu->b++;
				cpu->c++;

				cpu->pc++;
			 }
			 
			 // INC B
			 if (rom->data[pc] == 0x04) {
				if (dump)
				 	printf("INC B\n");

			 	cpu->b++;
			 	cpu->pc++;
			 }

			 // DEC B
			 if (rom->data[pc] == 0x05) {
				 if (dump)
					 printf("DEC B\n");

				 cpu->b--;
				 cpu->pc++;
			 }

			 // LD B, d8
			 if (rom->data[pc] == 0x06) {
				 if (dump)
					 printf("LD B, %02X", rom->data[cpu->pc + 1]);

				 cpu->b = rom->data[cpu->pc + 1];

				 cpu->pc+= 2;
			 }
			 
			 // RCLA
			 if (rom->data[pc] == 0x07) {

			 }
    }
}
