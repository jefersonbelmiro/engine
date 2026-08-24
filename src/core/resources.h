#pragma once

#include "core/defs.h"

typedef enum {
  RESOURCE_TYPE_TEXTURE,
  RESOURCE_TYPE_ATLAS,
  RESOURCE_TYPE_FONT,
  RESOURCE_TYPE_SOUND,
  RESOURCE_TYPE_MUSIC,
  // RESOURCE_TYPE_SHADER,
  RESOURCE_TYPE_COUNT,
} resource_type_t;

typedef struct  {
  u8         *buffer;
  char       *ext;
  u32         size;
} resource_texture_t;

typedef struct  {
  u8         *buffer;
  char       *ext;
  vec2_t      cell_size;
  u32         size;
} resource_atlas_t;

typedef struct  {
  u8         *buffer;
  char       *ext;
  u32         size;
} resource_font_t;

typedef struct  {
  u8         *buffer;
  char       *ext;
  float       volume;
  u32         size;
  u8          max_active;
} resource_sound_t;

typedef struct  {
  u8         *buffer;
  char       *ext;
  float       volume;
  u32         size;
} resource_music_t;
