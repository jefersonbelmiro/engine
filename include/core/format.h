#pragma once

#include "defs.h"
#include "string.h"

API const char * format_number(float value)
{
  if (value == 0) return "";
  float result = value;
  char *format = "%0.f";

  if (value > 1000) {
    format = "%0.1fK";
    result = value / 1000.0f;
  }

  return format_text(format, result);
}
