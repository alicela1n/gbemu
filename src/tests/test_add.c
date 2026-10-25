#include "../cpu.h"
#include "../rom.h"
#include "unity.h"
#include <stdio.h>
#include <assert.h>

void setUp(void) {} //Nothing to do yet
void tearDown(void) {} //Nothing to do yet

void test_add(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /* ASM
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld B, 0xA4
	ld A, 0xD9
	add A, B
	
	halt
  */

  const size_t dummy_rom_size = 9;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x03, 0x01, 0x06, 0xa4, 0x3e, 0xd9, 0x80, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0x7D, cpu->registers[a]);
  TEST_ASSERT_EQUAL_HEX8(0x10, cpu->registers[f]);

  

  cpu_cleanup(cpu);
}

int main(void) {
  UNITY_BEGIN();

  printf("main(): Beginning test\n");
  test_add();
  
  return UNITY_END();
}
