#pragma once

#include "core/defs.h"

typedef struct {
  char *name;
  char *version;
  char *binary;
  char *lib_raylib_path;
  char *lib_emsdk_path;
} project_t;

GLOBAL project_t g_project = {
  .name = "main",
  .version = "0.0.1",
  .binary = "main",
  .lib_raylib_path = "~/dev/libs/raylib",
  .lib_emsdk_path = "~/dev/libs/emsdk",
};

API project_t *project_ptr()
{
  return &g_project;
}
