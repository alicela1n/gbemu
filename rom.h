#ifndef ROM_H
#define ROM_H

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <fstream>

struct ROM {
	uint8_t *data;
	size_t size;

	uint8_t *title;

	bool cgb_flag;
	bool cgb_only;

	uint8_t cartridge_rom_size;
	uint8_t cartridge_ram_size;
	uint8_t cartridge_type;
};

bool validate_rom(const uint8_t *rom, size_t rom_size, bool dump);
std::string get_title(const uint8_t *rom, size_t rom_size);
bool cgb_flag(const uint8_t *rom, size_t rom_size, bool *cgb_only);
uint32_t get_cartridge_rom_size(const uint8_t *rom, size_t rom_size);
uint32_t get_cartridge_ram_size(const uint8_t *rom, size_t rom_size);
size_t calculate_filesize(FILE *fp);

#endif
