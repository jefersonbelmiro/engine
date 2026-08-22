#pragma once

#include "platform/api.h"
#include "raylib.h"

API void platform_init()
{
  (void)0;
}

API void platform_mark_ready() 
{
  (void)0;
}

API bool platform_is_mobile()
{
  return false;
}

API bool platform_has_touch()
{
  return false;
}

API bool platform_is_ready()
{
  return true;
}

API char *platform_binary_path()
{
  return (char*)GetApplicationDirectory();
}

API bool platform_save_file(const char *file_name, const void *data, const int data_size)
{
  char *base_directory = platform_binary_path();

  if (!DirectoryExists(base_directory)) {
    MakeDirectory(base_directory);
  }

  bool saved = SaveFileData(TextFormat("%s/%s", base_directory, file_name), data, data_size);

  return saved;
}

API unsigned char* platform_load_file(const char *file_name, int *data_size)
{
  const char *path = TextFormat("%s/%s", platform_binary_path(), file_name);
  unsigned char *buff = NULL;
  if (FileExists(path)) {
    buff = LoadFileData(path, data_size);
  }
  // @note: caller need to call unload
  // UnloadFileData(buff);
  return buff;
}

