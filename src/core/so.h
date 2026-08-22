#pragma once

#include "core/defs.h"
#include "core/string.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifndef SO_TEXT_BUFFER_LENGTH
  #define SO_TEXT_BUFFER_LENGTH 512
#endif
#ifndef SO_PATH_MAX
  #define SO_PATH_MAX 1024
#endif

#if defined(_WIN32) || defined(_WIN64)
  #include <process.h>
  #define execv _execv
#else
  #include <unistd.h>
#endif

API bool so_exec(const char *format, ...)
{
  assert(format);

  static char buffer[SO_TEXT_BUFFER_LENGTH];

  va_list args;
  va_start(args, format);
  int required_byte_count = vsnprintf(buffer, SO_TEXT_BUFFER_LENGTH, format, args);
  if (required_byte_count >= SO_TEXT_BUFFER_LENGTH) {
    printn("[ERROR] cmd(%d chars) >= SO_TEXT_BUFFER_LENGTH(%d chars)", required_byte_count, SO_TEXT_BUFFER_LENGTH);
    return false;
  }
  va_end(args);

  int error = system(buffer);
  if (error) {
    log_error("so_exec error: %s", buffer);
    return false;
  }

  return true;
}

API int so_execv(const char *path, char *const argv[])
{
  int error = execv(path, argv);
  if (error) {
    log_error("so_execv error %s", path);
    return error;
  }

  return 1;
}

API void so_resolve_home(const char *path, char *resolved_path) 
{
  char temp_path[SO_PATH_MAX] = {'\0'};

#if defined(_WIN32) || defined(_WIN64)
  if (path[0] == '~' && (path[1] == '\\' || path[1] == '\0')) {
    const char *home = getenv("USERPROFILE");
    if (!home)
      return;
    snprintf(temp_path, sizeof(temp_path), "%s%s", home, path + 1);
  } else {
    snprintf(temp_path, sizeof(temp_path), "%s", path);
  }
  str_trim_end(temp_path, '\\');
#else
  if (path[0] == '~' && (path[1] == '/' || path[1] == '\0')) {
    const char *home = getenv("HOME");
    if (!home)
      return;
    snprintf(temp_path, sizeof(temp_path), "%s%s", home, path + 1);
  } else {
    snprintf(temp_path, sizeof(temp_path), "%s", path);
  }
  str_trim_end(temp_path, '\\');
#endif

  strcpy(resolved_path, temp_path);
}

API char *so_exec_output(arena_t *arena, const char *format, ...)
{
  assert(format);
  assert(arena);

  static char buffer[SO_TEXT_BUFFER_LENGTH];

  va_list args;
  va_start(args, format);
  int required_byte_count = vsnprintf(buffer, SO_TEXT_BUFFER_LENGTH, format, args);
  va_end(args);
  if (required_byte_count >= SO_TEXT_BUFFER_LENGTH) {
    printn("[ERROR] cmd(%d chars) >= SO_TEXT_BUFFER_LENGTH(%d chars)", required_byte_count, SO_TEXT_BUFFER_LENGTH);
    return NULL;
  }

  FILE *pipe = popen(buffer, "r");
  if (!pipe) {
    log_error("so_exec_output error: %s", buffer);
    return NULL;
  }

  size_t capacity = KB(16);
  char *output = arena_push(arena, char, capacity);
  if (!output) {
    pclose(pipe);
    return NULL;
  }

  size_t len = 0;
  size_t n = 0;
  while (len + 1 < capacity &&
    (n = fread(output + len, 1, capacity - len - 1, pipe)) > 0) {
    len += n;
  }
  output[len] = '\0';

  int error = pclose(pipe);
  if (error) {
    log_error("so_exec_output error: %s", buffer);
    return NULL;
  }

  return output;
}
