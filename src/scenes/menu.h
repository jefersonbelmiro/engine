#pragma once

#include "backend/api.h"
#include "backend/codes.h"
#include "core/engine.h"
#include "core/arena.h"

typedef struct {
  char pad;
} menu_scene_t;

API menu_scene_t* menu_scene_init()
{
  arena_t *arena = engine_scene_arena();
  menu_scene_t *scene = arena_push(arena, menu_scene_t, 1);

  vec2_t vec = {20.33, 99.666923992};
  printn("menu_scene_init");
  printn(" - platform: %d", PLATFORM);
  printn(" - vec: (x:%g, y:%g)", vec.x, vec.y);
  return scene;
}

API bool menu_scene_exiting(menu_scene_t *scene)
{
  (void) scene;
  return true;
}

API bool menu_scene_entering(menu_scene_t *scene)
{                  
  (void) scene;
  return true;
}

API void menu_scene_sync(menu_scene_t *scene, sync_signal_type_t signal)
{
  (void) scene;
  switch (signal) {
    case SYNC_SIGNAL_WINDOW_RESIZED: {
      //
      break;
    }
    case SYNC_SIGNAL_ON_EXIT: {
      //
      break;
    }

    default: break;
  }
}

API void menu_scene_free(menu_scene_t *scene)
{
  (void) scene;
}

API void menu_scene_process(menu_scene_t *scene, float delta)
{
  if (be_key_down(BE_KEY_ESCAPE)) {
    engine_quit();
  }
  (void) scene; (void) delta;
}

API void menu_scene_draw(menu_scene_t *scene)
{
  (void) scene;
}
