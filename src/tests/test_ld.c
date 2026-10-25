#include "../cpu.h"
#include "../rom.h"
#include "../memory.h"
#include "unity.h"
#include <stdio.h>
#include <assert.h>

void setUp(void) {} //Nothing to do yet
void tearDown(void) {} //Nothing to do yet

void test_ld(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /*
SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

Main:
	ld B, 0xD9
	ld C, B
	ld H, 0xFF
	ld L, 0xAF
	ld [HL], B
	
	halt
*/

  const size_t dummy_rom_size = 12;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x03, 0x01, 0x06, 0xd9, 0x48, 0x26, 0xff, 0x2e, 0xaf, 0x70, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[b]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[c]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, read_memory8(cpu, 0xFFAF));

  cpu_cleanup(cpu);
}

int main(void) {
  UNITY_BEGIN();

  printf("main(): Beginning test\n");
  RUN_TEST(test_ld);

  return UNITY_END();
}
