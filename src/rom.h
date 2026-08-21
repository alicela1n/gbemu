#ifndef ROM_H
#define ROM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ROM {
  uint8_t *data;
  size_t size;

  char *title;
  char *title_length;

  bool cgb_flag;
  bool cgb_only;

  uint8_t cartridge_rom_size;
  uint8_t cartridge_ram_size;
};

bool validate_rom(struct ROM *rom, bool dump);
void get_title(struct ROM *rom);
bool cgb_flag(struct ROM *rom);
uint32_t get_cartridge_rom_size(struct ROM *rom);
uint32_t get_cartridge_ram_size(struct ROM *rom);
size_t calculate_filesize(FILE *fp);
int load_rom(FILE *fp, struct ROM *rom);
void dummy_rom(const uint8_t data[], const size_t data_size, struct ROM *rom);
void rom_cleanup(struct ROM *rom);

#endif
