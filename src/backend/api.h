#pragma once

#include "backend/codes.h"
#include "core/defs.h"
#include "core/package.h"

API void             backend_init();
API void             backend_fini();
API void             backend_main();
API void             backend_main_loop();

API screen_size_t    screen_size();
API bool             window_resized();

// input primitives (level state, backend-neutral)
API bool   input_key_pressed(key_code_t key);
API bool   input_key_down(key_code_t key);
API bool   input_key_released(key_code_t key);
API bool   input_key_up(key_code_t key);

API bool   input_mouse_button_pressed(mouse_button_t button);
API bool   input_mouse_button_down(mouse_button_t button);
API bool   input_mouse_button_released(mouse_button_t button);
API bool   input_mouse_button_up(mouse_button_t button);
API vec2_t input_mouse_position();
API vec2_t input_mouse_delta();
API vec2_t input_mouse_wheel();

API bool  input_gamepad_available(int gamepad);
API bool  input_gamepad_button_pressed(int gamepad, gamepad_button_t button);
API bool  input_gamepad_button_down(int gamepad, gamepad_button_t button);
API bool  input_gamepad_button_released(int gamepad, gamepad_button_t button);
API bool  input_gamepad_button_up(int gamepad, gamepad_button_t button);
API float input_gamepad_axis(int gamepad, gamepad_axis_t axis);
API int   input_gamepad_count();

API int    input_touch_count();
API vec2_t input_touch_position(int index);

API void load_package_handlers(package_t *package);

API void draw_rectangle_lines(rect_t rec, float thick, color_t color);

API void draw_texture(texture_t *texture, vec2_t position, float rotation,
                      float scale, color_t tint);
API void draw_texture_center(texture_t *texture, vec2_t position, float rotation,
                             float scale, color_t tint);
API void draw_texture_rect(texture_t *texture, rect_t source, rect_t dest,
                           vec2_t origin, float rotation, color_t tint);
API void draw_atlas(atlas_t *atlas, u32 idx, vec2_t pos, float scale,
                    float rotation, color_t tint);
API void draw_atlas_center(atlas_t *atlas, u32 idx, vec2_t pos, float scale,
                    float rotation, color_t tint);
