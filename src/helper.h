#ifndef HELPER_H
#define HELPER_H

#include <stddef.h>
#include <stdint.h>

typedef struct string {
  const char *ptr;
  size_t len;
} string;

static inline uint16_t uint8to16(uint8_t lo, uint8_t hi) {
  return ((uint16_t)hi << 8) | (uint16_t)lo;
}

#endif
