#pragma once

#include "core/arena.h"
#include "core/defs.h"
#include "core/package.h"

typedef struct {
  arena_t *arena;
  arena_t *scene_arena;
  arena_t *package_handler_arena;
  arena_t *package_resource_arena;
  engine_state_t state;

  u8                     scene;
  u8                     scene_next;
  void                  *scene_state;
  scene_transition_t     scene_transition;
  
  screen_size_t screen_size;
  float delta_time;
  package_t *packages[ENGINE_MAX_PACKAGES];

} engine_t;

GLOBAL engine_t *g_engine;

API engine_t      *engine_ptr();
API arena_t       *engine_scene_arena();
API float          engine_delta_time();
API bool           engine_paused();
API void           engine_quit();
API void           engine_emit_hot_sync();
API void           engine_pause(bool paused);
API void           engine_set_scene(u8 type);
API void           engine_init();
API void           engine_fini();
API void           engine_start();
API void           engine_step();
API void           engine_process();
API void           engine_draw();
API bool           engine_package_load(char *name);
API package_t     *engine_package_core();


API bool engine_scene_entering();
API bool engine_scene_exiting();
API void engine_scene_init();
API void engine_scene_process();
API void engine_scene_draw();
API void engine_scene_free();
API void engine_scene_sync(u8 scene, sync_signal_type_t signal);


