<!--
vim: foldmethod=marker foldlevel=0
-->

# src/project.h
```c {{{
#pragma once

#include "core/defs.h"

typedef struct {
  char *name;
  char *version;
  char *binary;
  char *lib_raylib_path;
  char *lib_emsdk_path;
  bool support_fileformat_jpg;
} project_t;

static project_t g_project = {
  .name = "{{ NAME }}",
  .version = "{{ VERSION }}",
  .binary = "{{ BINARY }}",
  .lib_raylib_path = "~/dev/libs/raylib",
  .lib_emsdk_path = "~/dev/libs/emsdk",
  .support_fileformat_jpg = true,
};

API project_t *project_ptr()
{
  return &g_project;
}
``` }}}

# src/scenes/main.h
```c {{{
#pragma once

#include "core/engine.h"
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

API bool main_scene_exiting(main_scene_t *scene)
{
  (void) scene;
  return true;
}

API bool main_scene_entering(main_scene_t *scene)
{
  (void) scene;
  return true;
}

API void main_scene_sync(main_scene_t *scene, sync_signal_type_t signal)
{
  (void) scene; (void) signal;
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
``` }}}

# src/scenes/menu.h
```c {{{
#pragma once

#include "core/engine.h"
#include "core/arena.h"

typedef struct {
  char pad;
} menu_scene_t;

API menu_scene_t* menu_scene_init()
{
  arena_t *arena = engine_scene_arena();
  menu_scene_t *scene = arena_push(arena, menu_scene_t, 1);
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
  (void) scene; (void) signal;
}

API void menu_scene_free(menu_scene_t *scene)
{
  (void) scene;
}

API void menu_scene_process(menu_scene_t *scene, float delta)
{
  (void) scene; (void) delta;
}

API void menu_scene_draw(menu_scene_t *scene)
{
  (void) scene;
}
``` }}}

# src/scenes/entry.h
```c {{{
#pragma once

#include "core/engine.h"
#include "scenes/main.h"
#include "scenes/menu.h"

typedef enum scene_type_t {
  SCENE_NONE,
  SCENE_MAIN,
  SCENE_MENU,
  SCENE_COUNT,
} scene_type_t;

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

API void engine_scene_process() 
{
  engine_t *app = engine_ptr();
  float delta = engine_delta_time();
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

API void engine_scene_sync(u8 scene, sync_signal_type_t signal)
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
``` }}}

# src/main.c
```c {{{
#include "core/engine.c"
#include "scenes/entry.h"

#if PLATFORM == PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

int main()
{
  printn("main()");

  engine_init();
  engine_set_scene(SCENE_MENU);
  engine_start();

#if PLATFORM == PLATFORM_WEB
  emscripten_set_main_loop(engine_step, 0, 1);
#else
  while (!window_should_close()) engine_step();
  engine_fini();
#endif

  return 0;
}
``` }}}

# compile_flags.txt
```txt {{{
-std=gnu11
-I./engine/src
-I./src
-I{{ RAYLIB_PATH }}/src
-I{{ EMSDK_PATH }}/upstream/emscripten/cache/sysroot/include
-DDEBUG
-DLOG_LEVEL=5
-DDEBUG_MEMORY_USAGE
-DARENA_FALLBACK_MALLOC
-DHOT_RELOAD
-DMODULE_BUILD
-DPLATFORM=PLATFORM_LINUX
-DBACKEND=BACKEND_RAYLIB
-Wall
-Wextra
``` }}}

# tools/entry.h
```c {{{
#pragma once

#include "core/defs.h"

static tool_t tools[] = {
  { 
    .name = "resource_pack",
    .description = "generate resource packages",
    .source_path = "tools/resource_pack.c",
  },
};

API tool_array_t *tools_entries()
{
  static tool_array_t array = {
    .array = tools,
    .count = countof(tools)
  };
  return &array;
}
``` }}}

# tools/resource_pack.c
```c {{{
#include "core/arena.h"
#include "core/defs.h"
#include "core/package.h"
#include "core/string.h"

#define DEFAULT_OUTPUT_DIR "resources/package"

API char *fs_binary_path()
{
  return "./";
}

static arena_t *g_arena;

void show_cmd_line_help()
{
  printn(
    "resource_pack\n"
    " usage   : resource_pack [options]\n"
    " options :\n"
    "    -h  --help          : show command line usage help\n"
    "    -o  --output=[PATH] : output directory to write packages, default '%s'\n"
  , DEFAULT_OUTPUT_DIR);
}

bool generate(char *output)
{
  package_t pkg = {0};
  package_def_t def = {0};
  package_count_t caps = {
    .textures = 1,
    .atlas = 1,
    .fonts = 1,
    .sounds = 1,
    .musics = 1,
  };
  package_def_init(&def, caps, g_arena);
  // package_def_append_texture(&def, "TEXTURE_001", "resources/texture/001.jpg");
  // package_def_append_atlas(&def, "ATLAS_01_64", "resources/texture/atlas_01_64.png", vec2(64, 64));
  // package_def_append_font(&def, "FONT_MONOGRAM", "resources/font/monogram.ttf");
  // package_def_append_sound(&def, "SOUND_POWERUP_01", "resources/sounds/sfx/sfx_powerup_01.wav", 0.5, 2);
  // package_def_append_music(&def, "MUSIC_MENU_01", "resources/sounds/music/menu_01.mp3", 0.5);

  package_def_write_header(&def, "core");
  package_def_make(&def, &pkg, g_arena);
  package_write(&pkg, output, "core", g_arena);

  return true;
}

int main(int argc, char **argv)
{
  g_arena = arena_create(MB(8), "resource_pack");
  char *output = NULL;

  for (int i = 0; i < argc; i++) {
    if (str_eq(argv[i], "-h") || str_eq(argv[i], "--help")) {
      show_cmd_line_help();
      return 0;
    }
    else if (str_eq(argv[i], "-o") || str_eq(argv[i], "--output")) {
      if (i + 1 < argc) output = argv[++i];
    }
    else if (str_start_with(argv[i], "-o=")) {
      output = argv[i] + 3;
    }
    else if (str_start_with(argv[i], "--output=")) {
      output = argv[i] + 9;
    }
  }

  if (!output) {
    output = str_dup(DEFAULT_OUTPUT_DIR, g_arena);
  }
  
  if (!generate(output)) {
    return 1;
  }
  return 0;
}
``` }}}

# .gitignore
``` {{{
/build
``` }}}
