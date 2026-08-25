#pragma once

#include "core/mem.h"
#include "core/io.h"
#include "core/string.h"
#include "core/defs.h"
#include "core/arena.h"
#include "core/resources.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifndef PACKAGE_COMPRESS
#define PACKAGE_COMPRESS 1
#endif

#define PACKAGE_MAGIC            0x31474B50u // "PKG1"
#define PACKAGE_FLAG_NONE        0u
#define PACKAGE_FLAG_COMPRESSED  (1u << 0)

typedef struct {
  u16 textures;
  u16 atlas;
  u16 fonts;
  u16 sounds;
  u16 musics;
} package_count_t;

typedef struct {
  texture_t *textures;
  atlas_t   *atlas;
  font_t    *fonts;
  sound_t   *sounds;
  music_t   *musics;
} package_handler_t;

typedef struct {
  resource_texture_t *textures;
  resource_atlas_t   *atlas;
  resource_font_t    *fonts;
  resource_sound_t   *sounds;
  resource_music_t   *musics;
} package_resouce_t;

typedef struct {
  // resource_texture_t *textures;
  // resource_atlas_t   *atlas;
  // resource_font_t    *fonts;
  // resource_sound_t   *sounds;
  // resource_music_t   *musics;

  package_resouce_t resources;
  package_handler_t handlers;

  package_count_t     count;
} package_t;

typedef struct {
  u32 magic;
  u32 flags;
  u32 size;       // size of the whole package when decompressed (bytes)
  u16 counts[RESOURCE_TYPE_COUNT];
} package_header_t;

typedef struct  {
  char **id;
  char **path;
  u16    count;
  u16    cap;
} package_texture_t;

typedef struct  {
  char  **id;
  char  **path;
  vec2_t *cell_size;
  u16     count;
  u16     cap;
} package_atlas_t;

typedef struct  {
  char **id;
  char **path;
  u16    count;
  u16    cap;
} package_font_t;

typedef struct  {
  char  **id;
  char  **path;
  float  *volume;
  u8     *max_active;
  u16     count;
  u16     cap;
} package_sound_t;

typedef struct  {
  char **id;
  char **path;
  float *volume;
  u16    count;
  u16    cap;
} package_music_t;

typedef struct {
  package_texture_t textures;
  package_atlas_t   atlas;
  package_font_t    fonts;
  package_sound_t   sounds;
  package_music_t   musics;
} package_def_t;

API void package_def_init(package_def_t *pkg, package_count_t caps, arena_t *arena)
{
  pkg->textures = (package_texture_t) {
    .id = arena_push(arena, char*, caps.textures),
    .path = arena_push(arena, char*, caps.textures),
    .count = 0,
    .cap = caps.textures,
  };
  pkg->atlas = (package_atlas_t) {
    .id = arena_push(arena, char*, caps.atlas),
    .path = arena_push(arena, char*, caps.atlas),
    // .cell_size = (float (*)[2])arena_push_stride(arena, float, caps.atlas, sizeof(float)),
    .cell_size = arena_push_stride(arena, vec2_t, caps.atlas, sizeof(vec2_t)),
    .count = 0,
    .cap = caps.atlas,
  };
  pkg->fonts = (package_font_t) {
    .id = arena_push(arena, char*, caps.fonts),
    .path = arena_push(arena, char*, caps.fonts),
    .count = 0,
    .cap = caps.fonts,
  };
  pkg->sounds = (package_sound_t) {
    .id = arena_push(arena, char*, caps.sounds),
    .path = arena_push(arena, char*, caps.sounds),
    .volume = arena_push(arena, float, caps.sounds),
    .max_active = arena_push(arena, u8, caps.sounds),
    .count = 0,
    .cap = caps.sounds,
  };
  pkg->musics = (package_music_t) {
    .id = arena_push(arena, char*, caps.musics),
    .path = arena_push(arena, char*, caps.musics),
    .volume = arena_push(arena, float, caps.musics),
    .count = 0,
    .cap = caps.musics,
  };
}

