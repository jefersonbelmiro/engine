#pragma once

#define _GNU_SOURCE

#include "platform/api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

API void fs_init()
{
  (void)0;
}

API void fs_mark_ready()
{
  (void)0;
}

API bool fs_ready()
{
  return true;
}

API bool device_is_mobile()
{
  return false;
}

API bool device_has_touch()
{
  return false;
}

API char *fs_binary_path()
{
  static char path[1024];
  ssize_t n = readlink("/proc/self/exe", path, sizeof(path) - 1);
  if (n > 0) {
    path[n] = '\0';
    char *sep = strrchr(path, '/');
    if (sep) sep[1] = '\0';
  } else {
    strcpy(path, "./");
  }
  return path;
}

API bool fs_save_file(const char *file_name, const void *data, int data_size)
{
  char path[2048];
  snprintf(path, sizeof(path), "%s%s", fs_binary_path(), file_name);

  FILE *file = fopen(path, "wb");
  if (!file) {
    return false;
  }

  size_t written = fwrite(data, 1, (size_t)data_size, file);
  fclose(file);

  return (int)written == data_size;
}

API unsigned char *fs_load_file(const char *file_name, int *data_size)
{
  char path[2048];
  snprintf(path, sizeof(path), "%s%s", fs_binary_path(), file_name);

  *data_size = 0;

  FILE *file = fopen(path, "rb");
  if (!file) {
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (size <= 0) {
    fclose(file);
    return NULL;
  }

  unsigned char *buffer = (unsigned char *)malloc((size_t)size);
  if (!buffer) {
    fclose(file);
    return NULL;
  }

  size_t read_count = fread(buffer, 1, (size_t)size, file);
  fclose(file);

  *data_size = (int)read_count;
  return buffer;
}
