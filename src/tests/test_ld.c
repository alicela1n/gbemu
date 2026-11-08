#include "../cpu.h"
#include "../rom.h"
#include "../memory.h"
#include "unity.h"
#include <stdio.h>
#include <assert.h>

void setUp(void) {} //Nothing to do yet
void tearDown(void) {} //Nothing to do yet

void test_ld1(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /*
    SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

    Main:
	ld B, 0xD9
	ld C, B
	ld D, 0xBC
	ld E, D
	ld B, D
	ld A, [DE]
	ld L, D
	ld H, C
	
	halt
*/

  const size_t dummy_rom_size = 14;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x6, 0xd9, 0x48, 0x16, 0xbc, 0x5a, 0x42, 0x1a, 0x6a, 0x61, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0xBC, cpu->registers[b]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[c]);
  TEST_ASSERT_EQUAL_HEX8(0xBC, cpu->registers[d]);
  TEST_ASSERT_EQUAL_HEX8(0xBC, cpu->registers[e]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[h]);
  TEST_ASSERT_EQUAL_HEX8(0xBC, cpu->registers[l]);

  cpu_cleanup(cpu);
}

void test_ld2(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /*
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	;; Write code here
	ld H, 0xFF
	ld L, 0xAF
	ld [HL], 0xA9
	ld B, [HL]
	ld C, [HL]
	ld D, [HL]
	ld E, [HL]
	ld A, [HL]
	ld H, [HL]
	ld [HL], 0xA0
	ld L, [HL]
	
	halt
*/

  const size_t dummy_rom_size = 20;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x03, 0x01, 0x26, 0xff, 0x2e, 0xaf, 0x36, 0xa9, 0x46, 0x4e, 0x56, 0x5e, 0x7e, 0x66, 0x36, 0xa0, 0x6e, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0xA9, cpu->registers[b]);
  TEST_ASSERT_EQUAL_HEX8(0xA9, cpu->registers[c]);
  TEST_ASSERT_EQUAL_HEX8(0xA9, cpu->registers[d]);
  TEST_ASSERT_EQUAL_HEX8(0xA9, cpu->registers[e]);
  TEST_ASSERT_EQUAL_HEX8(0xA9, cpu->registers[h]);
  TEST_ASSERT_EQUAL_HEX8(0xA0, cpu->registers[l]);
  TEST_ASSERT_EQUAL_HEX8(0xA9, cpu->registers[a]);
  TEST_ASSERT_EQUAL_HEX8(0xA9, read_memory8(cpu, 0xFFAF));
  TEST_ASSERT_EQUAL_HEX8(0xA0, read_memory8(cpu, 0xA9AF));

  cpu_cleanup(cpu);
}

void test_ld3(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /*
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld HL, 0xC000
	ld B, 0xCA
	ld C, 0xCA
	ld D, 0xCA
	ld E, 0xCA
	ld A, 0xCA
	ld [HL], B
	inc HL
	ld [HL], C
	inc HL
	ld [HL], D
	inc HL
	ld [HL], E
	inc HL
	ld [HL], A
	inc HL
	ld [HL], H
	inc HL
	ld [HL], L
	
	halt
*/

  const size_t dummy_rom_size = 30;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x21, 0x0, 0xc0, 0x6, 0xca, 0xe, 0xca, 0x16, 0xca, 0x1e, 0xca, 0x3e, 0xca, 0x70, 0x23, 0x71, 0x23, 0x72, 0x23, 0x73, 0x23, 0x77, 0x23, 0x74, 0x23, 0x75, 0x76};
  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);
  
  for (uint16_t i = 0xC000; i <= 0xC004; i++) {
    printf("0x%04X: %02X\n", i, read_memory8(cpu, i));
  }

  for (uint16_t i = 0xC000; i <= 0xC004; i++) {
    TEST_ASSERT_EQUAL_HEX8(0xCA, read_memory8(cpu, i));
  }

  cpu_cleanup(cpu);
}

void test_ld4(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /*
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld B, 0xD9
	ld B, B
	ld C, B
	ld D, B
	ld E, B
	ld H, B
	ld L, B
        ld A, B
	halt
*/

  const size_t dummy_rom_size = 13;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x6, 0xd9, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x78, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[b]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[c]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[d]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[e]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[h]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[l]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[a]);

  cpu_cleanup(cpu);
}

