#pragma once

#include "core/defs.h"

static tool_t tools[] = {
  { 
    .name = "core_pack",
    .description = "create core resource package",
    .source_path = "engine/tools/core_pack.c",
  },
  { 
    .name = "build_linux",
    .description = "build plataform target linux",
    .source_path = "engine/tools/build_linux.c",
  },
  { 
    .name = "build_web",
    .description = "build plataform target web",
    .source_path = "engine/tools/build_web.c",
  },
  { 
    .name = "gen_compile_flags",
    .description = "crate compile_flags.txt from project.cfg",
    .source_path = "engine/tools/gen_compile_flags.c",
  },
};

API tool_array_t* tools_entries()
{
  static tool_array_t array = {
    .array = tools,
    .count = countof(tools)
  };
  return &array;
}
