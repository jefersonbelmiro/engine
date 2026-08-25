#pragma once

#include "backend/api.h"
#include "backend/codes.h"
#include "core/package.h"
#include "platform/api.h"
#include "core/defs.h"
#include "core/engine.h"
#include "core/mem.h"
#include <raylib.h>
#include <time.h>

static const int k_keycode_map[INPUT_KEY_COUNT] = {
    [INPUT_KEY_NONE] = 0,
    [INPUT_KEY_A] = KEY_A,
    [INPUT_KEY_B] = KEY_B,
    [INPUT_KEY_C] = KEY_C,
    [INPUT_KEY_D] = KEY_D,
    [INPUT_KEY_E] = KEY_E,
    [INPUT_KEY_F] = KEY_F,
    [INPUT_KEY_G] = KEY_G,
    [INPUT_KEY_H] = KEY_H,
    [INPUT_KEY_I] = KEY_I,
    [INPUT_KEY_J] = KEY_J,
    [INPUT_KEY_K] = KEY_K,
    [INPUT_KEY_L] = KEY_L,
    [INPUT_KEY_M] = KEY_M,
    [INPUT_KEY_N] = KEY_N,
    [INPUT_KEY_O] = KEY_O,
    [INPUT_KEY_P] = KEY_P,
    [INPUT_KEY_Q] = KEY_Q,
    [INPUT_KEY_R] = KEY_R,
    [INPUT_KEY_S] = KEY_S,
    [INPUT_KEY_T] = KEY_T,
    [INPUT_KEY_U] = KEY_U,
    [INPUT_KEY_V] = KEY_V,
    [INPUT_KEY_W] = KEY_W,
    [INPUT_KEY_X] = KEY_X,
    [INPUT_KEY_Y] = KEY_Y,
    [INPUT_KEY_Z] = KEY_Z,
    [INPUT_KEY_ZERO] = KEY_ZERO,
    [INPUT_KEY_ONE] = KEY_ONE,
    [INPUT_KEY_TWO] = KEY_TWO,
    [INPUT_KEY_THREE] = KEY_THREE,
    [INPUT_KEY_FOUR] = KEY_FOUR,
    [INPUT_KEY_FIVE] = KEY_FIVE,
    [INPUT_KEY_SIX] = KEY_SIX,
    [INPUT_KEY_SEVEN] = KEY_SEVEN,
    [INPUT_KEY_EIGHT] = KEY_EIGHT,
    [INPUT_KEY_NINE] = KEY_NINE,
    [INPUT_KEY_SPACE] = KEY_SPACE,
    [INPUT_KEY_ENTER] = KEY_ENTER,
    [INPUT_KEY_ESCAPE] = KEY_ESCAPE,
    [INPUT_KEY_TAB] = KEY_TAB,
    [INPUT_KEY_BACKSPACE] = KEY_BACKSPACE,
    [INPUT_KEY_UP] = KEY_UP,
    [INPUT_KEY_DOWN] = KEY_DOWN,
    [INPUT_KEY_LEFT] = KEY_LEFT,
    [INPUT_KEY_RIGHT] = KEY_RIGHT,
    [INPUT_KEY_LEFT_SHIFT] = KEY_LEFT_SHIFT,
    [INPUT_KEY_RIGHT_SHIFT] = KEY_RIGHT_SHIFT,
    [INPUT_KEY_LEFT_ALT] = KEY_LEFT_ALT,
    [INPUT_KEY_RIGHT_ALT] = KEY_RIGHT_ALT,
    [INPUT_KEY_LEFT_CONTROL] = KEY_LEFT_CONTROL,
    [INPUT_KEY_RIGHT_CONTROL] = KEY_RIGHT_CONTROL,
    [INPUT_KEY_F1] = KEY_F1,
    [INPUT_KEY_F2] = KEY_F2,
    [INPUT_KEY_F3] = KEY_F3,
    [INPUT_KEY_F4] = KEY_F4,
    [INPUT_KEY_F5] = KEY_F5,
    [INPUT_KEY_F6] = KEY_F6,
    [INPUT_KEY_F7] = KEY_F7,
    [INPUT_KEY_F8] = KEY_F8,
    [INPUT_KEY_F9] = KEY_F9,
    [INPUT_KEY_F10] = KEY_F10,
    [INPUT_KEY_F11] = KEY_F11,
    [INPUT_KEY_F12] = KEY_F12,
};

