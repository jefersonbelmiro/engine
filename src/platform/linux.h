#pragma once

#include "platform/api.h"

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
  return false;
}
