#include "engine.h"
#include "backend/api.h"
#include "core/arena.h"
#include "core/defs.h"
#include "core/input.h"
#include "core/timer.h"
#include "core/tween.h"
#include "platform/api.h"

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

  engine->package_handler_arena = arena_create_sub(arena, ENGINE_PACKAGE_HANDLERS_ARENA_SIZE, "package_handler");
  input_init(arena_create_sub(arena, input_memory_size(), "input"));
  // resource_init(arena_create_sub(arena, resources_memory_size(), "resource"));
  // sound_init(arena_create_sub(arena, sound_memory_size(), "sound"));
  tween_init(arena_create_sub(arena, tween_memory_size(), "tween"));
  timer_init(arena_create_sub(arena, timer_memory_size(), "timer"));

  engine->package_resource_arena = arena_create(ENGINE_PACKAGE_RESOURCES_ARENA_SIZE, "package_resource");

  engine->scene = 0;//SCENE_NONE;
  engine->arena = arena;
  // keep scene arena to end, for cache locality(i think)
  engine->scene_arena = arena_create_sub(arena, ENGINE_SCENE_ARENA_SIZE, "scene");

  // engine_package_load("core");
}

API void engine_fini()
{
  engine_t *engine = engine_ptr();
  // resource_unload();

#if DEBUG_MEMORY_USAGE
  arena_print_stats(engine->package_resource_arena->debug_id);
  arena_print_stats(engine->arena->debug_id);
  // arena_print_track(engine->arena->debug_id, false);
#endif

#if DEBUG_MEMORY_USAGE
  mem_print_stats();
#endif

  arena_fini(engine->arena);
  arena_fini(engine->package_resource_arena);
  g_engine = NULL;
}

API void engine_start(void)
{
  // resource_start();
  // sound_start();

  engine_t *engine = engine_ptr();
  engine->state = ENGINE_RUNNING;

  assert((engine->scene || engine->scene_next) && "initial scene not defined. @see engine_set_scene(...)");

  platform_init();
  backend_init();

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

API void engine_process()
{
  assert(g_engine && "engine not initialized");
  engine_t *engine = engine_ptr();
  if (engine->state == ENGINE_EXITED) return;

  engine->screen_size = screen_size();

  tween_process();
  timer_process();
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

  if (window_resized()) {
    engine_scene_sync(engine->scene, SYNC_SIGNAL_WINDOW_RESIZED);
  }

  engine_scene_process();
}

API void engine_draw()
{
  engine_scene_draw();
}

API void engine_package_load(char *name)
{
  engine_t *engine = engine_ptr();
  package_t *package = arena_push(engine->package_handler_arena, package_t, 1);
  package_read(package, name,engine->package_resource_arena, engine->package_handler_arena);

  engine->packages[0] = package;
}

API package_t *engine_package_core()
{
  return engine_ptr()->packages[0];
}