API uint32_t package_def_append_texture(package_def_t *pkg, char *id, char *path)
{
  assert(pkg->textures.count < pkg->textures.cap);
  uint32_t index = pkg->textures.count++;
  pkg->textures.id[index] = id;
  pkg->textures.path[index] = path;
  return index;
}

API uint32_t package_def_append_atlas(package_def_t *pkg, char *id, char *path, vec2_t cell_size)
{
  assert(pkg->atlas.count < pkg->atlas.cap);
  uint32_t index = pkg->atlas.count++;
  pkg->atlas.id[index] = id;
  pkg->atlas.path[index] = path;
  mem_copy(&cell_size, pkg->atlas.cell_size, sizeof(vec2_t));
  return index;
}

API uint32_t package_def_append_font(package_def_t *pkg, char *id, char *path)
{
  assert(pkg->fonts.count < pkg->fonts.cap);
  uint32_t index = pkg->fonts.count++;
  pkg->fonts.id[index] = id;
  pkg->fonts.path[index] = path;
  return index;
}

API uint32_t package_def_append_sound(package_def_t *pkg, char *id, char *path, float volume, u8 max_active)
{
  assert(pkg->sounds.count < pkg->sounds.cap);
  uint32_t index = pkg->sounds.count++;
  pkg->sounds.id[index] = id;
  pkg->sounds.path[index] = path;
  pkg->sounds.volume[index] = volume;
  pkg->sounds.max_active[index] = max_active;
  return index;
}

API uint32_t package_def_append_music(package_def_t *pkg, char *id, char *path, float volume)
{
  assert(pkg->musics.count < pkg->musics.cap);
  uint32_t index = pkg->musics.count++;
  pkg->musics.id[index] = id;
  pkg->musics.path[index] = path;
  pkg->musics.volume[index] = volume;
  return index;
}

