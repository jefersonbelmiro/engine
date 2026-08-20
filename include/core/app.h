#pragma once

#include "core/arena.h"
#include "core/defs.h"

typedef struct {
  arena_t *arena;
  arena_t *scene_arena;
  app_state_t state;

  u8                     scene;
  u8                     scene_next;
  void                  *scene_state;
  app_scene_transition_t scene_transition;
  
  screen_size_t screen_size;

} app_t;

GLOBAL app_t *g_app;

API app_t         *app_ptr();
API screen_size_t *app_screen_size();
API arena_t       *app_scene_arena();
API bool           app_paused();
API void           app_quit();
API void           app_emit_hot_sync();
API void           app_pause(bool paused);
API void           app_set_scene(scene_type_t type);
API void           app_init(void);
API void           app_fini(void);
API void           app_start(void);
API void           app_process(float delta);
API void           app_draw(void);


