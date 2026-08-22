#include "engine.h"
#include "backend/api.h"
#include "core/arena.h"
#include "core/defs.h"
#include "core/input.h"
#include "core/timer.h"
#include "core/tween.h"
#include "platform/api.h"
#include "scenes/entry.h"

API engine_t* engine_ptr()
{
  return g_engine;
}

API bool engine_paused()
{
  engine_t *engine = engine_ptr();
  return engine->state == ENGINE_PAUSED;
}

API screen_size_t* engine_screen_size()
{
  return &engine_ptr()->screen_size;
}

API float engine_delta_time()
{
  return engine_ptr()->delta_time;
}

API void engine_set_scene(u8 type)
{
  engine_t *engine = engine_ptr();
  engine->scene_next = type;
}

API arena_t* engine_scene_arena()
{
  engine_t *engine = engine_ptr();
  return engine->scene_arena;
}

API void engine_init(void)
{
  assert(!g_engine);
  arena_t *arena = arena_create(ENGINE_ARENA_SIZE, "engine");
  engine_t *engine = arena_push_zero(arena, engine_t, 1);
  g_engine = engine;

  input_init(arena);
  // resource_init(arena_create_sub(arena, resources_memory_size(), "resource"));
  // sound_init(arena_create_sub(arena, sound_memory_size(), "sound"));
  tween_init(arena_create_sub(arena, tween_memory_size(), "tween"));
  timer_init(arena_create_sub(arena, timer_memory_size(), "timer"));

  engine->scene = 0;//SCENE_NONE;
  engine->arena = arena;
  // keep scene arena to end, for cache locality(i think)
  engine->scene_arena = arena_create_sub(arena, ENGINE_SCENE_ARENA_SIZE, "scene");
}

API void engine_fini()
{
  engine_t *engine = engine_ptr();
  // resource_unload();
  arena_fini(engine->arena);
  g_engine = NULL;
}

API void engine_start(void)
{
  engine_init();
  engine_scene_setup();

  // resource_start();
  // sound_start();

  engine_t *engine = engine_ptr();
  engine->state = ENGINE_RUNNING;

  platform_init();
  backend_init();

  // @fixme: mark ready after load core resources package
  platform_mark_ready();

  backend_main();
}

API void engine_quit()
{
  engine_t *engine = engine_ptr();
  if (engine->state != ENGINE_EXITING) {
    engine->state = ENGINE_EXITING;
  }
}

API void engine_pause(bool paused)
{
  engine_t *engine = engine_ptr();
  if (engine->state == ENGINE_EXITING || engine->state == ENGINE_EXITED) {
    return;
  }
  engine->state = paused ? ENGINE_PAUSED : ENGINE_RUNNING;
}

API void engine_emit_hot_sync()
{
  engine_scene_sync(engine_ptr()->scene, SYNC_SIGNAL_HOT_SYNC);
}

API void engine_process(float delta)
{
  assert(g_engine && "engine not initialized");
  engine_t *engine = engine_ptr();
  if (engine->state == ENGINE_EXITED) return;

  engine->screen_size = be_screen_size();

  tween_process(delta);
  timer_process(delta);
  // sound_process();
  input_process();

  if (engine->scene_next) {
    if (engine->scene && engine->scene_transition != SCENE_TRANSITION_EXITING) {
      engine_scene_sync(engine->scene, SYNC_SIGNAL_ON_EXIT);
      engine->scene_transition = SCENE_TRANSITION_EXITING;
    }

    // exiting phase: current scene
    if (engine->scene_transition == SCENE_TRANSITION_EXITING && !engine_scene_exiting()) {
      return;
    }

    if (engine->scene) {
      engine_scene_free();
      engine->scene = 0;//SCENE_NONE;
    }

    engine->scene_transition = SCENE_TRANSITION_ENTERING;
    engine->scene = engine->scene_next;
    engine->scene_next = 0;//SCENE_NONE;
    arena_reset(engine->scene_arena);
    engine_scene_sync(engine->scene, SYNC_SIGNAL_ON_ENTER);
    engine_scene_init();
  }

  if (engine->state == ENGINE_EXITING) {
    if (engine->scene_transition != SCENE_TRANSITION_EXITING) {
      engine->scene_transition = SCENE_TRANSITION_EXITING;
      engine_scene_sync(engine->scene, SYNC_SIGNAL_ON_EXIT);
    }
    if (engine_scene_exiting()) {
      engine_scene_free();
      engine->state = ENGINE_EXITED;
      engine->scene = 0;//SCENE_NONE;
      engine->scene_transition = SCENE_TRANSITION_NONE;
    }
    return;
  }

  if (engine->scene_transition == SCENE_TRANSITION_ENTERING && engine_scene_entering()) {
    engine->scene_transition = SCENE_TRANSITION_NONE;
    return;
  }

  if (be_window_resized()) {
    engine_scene_sync(engine->scene, SYNC_SIGNAL_WINDOW_RESIZED);
  }

  engine_scene_process(delta);
}

API void engine_draw()
{
  engine_scene_draw();
}