API void package_def_write_header(package_def_t *pkg, const char *name)
{
  char *output_path = str_format("src/gen/%s_resources.h", name);

  char output_dir[128];
  str_path_dirname(output_path, output_dir, sizeof(output_dir));

  if (!io_file_exists(output_dir)) {
    if (!io_mkdir(output_dir)) {
      log_error("cant create diretory: '%s'", output_dir);
      return;
    }
  }

  FILE *file = fopen(output_path, "w");
  if (!file) {
    log_error("cant open file: '%s'", output_path);
    fclose(file);
    return;
  }

  fprintf(file, "// NOTE: do not edit! this file is auto-generated.\n");

  fprintf(file, "\n#include \"core/defs.h\"\n");

  // textures: enum ids
  if (pkg->textures.count) {
    fprintf(file, "\ntypedef enum {\n");
    for (u32 i = 0; i < pkg->textures.count; i++) {
      fprintf(file, "  %s = %d, // %s\n", pkg->textures.id[i], i, pkg->textures.path[i]);
    }
    fprintf(file, "} _%s_resource_texture_id_t;\n", name);
  }

  // atlas: enum ids
  if (pkg->atlas.count) {
    fprintf(file, "\ntypedef enum {\n");
    for (u32 i = 0; i < pkg->atlas.count; i++) {
      fprintf(file, "  %s = %d, // %s\n", pkg->atlas.id[i], i, pkg->atlas.path[i]);
    }
    fprintf(file, "} _%s_resource_atlas_id_t;\n", name);
  }

  // fonts: enum ids
  if (pkg->fonts.count) {
    fprintf(file, "\ntypedef enum {\n");
    for (u32 i = 0; i < pkg->fonts.count; i++) {
      fprintf(file, "  %s = %d, // %s\n", pkg->fonts.id[i], i, pkg->fonts.path[i]);
    }
    fprintf(file, "} _%s_resource_fonts_id_t;\n", name);
  }

  // sounds: enum ids
  if (pkg->sounds.count) {
    fprintf(file, "\ntypedef enum {\n");
    for (u32 i = 0; i < pkg->sounds.count; i++) {
      fprintf(file, "  %s = %d, // %s\n", pkg->sounds.id[i], i, pkg->sounds.path[i]);
    }
    fprintf(file, "} _%s_resource_sounds_id_t;\n", name);
  }

  // musics: enum ids
  if (pkg->musics.count) {
    fprintf(file, "\ntypedef enum {\n");
    for (u32 i = 0; i < pkg->musics.count; i++) {
      fprintf(file, "  %s = %d, // %s\n", pkg->musics.id[i], i, pkg->musics.path[i]);
    }
    fprintf(file, "} _%s_resource_musics_id_t;\n", name);
  }

  // textures: get path by index
  if (pkg->textures.count) {
    fprintf(file, "\nAPI const char* %s_texture_path(u16 index)\n", name);
    fprintf(file, "{\n");
    fprintf(file, "  static char paths[][128] = {\n");
    for (u32 i = 0; i < pkg->textures.count; i++) {
      fprintf(file, "    [%s] = \"%s\",\n", pkg->textures.id[i], pkg->textures.path[i]);
    }
    fprintf(file, "  };\n");
    fprintf(file, "  return paths[index];\n");
    fprintf(file, "}\n");
  }

  // atlas: get path by index
  if (pkg->atlas.count) {
    fprintf(file, "\nAPI const char* %s_atlas_path(u16 index)\n", name);
    fprintf(file, "{\n");
    fprintf(file, "  static char paths[][128] = {\n");
    for (u32 i = 0; i < pkg->atlas.count; i++) {
      fprintf(file, "    [%s] = \"%s\",\n", pkg->atlas.id[i], pkg->atlas.path[i]);
    }
    fprintf(file, "  };\n");
    fprintf(file, "  return paths[index];\n");
    fprintf(file, "}\n");
  }

  // fonts: get path by index
  if (pkg->fonts.count) {
    fprintf(file, "\nAPI const char* %s_font_path(u16 index)\n", name);
    fprintf(file, "{\n");
    fprintf(file, "  static char paths[][128] = {\n");
    for (u32 i = 0; i < pkg->fonts.count; i++) {
      fprintf(file, "    [%s] = \"%s\",\n", pkg->fonts.id[i], pkg->fonts.path[i]);
    }
    fprintf(file, "  };\n");
    fprintf(file, "  return paths[index];\n");
    fprintf(file, "}\n");
  }

  // sounds: get path by index
  if (pkg->sounds.count) {
    fprintf(file, "\nAPI const char* %s_sound_path(u16 index)\n", name);
    fprintf(file, "{\n");
    fprintf(file, "  static char paths[][128] = {\n");
    for (u32 i = 0; i < pkg->sounds.count; i++) {
      fprintf(file, "    [%s] = \"%s\",\n", pkg->sounds.id[i], pkg->sounds.path[i]);
    }
    fprintf(file, "  };\n");
    fprintf(file, "  return paths[index];\n");
    fprintf(file, "}\n");
  }

  // musics: get path by index
  if (pkg->musics.count) {
    fprintf(file, "API const char* %s_music_path(u16 index)\n", name);
    fprintf(file, "{\n");
    fprintf(file, "  static char paths[][128] = {\n");
    for (u32 i = 0; i < pkg->musics.count; i++) {
      fprintf(file, "    [%s] = \"%s\",\n", pkg->musics.id[i], pkg->musics.path[i]);
    }
    fprintf(file, "  };\n");
    fprintf(file, "  return paths[index];\n");
    fprintf(file, "}\n\n");
  }

  fclose(file);
}

