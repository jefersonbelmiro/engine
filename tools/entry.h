#pragma once

#include "core/defs.h"

typedef struct {
  char *name;
  char *description;
  char *source_path;
} tool_t;

typedef struct {
  tool_t *array;
  u16     count;
} tool_array_t;

static tool_t tools[] = {
  { 
    .name = "core_pack",
    .description = "create core resource package",
    .source_path = "tools/core_pack.c",
  },
  { 
    .name = "build_linux",
    .description = "build plataform target linux",
    .source_path = "tools/build_linux.c",
  }
};

API tool_array_t* tools_entries()
{
  static  tool_array_t array = {
    .array = tools,
    .count = countof(tools)
  };
  return &array;
}
