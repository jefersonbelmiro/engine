#pragma once

#include "backend/api.h"
#include "backend/codes.h"
#include "platform/api.h"
#include "core/defs.h"
#include "core/engine.h"
#include "core/mem.h"
#include <raylib.h>
#include <time.h>

static const int k_be_key_map[BE_KEY_COUNT] = {
    [BE_KEY_NONE] = 0,
    [BE_KEY_A] = KEY_A,
    [BE_KEY_B] = KEY_B,
    [BE_KEY_C] = KEY_C,
    [BE_KEY_D] = KEY_D,
    [BE_KEY_E] = KEY_E,
    [BE_KEY_F] = KEY_F,
    [BE_KEY_G] = KEY_G,
    [BE_KEY_H] = KEY_H,
    [BE_KEY_I] = KEY_I,
    [BE_KEY_J] = KEY_J,
    [BE_KEY_K] = KEY_K,
    [BE_KEY_L] = KEY_L,
    [BE_KEY_M] = KEY_M,
    [BE_KEY_N] = KEY_N,
    [BE_KEY_O] = KEY_O,
    [BE_KEY_P] = KEY_P,
    [BE_KEY_Q] = KEY_Q,
    [BE_KEY_R] = KEY_R,
    [BE_KEY_S] = KEY_S,
    [BE_KEY_T] = KEY_T,
    [BE_KEY_U] = KEY_U,
    [BE_KEY_V] = KEY_V,
    [BE_KEY_W] = KEY_W,
    [BE_KEY_X] = KEY_X,
    [BE_KEY_Y] = KEY_Y,
    [BE_KEY_Z] = KEY_Z,
    [BE_KEY_0] = KEY_ZERO,
    [BE_KEY_1] = KEY_ONE,
    [BE_KEY_2] = KEY_TWO,
    [BE_KEY_3] = KEY_THREE,
    [BE_KEY_4] = KEY_FOUR,
    [BE_KEY_5] = KEY_FIVE,
    [BE_KEY_6] = KEY_SIX,
    [BE_KEY_7] = KEY_SEVEN,
    [BE_KEY_8] = KEY_EIGHT,
    [BE_KEY_9] = KEY_NINE,
    [BE_KEY_SPACE] = KEY_SPACE,
    [BE_KEY_ENTER] = KEY_ENTER,
    [BE_KEY_ESCAPE] = KEY_ESCAPE,
    [BE_KEY_TAB] = KEY_TAB,
    [BE_KEY_BACKSPACE] = KEY_BACKSPACE,
    [BE_KEY_UP] = KEY_UP,
    [BE_KEY_DOWN] = KEY_DOWN,
    [BE_KEY_LEFT] = KEY_LEFT,
    [BE_KEY_RIGHT] = KEY_RIGHT,
    [BE_KEY_LEFT_SHIFT] = KEY_LEFT_SHIFT,
    [BE_KEY_RIGHT_SHIFT] = KEY_RIGHT_SHIFT,
    [BE_KEY_LEFT_ALT] = KEY_LEFT_ALT,
    [BE_KEY_RIGHT_ALT] = KEY_RIGHT_ALT,
    [BE_KEY_LEFT_CONTROL] = KEY_LEFT_CONTROL,
    [BE_KEY_RIGHT_CONTROL] = KEY_RIGHT_CONTROL,
    [BE_KEY_F1] = KEY_F1,
    [BE_KEY_F2] = KEY_F2,
    [BE_KEY_F3] = KEY_F3,
    [BE_KEY_F4] = KEY_F4,
    [BE_KEY_F5] = KEY_F5,
    [BE_KEY_F6] = KEY_F6,
    [BE_KEY_F7] = KEY_F7,
    [BE_KEY_F8] = KEY_F8,
    [BE_KEY_F9] = KEY_F9,
    [BE_KEY_F10] = KEY_F10,
    [BE_KEY_F11] = KEY_F11,
    [BE_KEY_F12] = KEY_F12,
};

static const int k_be_mouse_map[BE_MOUSE_COUNT] = {
  [BE_MOUSE_LEFT] = MOUSE_BUTTON_LEFT,
  [BE_MOUSE_RIGHT] = MOUSE_BUTTON_RIGHT,
  [BE_MOUSE_MIDDLE] = MOUSE_BUTTON_MIDDLE,
  [BE_MOUSE_X1] = MOUSE_BUTTON_SIDE,
  [BE_MOUSE_X2] = MOUSE_BUTTON_EXTRA,
};

