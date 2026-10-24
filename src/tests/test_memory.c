#include "../cpu.h"
#include "../rom.h"
#include "../memory.h"
#include "unity.h"
#include <stdio.h>
#include <assert.h>

void setUp(void) {} //Nothing to do yet
void tearDown(void) {} //Nothing to do yet

void test_memory(void) {
  struct CPU *cpu = calloc(1, sizeof(struct CPU));
  cpu_init(cpu);

  /* Initial memory test */
  uint8_t v = 0;
  for (uint16_t i = 0x8000; i < 0x7FFF; i++) {
    write_memory8(cpu, v, i);
    v++;
  }

  v = 0;
  for (uint16_t i = 0x8000; i < 0x7FFF; i++) {
    TEST_ASSERT_EQUAL_HEX8(v, read_memory8(cpu, i));
    v++;
  }

  cpu_cleanup(cpu);
}

int main(void) {
  UNITY_BEGIN();

  printf("main(): Beginning test\n");
  test_memory();
  
  return UNITY_END();
}
