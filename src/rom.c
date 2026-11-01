#include "rom.h"
#include "helper.h"

/* validate_rom(): Validate that the rom is valid, the GameBoy requires that all
 * roms have a nintendo logo at 0x104 to 0x133. If the logo isn't present or was
 * changed, the bootrom will reject the rom. If the dump boolean passed is true,
 * dump the logo from the rom.
 */
bool validate_rom(struct ROM *rom, bool dump) {
  const uint8_t nintendo_logo[] = {
      0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83,
      0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
      0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63,
      0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E};
  const uint8_t *logo_offset = rom->data + 0x104;
  const uint8_t *logo_end_offset = rom->data + 0x133;
  const uint8_t *rom_end_offset = rom->data + rom->size;

  ptrdiff_t logo_offset_diff = logo_end_offset - logo_offset;
  ptrdiff_t rom_offset_diff = rom_end_offset - rom->data;

  if (rom->size < (32 * 1024))
    return false;

  if (rom_offset_diff < 0x150)
    return false;

  for (int i = 0; i < logo_offset_diff; i++) {
    if (dump)
      printf("%02X ", logo_offset[i]);
    if (logo_offset[i] != nintendo_logo[i]) {
      return false;
    }
  }

  // The calculated checksum is compared to the byte at 0x14D of the ROM,
  // on a real GameBoy the ROM won't start if these don't match
  uint8_t checksum = 0;

  // Calculated checksum
  for (uint16_t address = (uint16_t)0x134; address <= (uint16_t)0x14C;
       address++) {
    checksum = checksum - rom->data[address] - 1;
  }

  if ((checksum & 0xFF) != rom->data[0x14D])
    return false;

  if (dump)
    printf("\n");

  return true;
}

/* get_title(): Return the title from the rom and return it as a string,
 *              on GameBoy roms the title is located at 0x134 to 0x143.
 */
void get_title(struct ROM *rom) {
  const uint8_t *title_offset = rom->data + 0x134;
  const uint8_t *title_end_offset = rom->data + 0x143;
  ptrdiff_t title_offset_diff = title_end_offset - title_offset;

  rom->title = malloc(title_offset_diff * sizeof(char));

  for (int i = 0; i < title_offset_diff; i++) {
    rom->title[i] = title_offset[i];
    rom->title_length++;
  }
}

/* cgb_flag(): Return the CGB flag which tells you if the game supports the
 * GameBoy Color, is exclusive to the GameBoy Color, or B&W GameBoy. Using the
 * cgb_only bool, which gets passed as a pointer and set to flag that the game
 * only supports GBC.
 */
bool cgb_flag(struct ROM *rom) {
  const uint8_t *cgb_flag_offset = rom->data + 0x143;

  if (*cgb_flag_offset == 0x80 || *cgb_flag_offset == 0xC0) {
    if (*cgb_flag_offset == 0xC0) {
      rom->cgb_only = true;
    } else {
      rom->cgb_only = false;
    }
    return true;
  }
  return false;
}

/* get_cartridge_rom_size(): Return the size of the rom on the cartridge in KiB,
 * this is different from the file rom size and is found by computing 32768 by
 * (1 << 0x148).
 */
uint32_t get_cartridge_rom_size(struct ROM *rom) {
  uint16_t cartridge_rom_size;
  const uint8_t *cartridge_rom_size_offset = rom->data + 0x148;

  cartridge_rom_size = *cartridge_rom_size_offset;
  return 32768 * (1 << cartridge_rom_size);
}

/* get_cartridge_ram_size(): Return the size of the ram on the cartridge in KiB,
 * I'm not sure how or if this value is computed so I used switch statements to
 * get it.
 */
uint32_t get_cartridge_ram_size(struct ROM *rom) {
  uint32_t cartridge_ram_size;
  const uint8_t *cartridge_ram_size_offset = rom->data + 0x149;

  switch (*cartridge_ram_size_offset) {
  case 0:
    cartridge_ram_size = 0;
    break;
  case 1:
    cartridge_ram_size = 0;
    break;
  case 2:
    cartridge_ram_size = 8;
    break;
  case 3:
    cartridge_ram_size = 32;
    break;
  case 4:
    cartridge_ram_size = 128;
    break;
  case 5:
    cartridge_ram_size = 64;
    break;
  default:
    cartridge_ram_size = 0;
    break;
  }

  return cartridge_ram_size;
}

/* dummy_rom(): Create a dummy rom out of an array of opcodes
 * to use for unit testing.
 */
void dummy_rom(const uint8_t data[], const size_t data_size, struct ROM *rom) {
  rom->size = data_size;
  rom->data = (uint8_t *)malloc(sizeof(uint8_t) * (rom->size + 0x100));
  memcpy((void *)(rom->data + 0x100), (void *)data, rom->size);

  printf("dummy_rom(): Begin\n");
  for (int i = 0; i < rom->size + 0x100; i++) {
    printf("[0x%02X]: 0x%02X ", i, rom->data[i]);
  }
  printf("\n");
  printf("dummy_rom(): End\n");
}

/* load_rom(): Load a rom from a file and populate the rom
 * struct with rom metadata.
 */
int load_rom(FILE *fp, struct ROM *rom) {
  rom->size = calculate_filesize(fp);
  rom->data = (uint8_t *)malloc(sizeof(uint8_t) * rom->size);
  fread(rom->data, 1, rom->size, fp);

  if (!validate_rom(rom, false)) {
    return 1;
  }

  get_title(rom);
  rom->cgb_flag = cgb_flag(rom);
  rom->cartridge_ram_size = get_cartridge_ram_size(rom);
  rom->cartridge_rom_size = get_cartridge_rom_size(rom);

  return 0;
}
 
/* rom_cleanup(): Clean up and free the rom struct
 */
void rom_cleanup(struct ROM *rom) {
  free(rom->data);
  rom->data = NULL;
  free(rom);
}
