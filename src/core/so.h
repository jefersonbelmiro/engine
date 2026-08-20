#pragma once

#include "core/defs.h"
#include <assert.h>
#include <stdlib.h>

#ifndef SO_TEXT_BUFFER_LENGTH
  #define SO_TEXT_BUFFER_LENGTH 256
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
