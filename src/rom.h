#ifndef ROM_H
#define ROM_H

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>

struct ROM {
  uint8_t *data;
  size_t size;

  std::string title;

  bool cgb_flag;
  bool cgb_only;

  uint8_t cartridge_rom_size;
  uint8_t cartridge_ram_size;
};

bool validate_rom(const uint8_t *rom, size_t rom_size, bool dump);
std::string get_title(const uint8_t *rom, size_t rom_size);
bool cgb_flag(const uint8_t *rom, size_t rom_size, bool *cgb_only);
uint32_t get_cartridge_rom_size(const uint8_t *rom, size_t rom_size);
uint32_t get_cartridge_ram_size(const uint8_t *rom, size_t rom_size);
size_t calculate_filesize(FILE *fp);
int load_rom(FILE *fp, struct ROM *rom);
void dummy_rom(const uint8_t data[], const size_t data_size, struct ROM *rom);

#endif