static const int k_be_gamepad_button_map[BE_GAMEPAD_BUTTON_COUNT] = {
  [BE_GAMEPAD_DPAD_UP] = GAMEPAD_BUTTON_LEFT_FACE_UP,
  [BE_GAMEPAD_DPAD_DOWN] = GAMEPAD_BUTTON_LEFT_FACE_DOWN,
  [BE_GAMEPAD_DPAD_LEFT] = GAMEPAD_BUTTON_LEFT_FACE_LEFT,
  [BE_GAMEPAD_DPAD_RIGHT] = GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
  [BE_GAMEPAD_A] = GAMEPAD_BUTTON_RIGHT_FACE_DOWN,
  [BE_GAMEPAD_B] = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,
  [BE_GAMEPAD_X] = GAMEPAD_BUTTON_RIGHT_FACE_LEFT,
  [BE_GAMEPAD_Y] = GAMEPAD_BUTTON_RIGHT_FACE_UP,
  [BE_GAMEPAD_LB] = GAMEPAD_BUTTON_LEFT_TRIGGER_1,
  [BE_GAMEPAD_RB] = GAMEPAD_BUTTON_RIGHT_TRIGGER_1,
  [BE_GAMEPAD_LT] = GAMEPAD_BUTTON_LEFT_TRIGGER_2,
  [BE_GAMEPAD_RT] = GAMEPAD_BUTTON_RIGHT_TRIGGER_2,
  [BE_GAMEPAD_BACK] = GAMEPAD_BUTTON_MIDDLE_LEFT,
  [BE_GAMEPAD_START] = GAMEPAD_BUTTON_MIDDLE_RIGHT,
  [BE_GAMEPAD_GUIDE] = GAMEPAD_BUTTON_MIDDLE,
  [BE_GAMEPAD_L3] = GAMEPAD_BUTTON_LEFT_THUMB,
  [BE_GAMEPAD_R3] = GAMEPAD_BUTTON_RIGHT_THUMB,
};

static const int k_be_gamepad_axis_map[BE_GAMEPAD_AXIS_COUNT] = {
  [BE_GAMEPAD_AXIS_LEFT_X] = GAMEPAD_AXIS_LEFT_X,
  [BE_GAMEPAD_AXIS_LEFT_Y] = GAMEPAD_AXIS_LEFT_Y,
  [BE_GAMEPAD_AXIS_RIGHT_X] = GAMEPAD_AXIS_RIGHT_X,
  [BE_GAMEPAD_AXIS_RIGHT_Y] = GAMEPAD_AXIS_RIGHT_Y,
  [BE_GAMEPAD_AXIS_TRIGGER_LEFT] = GAMEPAD_AXIS_LEFT_TRIGGER,
  [BE_GAMEPAD_AXIS_TRIGGER_RIGHT] = GAMEPAD_AXIS_RIGHT_TRIGGER,
};

API screen_size_t be_screen_size()
{
  return (screen_size_t) {
    .x = GetScreenWidth(),
    .y = GetScreenHeight(),
  };
}

API bool be_window_resized()
{
  return IsWindowResized();
}

API bool be_key_down(be_key_t key)
{
  return IsKeyDown(k_be_key_map[key]);
}

API bool be_mouse_button_down(be_mouse_button_t button)
{
  return IsMouseButtonDown(k_be_mouse_map[button]);
}

API vec2_t be_mouse_position()
{
  Vector2 pos = GetMousePosition();
  return (vec2_t){ pos.x, pos.y };
}

API vec2_t be_mouse_delta()
{
  Vector2 delta = GetMouseDelta();
  return (vec2_t){ delta.x, delta.y };
}

API vec2_t be_mouse_wheel()
{
  Vector2 wheel = GetMouseWheelMoveV();
  return (vec2_t){ wheel.x, wheel.y };
}

API int be_gamepad_count()
{
  int count = 0;
  for (int gamepad = 0; gamepad < 4; gamepad++) {
    if (IsGamepadAvailable(gamepad)) count++;
  }
  return count;
}

API bool be_gamepad_button_down(int gamepad, be_gamepad_button_t button)
{
  return IsGamepadButtonDown(gamepad, k_be_gamepad_button_map[button]);
}

API float be_gamepad_axis(int gamepad, be_gamepad_axis_t axis)
{
  return GetGamepadAxisMovement(gamepad, k_be_gamepad_axis_map[axis]);
}

API int be_touch_count()
{
  return GetTouchPointCount();
}

API vec2_t be_touch_position(int index)
{
  Vector2 pos = GetTouchPosition(index);
  return (vec2_t){ pos.x, pos.y };
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
