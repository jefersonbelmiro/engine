#pragma once

#include "app/scenes.h"
#include "backend/api.h"
#include "core/app.h"
#include "core/arena.h"
#include "core/defs.h"
#include "core/input.h"
#include "core/timer.h"
#include "core/tween.h"

API app_t* app_ptr()
{
  return g_app;
}

API bool app_paused()
{
  app_t *app = app_ptr();
  return app->state == APP_PAUSED;
}

API screen_size_t* app_screen_size()
{
  return &app_ptr()->screen_size;
}

API void app_set_scene(scene_type_t type)
{
  app_t *app = app_ptr();
  app->scene_next = type;
}

API arena_t* app_scene_arena()
{
  app_t *app = app_ptr();
  return app->scene_arena;
}

API void app_init(void)
{
  assert(!g_app);
  arena_t *arena = arena_create(APP_ARENA_SIZE, "app");
  app_t *app = arena_push_zero(arena, app_t, 1);
  g_app = app;

  input_init(arena);
  // resource_init(arena_create_sub(arena, resources_memory_size(), "resource"));
  // sound_init(arena_create_sub(arena, sound_memory_size(), "sound"));
  tween_init(arena_create_sub(arena, tween_memory_size(), "tween"));
  timer_init(arena_create_sub(arena, timer_memory_size(), "timer"));

  app->scene = SCENE_NONE;
  app->arena = arena;
  // keep scene arena to end, for cache locality(i think)
  app->scene_arena = arena_create_sub(arena, APP_SCENE_ARENA_SIZE, "scene");
}

API void app_fini()
{
  app_t *app = app_ptr();
  // resource_unload();
  arena_fini(app->arena);
  g_app = NULL;
}

API void app_start(void)
{
  // resource_start();
  // sound_start();

  app_t *app = app_ptr();
  app->state = APP_RUNNING;
}

API void app_quit()
{
  app_t *app = app_ptr();
  if (app->state != APP_EXITING) {
    app->state = APP_EXITING;
  }
}

API void app_pause(bool paused)
{
  app_t *app = app_ptr();
  if (app->state == APP_EXITING || app->state == APP_EXITED) {
    return;
  }
  app->state = paused ? APP_PAUSED : APP_RUNNING;
}

API void app_emit_hot_sync()
{
  app_scene_sync(app_ptr()->scene, SYNC_SIGNAL_HOT_SYNC);
}

API void app_process(float delta)
{
  assert(g_app && "app not initialized");
  app_t *app = app_ptr();
  if (app->state == APP_EXITED) return;

  app->screen_size = get_screen_size();

  tween_process(delta);
  timer_process(delta);
  // sound_process();
  // input_process();

  if (app->scene_next) {
    if (app->scene && app->scene_transition != SCENE_TRANSITION_EXITING) {
      app_scene_sync(app->scene, SYNC_SIGNAL_ON_EXIT);
      app->scene_transition = SCENE_TRANSITION_EXITING;
    }

    // exiting phase: current scene
    if (app->scene_transition == SCENE_TRANSITION_EXITING && !app_scene_exiting()) {
      return;
    }

    if (app->scene) {
      app_scene_free();
      app->scene = SCENE_NONE;
    }

    app->scene_transition = SCENE_TRANSITION_ENTERING;
    app->scene = app->scene_next;
    app->scene_next = SCENE_NONE;
    arena_reset(app->scene_arena);
    app_scene_sync(app->scene, SYNC_SIGNAL_ON_ENTER);
    app_scene_init();
  }

  if (app->state == APP_EXITING) {
    if (app->scene_transition != SCENE_TRANSITION_EXITING) {
      app->scene_transition = SCENE_TRANSITION_EXITING;
      app_scene_sync(app->scene, SYNC_SIGNAL_ON_EXIT);
    }
    if (app_scene_exiting()) {
      app_scene_free();
      app->state = APP_EXITED;
      app->scene = SCENE_NONE;
      app->scene_transition = SCENE_TRANSITION_NONE;
    }
    return;
  }

  if (app->scene_transition == SCENE_TRANSITION_ENTERING && app_scene_entering()) {
    app->scene_transition = SCENE_TRANSITION_NONE;
    return;
  }

  if (is_window_resized()) {
    app_scene_sync(app->scene, SYNC_SIGNAL_WINDOW_RESIZED);
  }

  app_scene_process(delta);
}

API void app_draw()
{
  app_scene_draw();
}