API void package_def_make(package_def_t *def, package_t *out, arena_t *arena)
{
  mem_set_zero(out, sizeof(package_t));
  int data_size;

  out->resources.textures = arena_push(arena, resource_texture_t, def->textures.count);
  out->count.textures = def->textures.count;
  for (u32 i = 0; i < def->textures.count; i++) {
    data_size = 0;
    char *path = def->textures.path[i];
    u8 *buffer = io_load_file_data(path, &data_size, arena);
    out->resources.textures[i] = (resource_texture_t){
      .buffer = buffer,
      .size = data_size,
      .ext = str_path_file_extension(path),
    };
  }

  out->resources.atlas = arena_push(arena, resource_atlas_t, def->atlas.count);
  out->count.atlas = def->atlas.count;
  for (u32 i = 0; i < def->atlas.count; i++) {
    data_size = 0;
    char *path = def->atlas.path[i];
    u8 *buffer = io_load_file_data(path, &data_size, arena);
    out->resources.atlas[i] = (resource_atlas_t){
      .buffer = buffer,
      .size = data_size,
      .ext = str_path_file_extension(path),
      .cell_size = def->atlas.cell_size[i],
    };
  }

  out->resources.fonts = arena_push(arena, resource_font_t, def->fonts.count);
  out->count.fonts = def->fonts.count;
  for (u32 i = 0; i < def->fonts.count; i++) {
    data_size = 0;
    char *path = def->fonts.path[i];
    u8 *buffer = io_load_file_data(path, &data_size, arena);
    out->resources.fonts[i] = (resource_font_t){
      .buffer = buffer,
      .size = data_size,
      .ext = str_path_file_extension(path),
    };
  }

  out->resources.sounds = arena_push(arena, resource_sound_t, def->sounds.count);
  out->count.sounds = def->sounds.count;
  for (u32 i = 0; i < def->sounds.count; i++) {
    data_size = 0;
    char *path = def->sounds.path[i];
    u8 *buffer = io_load_file_data(path, &data_size, arena);
    out->resources.sounds[i] = (resource_sound_t){
      .buffer = buffer,
      .size = data_size,
      .ext = str_path_file_extension(path),
      .volume = def->sounds.volume[i],
      .max_active = def->sounds.max_active[i],
    };
  }

  out->resources.musics = arena_push(arena, resource_music_t, def->musics.count);
  out->count.musics = def->musics.count;
  for (u32 i = 0; i < def->musics.count; i++) {
    data_size = 0;
    char *path = def->musics.path[i];
    u8 *buffer = io_load_file_data(path, &data_size, arena);
    out->resources.musics[i] = (resource_music_t){
      .buffer = buffer,
      .size = data_size,
      .ext = str_path_file_extension(path),
      .volume = def->musics.volume[i],
    };
  }
}