static const int k_mouse_button_map[INPUT_MOUSE_BUTTON_COUNT] = {
  [INPUT_MOUSE_BUTTON_LEFT] = MOUSE_BUTTON_LEFT,
  [INPUT_MOUSE_BUTTON_RIGHT] = MOUSE_BUTTON_RIGHT,
  [INPUT_MOUSE_BUTTON_MIDDLE] = MOUSE_BUTTON_MIDDLE,
  [INPUT_MOUSE_BUTTON_SIDE] = MOUSE_BUTTON_SIDE,
  [INPUT_MOUSE_BUTTON_EXTRA] = MOUSE_BUTTON_EXTRA,
};

static const int k_gamepad_button_map[INPUT_GAMEPAD_BUTTON_COUNT] = {
  [INPUT_GAMEPAD_BUTTON_LEFT_FACE_UP] = GAMEPAD_BUTTON_LEFT_FACE_UP,
  [INPUT_GAMEPAD_BUTTON_LEFT_FACE_DOWN] = GAMEPAD_BUTTON_LEFT_FACE_DOWN,
  [INPUT_GAMEPAD_BUTTON_LEFT_FACE_LEFT] = GAMEPAD_BUTTON_LEFT_FACE_LEFT,
  [INPUT_GAMEPAD_BUTTON_LEFT_FACE_RIGHT] = GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
  [INPUT_GAMEPAD_BUTTON_RIGHT_FACE_DOWN] = GAMEPAD_BUTTON_RIGHT_FACE_DOWN,
  [INPUT_GAMEPAD_BUTTON_RIGHT_FACE_RIGHT] = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,
  [INPUT_GAMEPAD_BUTTON_RIGHT_FACE_LEFT] = GAMEPAD_BUTTON_RIGHT_FACE_LEFT,
  [INPUT_GAMEPAD_BUTTON_RIGHT_FACE_UP] = GAMEPAD_BUTTON_RIGHT_FACE_UP,
  [INPUT_GAMEPAD_BUTTON_LEFT_TRIGGER_1] = GAMEPAD_BUTTON_LEFT_TRIGGER_1,
  [INPUT_GAMEPAD_BUTTON_RIGHT_TRIGGER_1] = GAMEPAD_BUTTON_RIGHT_TRIGGER_1,
  [INPUT_GAMEPAD_BUTTON_LEFT_TRIGGER_2] = GAMEPAD_BUTTON_LEFT_TRIGGER_2,
  [INPUT_GAMEPAD_BUTTON_RIGHT_TRIGGER_2] = GAMEPAD_BUTTON_RIGHT_TRIGGER_2,
  [INPUT_GAMEPAD_BUTTON_MIDDLE_LEFT] = GAMEPAD_BUTTON_MIDDLE_LEFT,
  [INPUT_GAMEPAD_BUTTON_MIDDLE_RIGHT] = GAMEPAD_BUTTON_MIDDLE_RIGHT,
  [INPUT_GAMEPAD_BUTTON_MIDDLE] = GAMEPAD_BUTTON_MIDDLE,
  [INPUT_GAMEPAD_BUTTON_LEFT_THUMB] = GAMEPAD_BUTTON_LEFT_THUMB,
  [INPUT_GAMEPAD_BUTTON_RIGHT_THUMB] = GAMEPAD_BUTTON_RIGHT_THUMB,
};

static const int k_gamepad_axis_map[INPUT_GAMEPAD_AXIS_COUNT] = {
  [INPUT_GAMEPAD_AXIS_LEFT_X] = GAMEPAD_AXIS_LEFT_X,
  [INPUT_GAMEPAD_AXIS_LEFT_Y] = GAMEPAD_AXIS_LEFT_Y,
  [INPUT_GAMEPAD_AXIS_RIGHT_X] = GAMEPAD_AXIS_RIGHT_X,
  [INPUT_GAMEPAD_AXIS_RIGHT_Y] = GAMEPAD_AXIS_RIGHT_Y,
  [INPUT_GAMEPAD_AXIS_LEFT_TRIGGER] = GAMEPAD_AXIS_LEFT_TRIGGER,
  [INPUT_GAMEPAD_AXIS_RIGHT_TRIGGER] = GAMEPAD_AXIS_RIGHT_TRIGGER,
};

API screen_size_t screen_size()
{
  return (screen_size_t) {
    .x = GetScreenWidth(),
    .y = GetScreenHeight(),
  };
}

API bool window_resized()
{
  return IsWindowResized();
}

API bool input_key_pressed(key_code_t key)
{
  return IsKeyPressed(k_keycode_map[key]);
}

API bool input_key_down(key_code_t key)
{
  return IsKeyDown(k_keycode_map[key]);
}

API bool input_key_released(key_code_t key)
{
  return IsKeyReleased(k_keycode_map[key]);
}

API bool input_key_up(key_code_t key)
{
  return IsKeyUp(k_keycode_map[key]);
}

