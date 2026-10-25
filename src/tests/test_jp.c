#include "../cpu.h"
#include "../rom.h"
#include "unity.h"
#include <stdio.h>
#include <assert.h>

void setUp(void) {} //Nothing to do yet
void tearDown(void) {} //Nothing to do yet

void test_jp(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /* ASM
SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

Main:
	jp .conditional

.halt:
	halt

.conditional:
	ld B, 0xA4
	ld A, 0xA4
	sub A, B
	jp z, .halt
  */

  const size_t dummy_rom_size = 15;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x03, 0x01, 0xc3, 0x07, 0x01, 0x76, 0x06, 0xa4, 0x3e, 0xa4, 0x90, 0xca, 0x06, 0x01};


  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0x00, cpu->registers[a]);
  TEST_ASSERT_EQUAL_HEX8(0xA4, cpu->registers[b]);

  cpu_cleanup(cpu);
}

int main(void) {
  UNITY_BEGIN();

  printf("main(): Beginning test\n");
  RUN_TEST(test_jp);
  
  return UNITY_END();
}