API void package_write(package_t *pkg, const char *name, arena_t *arena)
{
  char *output_path = str_format("resources/packages/%s.pkg", name);

  char output_dir[128];
  str_path_dirname(output_path, output_dir, sizeof(output_dir));

  if (!io_file_exists(output_dir)) {
    if (!io_mkdir(output_dir)) {
      log_error("cant create diretory: '%s'", output_dir);
      return;
    }
  }

  package_header_t header = { 0 };
  header.magic  = PACKAGE_MAGIC;
  header.flags  = PACKAGE_COMPRESS ? PACKAGE_FLAG_COMPRESSED : PACKAGE_FLAG_NONE;
  header.size   = 0;
  header.counts[RESOURCE_TYPE_TEXTURE] = pkg->count.textures;
  header.counts[RESOURCE_TYPE_ATLAS]   = pkg->count.atlas;
  header.counts[RESOURCE_TYPE_FONT]    = pkg->count.fonts;
  header.counts[RESOURCE_TYPE_SOUND]   = pkg->count.sounds;
  header.counts[RESOURCE_TYPE_MUSIC]   = pkg->count.musics;

  // layout per resource: u32 size + u8 ext[4] + type extra + raw data
  size_t total = sizeof(package_header_t);
  // for (u16 i = 0; i < pkg->count.textures; i++) total += 8 + pkg->textures[i].size;
  for (u16 i = 0; i < pkg->count.textures; i++) {
    total += sizeof(u32) + sizeof(char) * 4 +  pkg->resources.textures[i].size;
  }
  // for (u16 i = 0; i < pkg->count.atlas;    i++) total += 16 + pkg->atlas[i].size;    // + cell_size vec[2]
  for (u16 i = 0; i < pkg->count.atlas; i++) {
    total += sizeof(u32) + sizeof(char) * 4 + sizeof(vec2_t) + pkg->resources.atlas[i].size; 
  }

  // for (u16 i = 0; i < pkg->count.fonts;    i++) total += 8 + pkg->fonts[i].size;
  for (u16 i = 0; i < pkg->count.fonts; i++) {
    total += sizeof(u32) + sizeof(char) * 4 + pkg->resources.fonts[i].size;
  }

  // for (u16 i = 0; i < pkg->count.sounds;   i++) total += 16 + pkg->sounds[i].size;   // + volume + max_active + pad
  for (u16 i = 0; i < pkg->count.sounds; i++) {
    total += sizeof(u32) + sizeof(char) * 4 + sizeof(float) + sizeof(u8) + pkg->resources.sounds[i].size;
  }

  // for (u16 i = 0; i < pkg->count.musics;   i++) total += 12 + pkg->musics[i].size;   // + volume
  for (u16 i = 0; i < pkg->count.musics; i++) {
    total += sizeof(u32) + sizeof(char) * 4 + sizeof(float) + pkg->resources.musics[i].size;
  }

  header.size = (u32)total;

  u8 *buffer = arena_push(arena, u8, total);
  size_t offset = 0;

  mem_copy(&header, buffer, sizeof(package_header_t));
  offset += sizeof(package_header_t);

  for (u16 i = 0; i < pkg->count.textures; i++) {
    mem_copy(&pkg->resources.textures[i].size, buffer + offset, sizeof(u32)); offset += sizeof(u32);
    mem_set_zero(buffer + offset, 4);
    mem_copy((void *)pkg->resources.textures[i].ext, buffer + offset, strlen(pkg->resources.textures[i].ext) > 4 ? 4 : strlen(pkg->resources.textures[i].ext)); offset += 4;
    mem_copy(pkg->resources.textures[i].buffer, buffer + offset, pkg->resources.textures[i].size); offset += pkg->resources.textures[i].size;
  }
  for (u16 i = 0; i < pkg->count.atlas; i++) {
    mem_copy(&pkg->resources.atlas[i].size, buffer + offset, sizeof(u32)); offset += sizeof(u32);
    mem_set_zero(buffer + offset, 4);
    mem_copy((void *)pkg->resources.atlas[i].ext, buffer + offset, strlen(pkg->resources.atlas[i].ext) > 4 ? 4 : strlen(pkg->resources.atlas[i].ext)); offset += 4;
    mem_copy(&pkg->resources.atlas[i].cell_size, buffer + offset, 2 * sizeof(float)); offset += 2 * sizeof(float);
    mem_copy(pkg->resources.atlas[i].buffer, buffer + offset, pkg->resources.atlas[i].size); offset += pkg->resources.atlas[i].size;
  }
  for (u16 i = 0; i < pkg->count.fonts; i++) {
    mem_copy(&pkg->resources.fonts[i].size, buffer + offset, sizeof(u32)); offset += sizeof(u32);
    mem_set_zero(buffer + offset, 4);
    mem_copy((void *)pkg->resources.fonts[i].ext, buffer + offset, strlen(pkg->resources.fonts[i].ext) > 4 ? 4 : strlen(pkg->resources.fonts[i].ext)); offset += 4;
    mem_copy(pkg->resources.fonts[i].buffer, buffer + offset, pkg->resources.fonts[i].size); offset += pkg->resources.fonts[i].size;
  }
  for (u16 i = 0; i < pkg->count.sounds; i++) {
    mem_copy(&pkg->resources.sounds[i].size, buffer + offset, sizeof(u32)); offset += sizeof(u32);
    mem_set_zero(buffer + offset, 4);
    mem_copy((void *)pkg->resources.sounds[i].ext, buffer + offset, strlen(pkg->resources.sounds[i].ext) > 4 ? 4 : strlen(pkg->resources.sounds[i].ext)); offset += 4;
    mem_copy(&pkg->resources.sounds[i].volume, buffer + offset, sizeof(float)); offset += sizeof(float);
    mem_copy(&pkg->resources.sounds[i].max_active, buffer + offset, sizeof(u8)); offset += sizeof(u8);// + 3; // pad
    mem_copy(pkg->resources.sounds[i].buffer, buffer + offset, pkg->resources.sounds[i].size); offset += pkg->resources.sounds[i].size;
  }
  for (u16 i = 0; i < pkg->count.musics; i++) {
    mem_copy(&pkg->resources.musics[i].size, buffer + offset, sizeof(u32)); offset += sizeof(u32);
    mem_set_zero(buffer + offset, 4);
    mem_copy((void *)pkg->resources.musics[i].ext, buffer + offset, strlen(pkg->resources.musics[i].ext) > 4 ? 4 : strlen(pkg->resources.musics[i].ext)); offset += 4;
    mem_copy(&pkg->resources.musics[i].volume, buffer + offset, sizeof(float)); offset += sizeof(float);
    mem_copy(pkg->resources.musics[i].buffer, buffer + offset, pkg->resources.musics[i].size); offset += pkg->resources.musics[i].size;
  }

#if PACKAGE_COMPRESS
  // keep the header plain, compress only the entries
  int comp_data_size = 0;
  u8 *comp_data = io_compress_data(buffer + sizeof(package_header_t), (int)(total - sizeof(package_header_t)), &comp_data_size, arena);

  size_t out_size = sizeof(package_header_t) + (size_t)comp_data_size;
  u8 *out = arena_push(arena, u8, out_size);
  mem_copy(buffer, out, sizeof(package_header_t));
  mem_copy(comp_data, out + sizeof(package_header_t), comp_data_size);
  io_save_file_data(output_path, out, (int)out_size);
#else
  io_save_file_data(output_path, buffer, (int)total);
#endif
}

