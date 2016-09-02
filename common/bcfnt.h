// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16_le;
typedef uint32_t u32_le;

struct CFNT {
    u8 magic[4];
    u16_le endianness;
    u16_le header_size;
    u32_le version;
    u32_le file_size;
    u32_le num_blocks;
};

struct FINF {
    u8 magic[4];
    u32_le section_size;
    u8 font_type;
    u8 line_feed;
    u16_le alter_char_index;
    u8 default_width[3];
    u8 encoding;
    u32_le tglp_offset;
    u32_le cwdh_offset;
    u32_le cmap_offset;
    u8 height;
    u8 width;
    u8 ascent;
    u8 reserved;
};

struct TGLP {
    u8 magic[4];
    u32_le section_size;
    u8 cell_width;
    u8 cell_height;
    u8 baseline_position;
    u8 max_character_width;
    u32_le sheet_size;
    u16_le num_sheets;
    u16_le sheet_image_format;
    u16_le num_columns;
    u16_le num_rows;
    u16_le sheet_width;
    u16_le sheet_height;
    u32_le sheet_data_offset;
};

struct CMAP {
    u8 magic[4];
    u32_le section_size;
    u16_le code_begin;
    u16_le code_end;
    u16_le mapping_method;
    u16_le reserved;
    u32_le next_cmap_offset;
};

struct CWDH {
    u8 magic[4];
    u32_le section_size;
    u16_le start_index;
    u16_le end_index;
    u32_le next_cwdh_offset;
};

enum SheetFormat {
  RGBA8 = 0,
  RGB8,
  RGBA5551,
  RGB565,
  RGBA4,
  LA8,
  HILO8,
  L8,
  A8,
  LA4,
  L4,
  A4,
  ETC1,
  ETC1A4
};
