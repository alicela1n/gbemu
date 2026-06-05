#include <cstdio>
#include "cpu.h"

int main(int argc, char **argv) {
	// Testing the CPU
	printf("main(): Initializing CPU\n");
	CPU cpu;
	cpu_init(&cpu);
	printf("main(): CPU initialized\n");

	return 0;
}