void test_ld5(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /*
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld C, 0xD9
	ld B, C
	ld C, C
	ld D, C
	ld E, C
	ld H, C
	ld L, C
	ld A, C
	halt
*/

  const size_t dummy_rom_size = 13;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0xe, 0xd9, 0x41, 0x49, 0x51, 0x59, 0x61, 0x69, 0x79, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[b]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[c]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[d]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[e]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[h]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[l]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[a]);

  cpu_cleanup(cpu);
}

void test_ld6(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /*
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	;; Write code here
	ld D, 0xD9
	ld B, D
	ld C, D
	ld D, D
	ld E, D
	ld H, D
	ld L, D
	ld A, D
	halt
*/

  const size_t dummy_rom_size = 13;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x16, 0xd9, 0x42, 0x4a, 0x52, 0x5a, 0x62, 0x6a, 0x7a, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[b]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[c]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[d]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[e]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[h]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[l]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[a]);

  cpu_cleanup(cpu);
}

void test_ld7(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /*
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	;; Write code here
	ld E, 0xD9
	ld B, E
	ld C, E
	ld D, E
	ld E, E
	ld H, E
	ld L, E
	ld A, E
	halt
*/

  const size_t dummy_rom_size = 13;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x1e, 0xd9, 0x43, 0x4b, 0x53, 0x5b, 0x63, 0x6b, 0x7b, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[b]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[c]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[d]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[e]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[h]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[l]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[a]);

  cpu_cleanup(cpu);
}

void test_ld8(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /*
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	;; Write code here
	ld H, 0xD9
	ld B, H
	ld C, H
	ld D, H
	ld E, H
	ld H, H
	ld L, H
	ld A, H
	halt

*/

  const size_t dummy_rom_size = 13;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x26, 0xd9, 0x44, 0x4c, 0x54, 0x5c, 0x64, 0x6c, 0x7c, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[b]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[c]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[d]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[e]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[h]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[l]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[a]);

  cpu_cleanup(cpu);
}

void test_ld9(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /*
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	ld L, 0xD9
	ld B, L
	ld C, L
	ld D, L
	ld E, L
	ld H, L
	ld L, L
	ld A, L
	halt
*/

  const size_t dummy_rom_size = 13;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x2e, 0xd9, 0x45, 0x4d, 0x55, 0x5d, 0x65, 0x6d, 0x7d, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[b]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[c]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[d]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[e]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[h]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[l]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[a]);

  cpu_cleanup(cpu);
}

void test_ld10(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /*
  SECTION "header", ROM0[$100]
	ds $100 - @, 0

	jp nz, Main

  Main:
	;; Write code here
	ld A, 0xD9
	ld B, A
	ld C, A
	ld D, A
	ld E, A
	ld H, A
	ld L, A
	ld A, A
	halt
*/

  const size_t dummy_rom_size = 13;
  const uint8_t dummy_rom_data[dummy_rom_size] = {0xc2, 0x3, 0x1, 0x3e, 0xd9, 0x47, 0x4f, 0x57, 0x5f, 0x67, 0x6f, 0x7f, 0x76};

  dummy_rom(dummy_rom_data, dummy_rom_size, cpu->rom, false);

  cpu_execute(cpu, true, false, false, true);

  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[b]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[c]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[d]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[e]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[h]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[l]);
  TEST_ASSERT_EQUAL_HEX8(0xD9, cpu->registers[a]);

  cpu_cleanup(cpu);
}


int main(void) {
  UNITY_BEGIN();

  printf("main(): Beginning test\n");
  RUN_TEST(test_ld1);
  RUN_TEST(test_ld2);
  RUN_TEST(test_ld3);
  RUN_TEST(test_ld4);
  RUN_TEST(test_ld5);
  RUN_TEST(test_ld6);
  RUN_TEST(test_ld7);
  RUN_TEST(test_ld8);
  RUN_TEST(test_ld9);
  RUN_TEST(test_ld10);

  return UNITY_END();
}
