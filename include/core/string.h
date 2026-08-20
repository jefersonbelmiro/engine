#pragma once

#include "core/arena.h"
#include "core/mem.h"
#include "defs.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// size of internal static buffers used on some functions:
#ifndef MAX_TEXT_BUFFER_LENGTH
  #define MAX_TEXT_BUFFER_LENGTH 256
#endif

// maximum number of static buffers for text formatting
#ifndef MAX_TEXTFORMAT_BUFFERS
  #define MAX_TEXTFORMAT_BUFFERS 4
#endif

#if defined(_WIN32) || defined(_WIN64)
  #define PATH_SEP '\\'
#else
  #define PATH_SEP '/'
#endif

// @note: we don't have to worry about str being shorter than pre because
// according to the c standard (7.21.4.4/2):
//    The strncmp function compares not more than n characters (characters that
//    follow a null character are not compared) from the array pointed to by s1
//    to the array pointed to by s2."
API bool str_start_with(char *str, char *pre)
{
  return strncmp(pre, str, strlen(pre)) == 0;
}

API bool str_start_with_n(char *str, char *pre, size_t pre_length)
{
  return strncmp(pre, str, pre_length) == 0;
}

API bool str_eq(char *str, char *pre)
{
  return strcmp(pre, str) == 0;
}

API bool char_is_alpha_num(char c) 
{
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
         (c >= 'a' && c <= 'z');
}

API bool char_is_empty(char c) 
{
  return c == ' ' || c == '\t' || c == '\n';
}

API bool char_is_space(char c)
{
  return c == ' ' || c == '\t';
}

API void str_trim_start(char *string, char delim)
{
  u32 start = 0;
  while (string[start] != '\0' && string[start] == delim) {
    start++;
  }
  if (start > 0) {
    u32 len = strlen(&string[start]);
    mem_move(string, &string[start], len + 1);
  }
}

API void str_trim_end(char *string, char delim)
{
  u32 len = strlen(string);
  if (len == 0) return;
  u32 end = len - 1;
  while (end >= 0 && string[end] == delim) {
    end--;
  }
  string[end + 1] = 0x0;
}

API void str_trim(char *string, char delim)
{
  str_trim_end(string, delim);
  str_trim_start(string, delim);
}

API void str_slugify(char *string)
{
  while (*string != '\0') {
    if (!char_is_alpha_num(*string)) {
      *string = '-';
    }
    string++;
  }
}

API char *str_dup(char *source, arena_t *arena)
{
  size_t len = strlen(source) + 1;
  char *result = arena_push(arena, char, len);
  strcpy(result, source);
  result[len] = 0x0;
  return result;
}

API const char* str_format(const char *format, ...)
{
  static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH] = { 0 };
  static int index = 0;

  char *buffer = buffers[index];
  memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH); // Clear buffer before using

  if (format != NULL) {
    va_list args;
    va_start(args, format);
    int required_byte_count = vsnprintf(buffer, MAX_TEXT_BUFFER_LENGTH, format, args);
    va_end(args);

    // If required_byte_count is larger than the MAX_TEXT_BUFFER_LENGTH, then overflow occurred
    if (required_byte_count >= MAX_TEXT_BUFFER_LENGTH) {
      // Inserting "..." at the end of the string to mark as truncated
      char *trunc_buffer = buffers[index] + MAX_TEXT_BUFFER_LENGTH - 4; // Adding 4 bytes = "...\0"
      snprintf(trunc_buffer, 4, "...");
      printf("[WARN] previous log messat truncated %d chars", required_byte_count - (int)sizeof(buffer) + 1);
    }

    index += 1;     // Move to next buffer for next function call
    if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;
  }

  return buffer;
}

API const char* str_path_filename(const char *path) 
{
  // find the last occurrence of the platform's separator
  const char *last_sep = strrchr(path, PATH_SEP);

  // if a separator was found, the filename starts right after it
  if (last_sep != NULL) {
    return last_sep + 1;
  }
  // If no separator was found, the path itself is the filename
  return path;
}

API void str_path_dirname(const char *path, char *dir_out, size_t max_len) 
{
  strncpy(dir_out, path, max_len);
  dir_out[max_len - 1] = '\0';

  char *last_sep = strrchr(dir_out, PATH_SEP);

  if (last_sep != NULL) {
    // if it is the root directory (e.g., "/" or "C:\"), keep the slash
    if (last_sep == dir_out) {
      *(last_sep + 1) = '\0';
    } else {
      *last_sep = '\0';
    }
  } else {
    strncpy(dir_out, ".", max_len);
  }
}

// - name_out: buffer to store the filename without extension
// - returns: pointer to the extension inside the original path (without the dot), or empty string ""
API const char* str_path_split_filename(const char *path, char *name_out, size_t max_len) 
{
  // 1. Find the start of the filename first
  const char *last_sep = strrchr(path, PATH_SEP);
  const char *filename = (last_sep != NULL) ? (last_sep + 1) : path;

  // 2. Find the last dot ONLY within the filename component
  const char *last_dot = strrchr(filename, '.');

  if (last_dot != NULL && last_dot != filename) {
    // Calculate the length of the filename without extension
    size_t name_len = last_dot - filename;
    if (name_len >= max_len) {
      name_len = max_len - 1;
    }

    // Copy just the name part
    strncpy(name_out, filename, name_len);
    name_out[name_len] = '\0';

    return last_dot + 1; // Returns "txt", "png", etc.
  }

  // Edge case: No extension found
  strncpy(name_out, filename, max_len);
  name_out[max_len - 1] = '\0';
  return ""; 
}

// helper to isolate filename start pointer
API const char* str_path_filename_start(const char *path) 
{
  const char *last_sep = strrchr(path, PATH_SEP);
  return (last_sep != NULL) ? (last_sep + 1) : path;
}

// get filename WITHOUT extension (copies to output buffer)
API void str_path_filename_no_ext(const char *path, char *name_out, size_t max_len) {
  const char *filename = str_path_filename_start(path);
  const char *last_dot = strrchr(filename, '.');

  // Only split if dot is found and it is not a hidden file dot (e.g., .gitignore)
  if (last_dot != NULL && last_dot != filename) {
    size_t name_len = last_dot - filename;
    if (name_len >= max_len) {
      name_len = max_len - 1;
    }
    strncpy(name_out, filename, name_len);
    name_out[name_len] = '\0';
  } else {
    // No extension found, copy full filename
    strncpy(name_out, filename, max_len);
    name_out[max_len - 1] = '\0';
  }
}

// get extension ONLY (returns pointer inside original string, or empty string "")
API const char* str_path_file_extension(const char *path) 
{
  const char *filename = str_path_filename_start(path);
  const char *last_dot = strrchr(filename, '.');

  if (last_dot != NULL && last_dot != filename) {
    return last_dot + 1; // Returns "png", "txt", etc.
  }
  return ""; // No extension
}
