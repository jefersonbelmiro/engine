#pragma once

#include "backend/api.h"
#include "platform/api.h"
#include "core/defs.h"
#include "core/engine.h"
#include "core/mem.h"
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
  engine_ptr()->delta_time = GetFrameTime();

  if (unlikely(!platform_is_ready())) {
    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();
    return;
  }

#if HOT_RELOAD
  // hot_process(GetFrameTime());
#endif

  engine_process(GetFrameTime());

  BeginDrawing();
#if WINDOW_TRANSPARENT
  ClearBackground(BLANK);
  // SetWindowOpacity(0.8);
#else
  ClearBackground(BLACK);
#endif
  engine_draw();
  DrawFPS(10, 10);
  // draw_fps();

#if WINDOW_UNDECORATED
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
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME);
  SetExitKey(KEY_NULL);
}

API void backend_main()
{
  printn("[raylib] backend_main()");
  printn(" - PLATFORM: %d", PLATFORM);

#if PLATFORM == PLATFORM_WEB
  emscripten_set_main_loop(backend_main_loop, 0, 1);
#else

  engine_t *app = engine_ptr();
  while (app->state != ENGINE_EXITED) {
    if (WindowShouldClose()) engine_quit();

    // fullscreen toggle
    if (IsKeyPressed(KEY_F11)) {
      if (!IsWindowFullscreen()) {
        int monitor = GetCurrentMonitor();
        SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        ToggleFullscreen();
      } else {
        ToggleFullscreen();
        SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
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
  arena_print_stats(engine_ptr()->arena->debug_id);
  // arena_print_track(app->arena->debug_id, false);
#endif
  engine_fini();
#if DEBUG_MEMORY_USAGE
  mem_print_stats();
#endif
  // CloseWindow();
}

