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

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0x7D, cpu->registers[a]);
  TEST_ASSERT_EQUAL_HEX8(0x10, cpu->registers[f]);

  cpu_cleanup(cpu);
}

void test_add2(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /* ASM
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld C, 0xA4
	ld A, 0xD9
	add A, C
	
	halt
  */

  const size_t dummy_rom_size = 9;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0xe, 0xa4, 0x3e, 0xd9, 0x81, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0x7D, cpu->registers[a]);
  TEST_ASSERT_EQUAL_HEX8(0x10, cpu->registers[f]);

  cpu_cleanup(cpu);
}

void test_add3(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /* ASM
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld D, 0xA4
	ld A, 0xD9
	add A, D
	
	halt
  */

  const size_t dummy_rom_size = 9;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x16, 0xa4, 0x3e, 0xd9, 0x82, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0x7D, cpu->registers[a]);
  TEST_ASSERT_EQUAL_HEX8(0x10, cpu->registers[f]);

  cpu_cleanup(cpu);
}

void test_add4(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /* ASM
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld E, 0xA4
	ld A, 0xD9
	add A, E
	
	halt
  */

  const size_t dummy_rom_size = 9;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x1e, 0xa4, 0x3e, 0xd9, 0x83, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0x7D, cpu->registers[a]);
  TEST_ASSERT_EQUAL_HEX8(0x10, cpu->registers[f]);

  cpu_cleanup(cpu);
}

void test_add5(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /* ASM
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld H, 0xA4
	ld A, 0xD9
	add A, H
	
	halt
  */

  const size_t dummy_rom_size = 9;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x26, 0xa4, 0x3e, 0xd9, 0x84, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0x7D, cpu->registers[a]);
  TEST_ASSERT_EQUAL_HEX8(0x10, cpu->registers[f]);

  cpu_cleanup(cpu);
}

void test_add6(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /* ASM
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld L, 0xA4
	ld A, 0xD9
	add A, L
	
	halt
  */

  const size_t dummy_rom_size = 9;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x2e, 0xa4, 0x3e, 0xd9, 0x85, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0x7D, cpu->registers[a]);
  TEST_ASSERT_EQUAL_HEX8(0x10, cpu->registers[f]);

  cpu_cleanup(cpu);
}

void test_add7(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /* ASM
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld A, 0xD9
	add A, 0xA4
	
	halt
  */

  const size_t dummy_rom_size = 8;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x3e, 0xd9, 0xc6, 0xa4, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0x7D, cpu->registers[a]);
  TEST_ASSERT_EQUAL_HEX8(0x10, cpu->registers[f]);

  cpu_cleanup(cpu);
}

void test_add8(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /* ASM
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld HL, 0xC004
	ld [HL], 0xA4
	ld A, 0xD9
	
	add A, [HL]
	
	halt
  */

  const size_t dummy_rom_size = 12;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x21, 0x4, 0xc0, 0x36, 0xa4, 0x3e, 0xd9, 0x86, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0x7D, cpu->registers[a]);
  TEST_ASSERT_EQUAL_HEX8(0x10, cpu->registers[f]);

  cpu_cleanup(cpu);
}


int main(void) {
  UNITY_BEGIN();

  printf("main(): Beginning test\n");
  RUN_TEST(test_add);
  RUN_TEST(test_add2);
  RUN_TEST(test_add3);
  RUN_TEST(test_add4);
  RUN_TEST(test_add5);
  RUN_TEST(test_add6);
  RUN_TEST(test_add7);
  RUN_TEST(test_add8);
  
  return UNITY_END();
}