API void package_read(package_t *pkg, const char *name, arena_t *resource_arena, arena_t *handler_arena)
{
  const char *path = str_format("resources/packages/%s.pkg", name);

  int data_size = 0;
  u8 *buffer = io_load_file_data(path, &data_size, resource_arena);

  if (!data_size || !buffer || data_size < (int)sizeof(package_header_t)) {
    log_error("package: invalid file size for '%s'", path);
    return;
  }

  package_header_t header;
  mem_copy(buffer, &header, sizeof(package_header_t));

  if (header.magic != PACKAGE_MAGIC) {
    log_error("package: '%s' is not a valid package (bad magic)", path);
    return;
  }

  u8 *payload = buffer + sizeof(package_header_t);
  int payload_size = data_size - (int)sizeof(package_header_t);

  if ((header.flags & PACKAGE_FLAG_COMPRESSED)) {
    payload = io_decompress_data(payload, payload_size, &payload_size, resource_arena);
    if (!payload || payload_size <= 0) {
      log_error("package: failed to decompress '%s'", path);
      return;
    }
    if ((u32)payload_size != header.size - sizeof(package_header_t)) {
      log_error("package: '%s' size mismatch after decompress", path);
      return;
    }
  }

  size_t offset = 0;

  mem_set_zero(pkg, sizeof(package_t));
  pkg->count.textures = header.counts[RESOURCE_TYPE_TEXTURE];
  pkg->count.atlas    = header.counts[RESOURCE_TYPE_ATLAS];
  pkg->count.fonts    = header.counts[RESOURCE_TYPE_FONT];
  pkg->count.sounds   = header.counts[RESOURCE_TYPE_SOUND];
  pkg->count.musics   = header.counts[RESOURCE_TYPE_MUSIC];

  pkg->resources.textures = arena_push(resource_arena, resource_texture_t, pkg->count.textures);
  pkg->resources.atlas    = arena_push(resource_arena, resource_atlas_t,    pkg->count.atlas);
  pkg->resources.fonts    = arena_push(resource_arena, resource_font_t,     pkg->count.fonts);
  pkg->resources.sounds   = arena_push(resource_arena, resource_sound_t,    pkg->count.sounds);
  pkg->resources.musics   = arena_push(resource_arena, resource_music_t,    pkg->count.musics);

  pkg->handlers.textures = arena_push(handler_arena, texture_t, pkg->count.textures);
  pkg->handlers.atlas = arena_push(handler_arena, atlas_t, pkg->count.atlas);
  pkg->handlers.fonts = arena_push(handler_arena, font_t, pkg->count.fonts);
  pkg->handlers.sounds = arena_push(handler_arena, sound_t, pkg->count.sounds);
  pkg->handlers.musics = arena_push(handler_arena, music_t, pkg->count.musics);

  u32 size;
  char *ext = arena_push(resource_arena, char, 5);
  for (u16 i = 0; i < pkg->count.textures; i++) {
    mem_copy(payload + offset, &size, sizeof(u32)); offset += sizeof(u32);
    char *ext = arena_push(resource_arena, char, 5);
    mem_copy(payload + offset, ext, 4); ext[4] = '\0'; offset += 4;
    pkg->resources.textures[i] = (resource_texture_t){
      .buffer = payload + offset,
      .ext = ext,
      .size = size,
    };
    offset += size;
  }
  for (u16 i = 0; i < pkg->count.atlas; i++) {
    mem_copy(payload + offset, &size, sizeof(u32)); offset += sizeof(u32);
    mem_copy(payload + offset, ext, 4); ext[4] = '\0'; offset += 4;
    vec2_t cell_size;
    mem_copy(payload + offset, &cell_size, sizeof(cell_size)); offset += sizeof(cell_size);
    pkg->resources.atlas[i] = (resource_atlas_t){
      .buffer = payload + offset,
      .ext = ext,
      .cell_size = cell_size,
      .size = size,
    };
    offset += size;
  }
  for (u16 i = 0; i < pkg->count.fonts; i++) {
    mem_copy(payload + offset, &size, sizeof(u32)); offset += sizeof(u32);
    mem_copy(payload + offset, ext, 4); ext[4] = '\0'; offset += 4;
    pkg->resources.fonts[i] = (resource_font_t){
      .buffer = payload + offset,
      .ext = ext,
      .size = size,
    };
    offset += size;
  }
  for (u16 i = 0; i < pkg->count.sounds; i++) {
    mem_copy(payload + offset, &size, sizeof(u32)); offset += sizeof(u32);
    mem_copy(payload + offset, ext, 4); ext[4] = '\0'; offset += 4;
    float volume;
    u8 max_active;
    mem_copy(payload + offset, &volume, sizeof(float)); offset += sizeof(float);
    mem_copy(payload + offset, &max_active, sizeof(u8)); offset += sizeof(u8);// + 3; // pad
    pkg->resources.sounds[i] = (resource_sound_t){
      .buffer = payload + offset,
      .ext = ext,
      .volume = volume,
      .size = size,
      .max_active = max_active,
    };
    offset += size;
  }
  for (u16 i = 0; i < pkg->count.musics; i++) {
    mem_copy(payload + offset, &size, sizeof(u32)); offset += sizeof(u32);
    char *ext = arena_push(resource_arena, char, 5);
    mem_copy(payload + offset, ext, 4); ext[4] = '\0'; offset += 4;
    float volume;
    mem_copy(payload + offset, &volume, sizeof(float)); offset += sizeof(float);
    pkg->resources.musics[i] = (resource_music_t){
      .buffer = payload + offset,
      .ext = ext,
      .volume = volume,
      .size = size,
    };
    offset += size;
  }

  if (offset != (size_t)payload_size) {
    log_warn("package: '%s' consumed %zu of %d bytes", path, offset, payload_size);
  }

  printn("[package_read]");
  printn(" - data_size: %d", data_size);
  printn(" - header size: %d", sizeof(package_header_t));
  printn(" - textures : %d", header.counts[RESOURCE_TYPE_TEXTURE]);
  printn(" - atlases  : %d", header.counts[RESOURCE_TYPE_ATLAS]);
  printn(" - fonts    : %d", header.counts[RESOURCE_TYPE_FONT]);
  printn(" - sounds   : %d", header.counts[RESOURCE_TYPE_SOUND]);
  printn(" - musics   : %d", header.counts[RESOURCE_TYPE_MUSIC]);
}


