#pragma once

#include "app/defs.h"
#include "core/app_op.h"
#include "backend/api.h"
#include "core/defs.h"

API void core_init()
{
  backend_init();
  app_init();
  app_scene_setup();
  app_start();
  backend_main();
}
