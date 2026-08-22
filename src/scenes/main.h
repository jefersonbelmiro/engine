#pragma once

#include "core/engine.h"
#include "core/defs.h"
#include "core/arena.h"

typedef struct {
  char pad;
} main_scene_t;

API main_scene_t* main_scene_init()
{
  arena_t *arena = engine_scene_arena();
  main_scene_t *scene = arena_push(arena, main_scene_t, 1);
  return scene;
}

API bool main_scene_exiting(UNUSED main_scene_t *scene)
{
  return true;
}

API bool main_scene_entering(UNUSED main_scene_t *scene)
{
  return true;
}

API void main_scene_sync(main_scene_t *scene, sync_signal_type_t signal)
{
  (void) scene;
  switch (signal) {
    case SYNC_SIGNAL_WINDOW_RESIZED:
      // 
    break;
    case SYNC_SIGNAL_ON_EXIT: {
      // 
      break;
    }

    default: break;
  }
}

API void main_scene_free(main_scene_t *scene)
{
  (void) scene;
}

API void main_scene_process(main_scene_t *scene, float delta)
{
  (void) scene; (void) delta;
}

API void main_scene_draw(main_scene_t *scene)
{
  (void) scene;
}
