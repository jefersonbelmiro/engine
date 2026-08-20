#pragma once

#include "backend/api.h"
#include "core/app.h"
#include "core/arena.h"
#include "core/defs.h"
#include "core/mem.h"
#include "platform/api.h"
#include <raylib.h>
#include <time.h>

API screen_size_t get_screen_size()
{
  return (screen_size_t) {
    .x = GetScreenWidth(),
    .y = GetScreenHeight(),
  };
}

API bool is_window_resized()
{
  return IsWindowResized();
}

API void backend_main_loop()
{
  if (unlikely(!platform_is_ready())) {
    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();
    return;
  }

#if HOT_RELOAD
  // hot_process(GetFrameTime());
#endif

  app_process(GetFrameTime());

  BeginDrawing();
#if APP_WINDOW_TRANSPARENT
  ClearBackground(BLANK);
  // SetWindowOpacity(0.8);
#else
  ClearBackground(BLACK);
#endif
  app_draw();
  // draw_fps();

#if APP_WINDOW_UNDECORATED
  if (!IsWindowFullscreen()) {
    DrawRectangleLinesEx(
      (Rectangle){ 0, 0, GetScreenWidth(), GetScreenHeight(), },
      2, (Color){ 24, 24, 32, 255 }
    );
  }
#endif

  EndDrawing();
}

API void backend_init()
{
  printn("[raylib] backend_init()");

  srand(time(NULL));

  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST | FLAG_WINDOW_RESIZABLE);
  SetTraceLogLevel(LOG_WARNING);
  InitWindow(APP_WINDOW_WIDTH, APP_WINDOW_HEIGHT, APP_WINDOW_NAME);
  SetExitKey(KEY_NULL);
}

API void backend_main()
{
  printn("[raylib] backend_main()");

#if PLATFORM == PLATFORM_WEB
  emscripten_set_main_loop(backend_main_loop, 0, 1);
#else

  app_t *app = app_ptr();
  while (app->state != APP_EXITED) {
    if (WindowShouldClose()) app_quit();

    // fullscreen toggle
    if (IsKeyPressed(KEY_F11)) {
      if (!IsWindowFullscreen()) {
        int monitor = GetCurrentMonitor();
        SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        ToggleFullscreen();
      } else {
        ToggleFullscreen();
        SetWindowSize(APP_WINDOW_WIDTH, APP_WINDOW_HEIGHT);
      }
    }

    backend_main_loop();
  }

  backend_fini();

#endif
}

API void backend_fini()
{
  printn("[raylib] backend_fini()");

#if DEBUG_MEMORY_USAGE
  arena_print_stats(app_ptr()->arena->debug_id);
  // arena_print_track(app->arena->debug_id, false);
#endif
  app_fini();
#if DEBUG_MEMORY_USAGE
  mem_print_stats();
#endif
  // CloseWindow();
}

