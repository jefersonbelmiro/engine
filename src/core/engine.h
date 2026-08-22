#pragma once

#include "core/arena.h"
#include "core/defs.h"

typedef struct {
  arena_t *arena;
  arena_t *scene_arena;
  engine_state_t state;

  u8                     scene;
  u8                     scene_next;
  void                  *scene_state;
  scene_transition_t     scene_transition;
  
  screen_size_t screen_size;
  float delta_time;

} engine_t;

GLOBAL engine_t *g_engine;

API engine_t      *engine_ptr();
API arena_t       *engine_scene_arena();
API bool           engine_paused();
API void           engine_quit();
API void           engine_emit_hot_sync();
API void           engine_pause(bool paused);
API void           engine_set_scene(u8 type);
API void           engine_init(void);
API void           engine_fini(void);
API void           engine_start(void);
API void           engine_process(float delta);
API void           engine_draw(void);