API bool input_mouse_button_pressed(mouse_button_t button)
{
  return IsMouseButtonPressed(k_mouse_button_map[button]);
}

API bool input_mouse_button_down(mouse_button_t button)
{
  return IsMouseButtonDown(k_mouse_button_map[button]);
}

API bool input_mouse_button_released(mouse_button_t button)
{
  return IsMouseButtonReleased(k_mouse_button_map[button]);
}

API bool input_mouse_button_up(mouse_button_t button)
{
  return IsMouseButtonUp(k_mouse_button_map[button]);
}

API vec2_t input_mouse_position()
{
  Vector2 pos = GetMousePosition();
  return (vec2_t){ pos.x, pos.y };
}

API vec2_t input_mouse_delta()
{
  Vector2 delta = GetMouseDelta();
  return (vec2_t){ delta.x, delta.y };
}

API vec2_t input_mouse_wheel()
{
  Vector2 wheel = GetMouseWheelMoveV();
  return (vec2_t){ wheel.x, wheel.y };
}

API bool input_gamepad_available(int gamepad)
{
  return IsGamepadAvailable(gamepad);
}

API bool input_gamepad_button_pressed(int gamepad, gamepad_button_t button)
{
  return IsGamepadButtonPressed(gamepad, k_gamepad_button_map[button]);
}

API bool input_gamepad_button_down(int gamepad, gamepad_button_t button)
{
  return IsGamepadButtonDown(gamepad, k_gamepad_button_map[button]);
}

API bool input_gamepad_button_released(int gamepad, gamepad_button_t button)
{
  return IsGamepadButtonReleased(gamepad, k_gamepad_button_map[button]);
}

API bool input_gamepad_button_up(int gamepad, gamepad_button_t button)
{
  return IsGamepadButtonUp(gamepad, k_gamepad_button_map[button]);
}

API float input_gamepad_axis(int gamepad, gamepad_axis_t axis)
{
  return GetGamepadAxisMovement(gamepad, k_gamepad_axis_map[axis]);
}

API int input_gamepad_count()
{
  int count = 0;
  for (int gamepad = 0; gamepad < 4; gamepad++) {
    if (IsGamepadAvailable(gamepad)) count++;
  }
  return count;
}

API int input_touch_count()
{
  return GetTouchPointCount();
}

API vec2_t input_touch_position(int index)
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

  engine_process();

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

  load_package_handlers(engine_ptr()->packages[0]);
}

API void backend_main()
{
  printn("[raylib] backend_main()");
  printn(" - PLATFORM: %d", PLATFORM);
  printn(" - BACKEND: %d", BACKEND);
  printn(" - SCENE: %d", engine_ptr()->scene);

#if PLATFORM == PLATFORM_WEB
  emscripten_set_main_loop(backend_main_loop, 0, 1);
#else

  engine_t *engine = engine_ptr();
  while (engine->state != ENGINE_EXITED) {
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
  // arena_print_track(engine_ptr()->arena->debug_id, false);
#endif
  engine_fini();
#if DEBUG_MEMORY_USAGE
  mem_print_stats();
#endif
  // CloseWindow();
}


API void draw_rectangle_lines(rect_t rec, float thick, color_t color)
{
  DrawRectangleLinesEx(
    (Rectangle) {rec.x, rec.y, rec.width, rec.height}, 
    thick, (Color) {color.r, color.g, color.b, color.a}
  );
}

API void load_package_handlers(package_t *package)
{
  if (!package) {
    return;
  }
  arena_t *arena = engine_ptr()->package_arena;
  package->handlers.textures = arena_push(arena, texture_t, package->count.textures);
  package->handlers.atlas = arena_push(arena, atlas_t, package->count.atlas);
  package->handlers.fonts = arena_push(arena, font_t, package->count.fonts);
  package->handlers.sounds = arena_push(arena, sound_t, package->count.sounds);
  package->handlers.musics = arena_push(arena, music_t, package->count.musics);

  for (u32 i = 0; i < package->count.textures; i++) {
    resource_texture_t *resource = &package->resources.textures[i];

    Image image = LoadImageFromMemory(resource->ext, resource->buffer, resource->size);
    if (!IsImageValid(image)) {
      log_error("[backend/reylib] fail to load image from memory at %d", i);
      continue;
    }

    Texture texture = LoadTextureFromImage(image);
    if (!IsTextureValid(texture)) {
      log_error("[backend/reylib] fail to load texture from image at %d", i);
      continue;
    }

    Texture *handler = arena_push(arena, Texture, 1);
    *handler = texture;
    package->handlers.textures[i] = (texture_t){
      .handler = handler,
    };
  }
}
