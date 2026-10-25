#include "../cpu.h"
#include "../rom.h"
#include "unity.h"
#include <stdio.h>
#include <assert.h>

void setUp(void) {} //Nothing to do yet
void tearDown(void) {} //Nothing to do yet

void test_sub(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /* ASM
   SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

    Main:
	ld A, 0xA4
	ld B, 0xD9
	sub A, B
	
	halt
   */

  const size_t dummy_rom_size = 9;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x03, 0x01, 0x3e, 0xa4, 0x06, 0xd9, 0x90, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0xCB, cpu->registers[a]);
  TEST_ASSERT_EQUAL_HEX8(0x70, cpu->registers[f]);

  cpu_cleanup(cpu);
}

int main(void) {
  UNITY_BEGIN();

  printf("main(): Beginning test\n");
  RUN_TEST(test_sub);
  
  return UNITY_END();
}
