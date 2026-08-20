#pragma once

#include "core/arena.h"
#include "core/defs.h"
#include "ext/sdefl.h"
#include "ext/sinfl.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <direct.h>
    #define _make_dir(path) _mkdir(path)
#else
    #define _make_dir(path) mkdir(path, 0755)
#endif

API bool io_file_exists(const char *path) 
{
  struct stat st;
  if (stat(path, &st) == 0) {
    return true;
  }
  return false;
}

API bool io_dir_exists(const char *path) 
{
  struct stat st;
  if (stat(path, &st) == 0) {
    return true;
  }
  return false;
}

API size_t io_file_size(const char *path)
{
  struct stat sb;
  if (stat(path, &sb) == 0) {
    return sb.st_size;
  }
  return 0;
}

API bool io_mkdir(const char *path) 
{
  if (_make_dir(path) == 0) {
    return true; 
  }
  return false;
}

API bool io_mkdir_recursive(const char *path) 
{
  char lpath[128];
  u16 len = strlen(path);
  u16 index = 0;
  strcpy(lpath, path);

  if (path[len - 1] != '/') {
    lpath[len] = '/';
    lpath[len + 1] = '\0';
    len++;
  }

  assert(len <= 128);

  while (index < len) {
    if (lpath[index] == '/') {
      lpath[index] = '\0';
      if (!io_dir_exists(lpath) && !io_mkdir(lpath)) {
        log_error("io: [%s] failed to create directory", lpath);
        return false;
      }
      lpath[index] = '/';
    }
    index++;
  }
  return true;
}

API unsigned char *io_load_file_data(const char *path, int *data_size, arena_t *arena)
{
  unsigned char *data = NULL;
  *data_size = 0;

  FILE *file = fopen(path, "rb");
  if (!file) {
    log_error("io: [%s] failed to open file", path);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (file == 0) {
    log_error("io: [%s] failed to read file", path);
    goto teardown;
  }

  data = arena_push(arena, unsigned char, size);

  if (!data) {
    log_error("io: [%s] failed to allocate in arena size: %d", path, size);
    goto teardown;
  }

  long read_count = fread(data, sizeof(unsigned char), size, file);

  if (read_count != size) {
    log_warn("io: [%s] file partially loaded (%i bytes out of %i)", path, read_count, size);
  }

  *data_size = read_count;

  fclose(file);
  return data;

teardown:
    fclose(file);
    return NULL;
}

API bool io_save_file_data(const char *path, const void *data, int data_size)
{
  FILE *file = fopen(path, "wb");
  if (!file) {
    log_error("io: [%s] failed to open file", path);
    return false;
  }

  int count = fwrite(data, sizeof(unsigned char), data_size, file);
  if (count == 0) {
    log_error("io: [%s] failed to wite file", path);
    goto teardown;
  }
  else if (count != data_size) {
    log_error("io: [%s] file partially written", path);
    goto teardown;
  }

  return fclose(file) == 0;

teardown:
    fclose(file);
    return false;
}

// compress data (deflate algorithm)
API unsigned char *io_compress_data(const unsigned char *data, int data_size, int *comp_data_size, arena_t *arena)
{
  unsigned char *comp_data = NULL;

  int bounds = sdefl_bound(data_size);
  comp_data = (unsigned char *)arena_push_stride(arena, char, 1, bounds);

  size_t arena_offset = arena->offset;

  // compress data and generate a valid DEFLATE stream
  struct sdefl *sdefl = (struct sdefl *)arena_push(arena, struct sdefl, 1);

  *comp_data_size = sdeflate(sdefl, comp_data, data, data_size, 8);   // Compression level 8, same as stbiw

  arena_restore(arena, arena_offset); // discart sdefl
  // mem_free(sdefl);

  return comp_data;
}

// decompress data (DEFLATE algorithm)
unsigned char *io_decompress_data(const unsigned char *comp_data, int comp_data_size, int *data_size, arena_t *arena)
{
#ifndef MAX_DECOMPRESSION_SIZE
  #define MAX_DECOMPRESSION_SIZE 8 // Maximum size allocated for decompression in MB
#endif

  size_t arena_offset = arena->offset;

  // Decompress data from a valid DEFLATE stream
  unsigned char *data = (unsigned char *)arena_push_stride(arena, char, 1, MB(MAX_DECOMPRESSION_SIZE));
  int size = sinflate(data, MB(MAX_DECOMPRESSION_SIZE), comp_data, comp_data_size);

  if (size <= 0 || size > MB(MAX_DECOMPRESSION_SIZE)) {
    arena_restore(arena, arena_offset);
    *data_size = 0;
    return NULL;
  }

  arena_restore(arena, arena_offset + size);

  *data_size = size;

  return data;
}

