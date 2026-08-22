#pragma once

#include "core/defs.h"
#include "core/defs.h"
#include "scenes/main.h"
#include "scenes/menu.h"

typedef enum scene_type_t {
  SCENE_NONE,
  SCENE_MENU,
  SCENE_MAIN,
  SCENE_COUNT,
} scene_type_t;

API void engine_scene_setup()
{
  engine_set_scene(SCENE_MENU);
}

API bool engine_scene_entering() 
{
  engine_t *app = engine_ptr();
  switch (app->scene) {
  case SCENE_MAIN:
    return main_scene_entering(app->scene_state);
  case SCENE_MENU:
    return menu_scene_entering(app->scene_state);
  default: return true;
  }
}

API bool engine_scene_exiting() 
{
  engine_t *app = engine_ptr();
  switch (app->scene) {
    case SCENE_MAIN:
      return main_scene_exiting(app->scene_state);
    case SCENE_MENU:
      return menu_scene_exiting(app->scene_state);
    default: return true;
  }
}

API void engine_scene_init() 
{
  engine_t *app = engine_ptr();
  switch (app->scene) {
    case SCENE_MAIN:
      app->scene_state = main_scene_init();
    break;
    case SCENE_MENU:
      app->scene_state = menu_scene_init();
    break;
    default: break;
  }
}

API void engine_scene_process(float delta) 
{
  engine_t *app = engine_ptr();
  switch (app->scene) {
    case SCENE_MAIN: 
      main_scene_process(app->scene_state, delta);
    break;
    case SCENE_MENU: 
      menu_scene_process(app->scene_state, delta);
    break;
    default: break;
  }
}

API void engine_scene_draw() 
{
  engine_t *app = engine_ptr();
  switch (app->scene) {
    case SCENE_MAIN: 
      main_scene_draw(app->scene_state);
    break;
    case SCENE_MENU: 
      menu_scene_draw(app->scene_state);
    break;
    default: break;
  }
}

API void engine_scene_free() 
{
  engine_t *app = engine_ptr();
  switch (app->scene) {
    case SCENE_MAIN: 
      main_scene_free(app->scene_state);
    break;
    case SCENE_MENU: 
      menu_scene_free(app->scene_state);
    break;
    default: break;
  }
}

API void engine_scene_sync(scene_type_t scene, sync_signal_type_t signal)
{
  switch (scene) {
    case SCENE_MAIN:
      main_scene_sync(engine_ptr()->scene_state, signal);
    break;
    case SCENE_MENU:
      menu_scene_sync(engine_ptr()->scene_state, signal);
    break;
    default: break;
  }
}

