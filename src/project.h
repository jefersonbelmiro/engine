#pragma once

#include "core/defs.h"

typedef struct {
  char *name;
  char *binary;
} project_t;

GLOBAL project_t g_project = {
  .name = "main",
  .binary = "main",
};
