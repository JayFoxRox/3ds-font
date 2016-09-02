#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bcfnt.h"
#include "morton.h"
#include "unicode.h"

uint32_t base = 0x18000000;

static uint32_t getAddress(uint32_t address) {
  // NULL-ptr is an exception
  if (address == 0) {
    return 0;
  }
  assert(address >= base);
  return address - base;
}

static void printAddress_(const char* label, uint32_t address) {
  printf("%s = 0x%" PRIX32 "; // (in File: 0x%" PRIX32 ")\n", label, address, getAddress(address));
}
#define printAddress(address) printAddress_(#address, (address))

static void printU8_(const char* label, uint8_t value) {
  printf("%s = %" PRIu8 "; // (0x%02" PRIX8 ")\n", label, value, value);
}
static void printU16_(const char* label, uint16_t value) {
  printf("%s = %" PRIu16 "; // (0x%04" PRIX16 ")\n", label, value, value);
}
static void printU32_(const char* label, uint32_t value) {
  printf("%s = %" PRIu32 "; // (0x%08" PRIX32 ")\n", label, value, value);
}
#define printField(field) _Generic((field), \
    uint8_t: printU8_, \
    uint16_t: printU16_, \
    uint32_t: printU32_ \
  )(#field, (field));

static void printTGLP(struct TGLP tglp) {
  printf("struct TGLP tglp;\n");
  //FIXME: printField(tglp.magic);
  printField(tglp.section_size);
  printField(tglp.cell_width);
  printField(tglp.cell_height);
  printField(tglp.baseline_position);
  printField(tglp.max_character_width);
  printField(tglp.sheet_size);
  printField(tglp.num_sheets);
  printField(tglp.sheet_image_format);
  printField(tglp.num_columns);
  printField(tglp.num_rows);
  printField(tglp.sheet_width);
  printField(tglp.sheet_height);
  printAddress(tglp.sheet_data_offset);
  printf("// %lu unknown bytes in TGLP section\n", tglp.section_size - sizeof(tglp));
}

static void printFINF(struct FINF finf) {
  printf("struct FINF finf;\n");
  //FIXME: printField(finf.magic);
  printField(finf.section_size);
  printField(finf.font_type);
  printField(finf.line_feed);
  printField(finf.alter_char_index);
  //FIXME: printField(finf.default_width);
  printField(finf.encoding);
  printAddress(finf.tglp_offset);
  printAddress(finf.cwdh_offset);
  printAddress(finf.cmap_offset);
  printField(finf.height);
  printField(finf.width);
  printField(finf.ascent);
  printField(finf.reserved);
  printf("// %lu unknown bytes in FINF section\n", finf.section_size - sizeof(finf));
}

static void printCMAP(struct CMAP cmap) {
  printf("struct CMAP cmap;\n");
  //FIXME: printField(cmap.magic);
  printField(cmap.section_size);
  printField(cmap.code_begin);
  printField(cmap.code_end);
  printField(cmap.mapping_method);
  printField(cmap.reserved);
  printAddress(cmap.next_cmap_offset); // Points directly into the section (.code_begin instead of .magic)
  printf("// %lu unknown bytes in CMAP section\n", cmap.section_size - sizeof(cmap));
}

static void printCWDH(struct CWDH cwdh) {
  printf("struct CWDH cwdh;\n");
  //FIXME: printField(cwdh.magic);
  printField(cwdh.section_size);
  printField(cwdh.start_index);
  printField(cwdh.end_index);
  printField(cwdh.next_cwdh_offset);
  printf("// %lu unknown bytes in CWDH section\n", cwdh.section_size - sizeof(cwdh));
}

int main(int argc, char* argv[]) {

  // Parse the options

  if (argc != 2) {
    printf("Usage: %s <path>\n", argv[0]);
    return 1;
  }
  const char* path = argv[1];

  // Load the font file

  FILE* font = fopen(path, "rb");
  if (font == NULL) {
    fprintf(stderr, "Could not load '%s'\n", path);
    return 1;
  }

  // Somehow we got 128 bytes of garbage? (Related to shared memory?!)

  fseek(font, 128, SEEK_CUR);

  // Read the CFNU / CFNT header

  struct CFNT cfnt;
  assert(fread(&cfnt, 1, sizeof(cfnt), font) == sizeof(cfnt));

  uint8_t cfnt_magic[4] = { 'C', 'F', 'N', 'T' };
  uint8_t cfnu_magic[4] = { 'C', 'F', 'N', 'U' };
  uint8_t ffnt_magic[4] = { 'F', 'F', 'N', 'T' };
  assert(!memcmp(cfnt.magic, cfnt_magic, 4) ||
         !memcmp(cfnt.magic, cfnu_magic, 4) ||
         !memcmp(cfnt.magic, ffnt_magic, 4));

  assert(cfnt.endianness == 0xFEFF);
  assert(cfnt.header_size == sizeof(cfnt));

  // We only support version 3 at the moment
  // FIXME: Support other versions  

  assert(cfnt.version == 0x03000000);

  // Read the FINF header

  size_t finf_offset = ftell(font);

  struct FINF finf;
  assert(fread(&finf, 1, sizeof(finf), font) == sizeof(finf));

  uint8_t finf_magic[4] = { 'F', 'I', 'N', 'F' };
  assert(!memcmp(finf.magic, finf_magic, 4));
  printFINF(finf);
  fseek(font, finf_offset + finf.section_size, SEEK_SET);

  // Read the TGLP header

  size_t tglp_offset = ftell(font);
  struct TGLP tglp;
  assert(fread(&tglp, 1, sizeof(tglp), font) == sizeof(tglp));
  uint8_t tglp_magic[4] = { 'T', 'G', 'L', 'P' };
  assert(!memcmp(tglp.magic, tglp_magic, 4));
  printTGLP(tglp);

  // Hack to extract all sheets

  size_t origin = ftell(font);
  fseek(font, tglp.sheet_data_offset - base, SEEK_SET);
  char name[128];
  for(unsigned int i = 0; i < tglp.num_sheets; i++) {
    sprintf(name, "tmp/sheet-%d.bin", i);
    FILE* tmp = fopen(name, "wb");
    uint32_t size = tglp.sheet_width * tglp.sheet_height;
    size /= 2; // A4 requires 4 bit per pixel!
    uint8_t* data = malloc(size);
    fread(data, 1, size, font);
    fwrite(data, 1, size, tmp);
    free(data);
    fclose(tmp);
  }
  fseek(font, origin, SEEK_SET);
  fseek(font, tglp_offset + tglp.section_size, SEEK_SET);

  // Read the CWDH header

  size_t cwdh_offset = ftell(font);
  struct CWDH cwdh;
  assert(fread(&cwdh, 1, sizeof(cwdh), font) == sizeof(cwdh));
  printCWDH(cwdh);
  struct CWDHEntry {
    uint8_t left;
    uint8_t glyph_width;
    uint8_t char_width;
  } cwdh_entry;
  printf("Character Width: %" PRIu16 " to %" PRIu16 "\n", cwdh.start_index, cwdh.end_index);
  for(uint16_t index = cwdh.start_index; index <= cwdh.end_index; index++) {
    assert(fread(&cwdh_entry, 1, sizeof(cwdh_entry), font) == sizeof(cwdh_entry));
    printf("\t%" PRIu16 ": %" PRIu8 ", %" PRIu8 ", %" PRIu8 "\n",
           index, cwdh_entry.left, cwdh_entry.glyph_width, cwdh_entry.char_width);
  }
  fseek(font, cwdh_offset + cwdh.section_size, SEEK_SET);

  // Read the CMAP header

  while(1) {
    size_t cmap_offset = ftell(font);

    struct CMAP cmap;
    assert(fread(&cmap, 1, sizeof(cmap), font) == sizeof(cmap));
    uint8_t cmap_magic[4] = { 'C', 'M', 'A', 'P' };
    assert(!memcmp(cmap.magic, cmap_magic, 4));
    printCMAP(cmap);
    switch(cmap.mapping_method) {
      case 0: // Direct
        printf("Mapping Direct: 0x%" PRIX16 " to 0x%" PRIX16 "\n", cmap.code_begin, cmap.code_end);
        uint16_t index;
        assert(fread(&index, 1, sizeof(index), font) == sizeof(index));
        for(uint16_t code = cmap.code_begin; code <= cmap.code_end; code++) {
          printf("\t%" PRIu16 " = 0x%" PRIX16 " (%s)\n", index, code, unicodeRange(code));
          index++;
        }
        break;
      case 1: // Table
        printf("Mapping Table: 0x%" PRIX16 " to 0x%" PRIX16 "\n", cmap.code_begin, cmap.code_end);
        for(uint16_t code = cmap.code_begin; code <= cmap.code_end; code++) {
          uint16_t index;
          assert(fread(&index, 1, sizeof(index), font) == sizeof(index));
          printf("\t%" PRIu16 " = 0x%" PRIX16 " (%s)\n", index, code, unicodeRange(code));
        }
        break;
      case 2: // Scan
        printf("Mapping Scan:\n");
        assert(cmap.code_begin == 0x0000);
        assert(cmap.code_end == 0xFFFF);
        uint16_t count;
        assert(fread(&count, 1, sizeof(count), font) == sizeof(count));
        for(uint16_t i = 0; i < count; i++) {
          uint16_t code;
          uint16_t index;
          assert(fread(&code, 1, sizeof(code), font) == sizeof(code));
          assert(fread(&index, 1, sizeof(index), font) == sizeof(index));
          printf("\t%" PRIu16 " = 0x%" PRIX16 " (%s)\n", index, code, unicodeRange(code));
        }
        break;
      default:
        assert(false);
        break;
    }
    fseek(font, cmap_offset + cmap.section_size, SEEK_SET);

    if (cmap.next_cmap_offset == 0) {
      break;
    }
  }

  // Close the font file

  fclose(font);  
  return 0;
}
