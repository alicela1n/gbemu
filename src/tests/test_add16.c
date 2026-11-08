#include "../cpu.h"
#include "../rom.h"
#include "unity.h"
#include <stdio.h>
#include <assert.h>

void setUp(void) {} //Nothing to do yet
void tearDown(void) {} //Nothing to do yet

void test_add16(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /* ASM
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld BC, 0xA4D9
	ld HL, 0xD9A4
	add HL, BC
	
	halt
    */

  const size_t dummy_rom_size = 11;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x03, 0x01, 0x01, 0xd9, 0xa4, 0x21, 0xa4, 0xd9, 0x09, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX16(0x7E7D, cpu_read_reg16(cpu, hl));
  TEST_ASSERT_EQUAL_HEX8(0x90, cpu->registers[f]);

  cpu_cleanup(cpu);
}

void test_add16_2(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /* ASM
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld DE, 0xA4D9
	ld HL, 0xD9A4
	add HL, DE
	
	halt
    */

  const size_t dummy_rom_size = 11;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x11, 0xd9, 0xa4, 0x21, 0xa4, 0xd9, 0x19, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX16(0x7E7D, cpu_read_reg16(cpu, hl));
  TEST_ASSERT_EQUAL_HEX8(0x90, cpu->registers[f]);

  cpu_cleanup(cpu);
}

void test_add16_3(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /* ASM
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld HL, 0xD9A4
	add HL, HL
	
	halt
    */

  const size_t dummy_rom_size = 8;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x21, 0xa4, 0xd9, 0x29, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX16(0x7E7D, cpu_read_reg16(cpu, hl));
  TEST_ASSERT_EQUAL_HEX8(0xB0, cpu->registers[f]);

  cpu_cleanup(cpu);
}

void test_add16_4(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /* ASM
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld HL, 0xD9A4
	add HL, HL
	
	halt
    */

  const size_t dummy_rom_size = 11;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x31, 0xd9, 0xa4, 0x21, 0xa4, 0xd9, 0x39, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX16(0x7E7D, cpu_read_reg16(cpu, hl));
  TEST_ASSERT_EQUAL_HEX8(0x90, cpu->registers[f]);

  cpu_cleanup(cpu);
}

int main(void) {
  UNITY_BEGIN();

  printf("main(): Beginning test\n");
  RUN_TEST(test_add16);
  RUN_TEST(test_add16_2);
  RUN_TEST(test_add16_3);
  RUN_TEST(test_add16_4);
  
  return UNITY_END();
}
