#ifndef HELPER_H
#define HELPER_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct string {
  const char *ptr;
  size_t len;
} string;

static inline uint16_t uint8to16(uint8_t lo, uint8_t hi) {
  return ((uint16_t)hi << 8) | (uint16_t)lo;
}

// https://stackoverflow.com/a/19216660/15526385
static inline unsigned char circshift(unsigned char x, int n) {
  return (x << n) | (x >> (8 - n));
}

static inline uint16_t swap_byte16(uint16_t n) {
  return (n << 8) | (n >> 8);
}

/* calculate_filesize(): Return the size of a FILE object.
 */
static inline size_t calculate_filesize(FILE *fp) {
  fseek(fp, 0L, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  return size;
}

/* cleanup_string(): Clean up a string
 */
static inline void cleanup_string(string *str) {
  free((char *)str->ptr);
  free(str);
}

/* new_string(): Create new string
 */
static inline string * new_string(const char str[]) {
  string *new_str = malloc(sizeof(string));
  memset(new_str, 0, sizeof(string));

  new_str->ptr = malloc(sizeof(char *) * strlen(str));
  strncpy((char *)new_str->ptr, str, strlen(str));
  new_str->len = strlen(str);

  return new_str;
}

#endif
