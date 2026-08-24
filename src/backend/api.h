#pragma once

#include "backend/codes.h"
#include "core/defs.h"
#include "core/package.h"

API void             backend_init();
API void             backend_fini();
API void             backend_main();
API void             backend_main_loop();

API screen_size_t    be_screen_size();
API bool             be_window_resized();

// input primitives (level state, backend-neutral)
API bool   be_key_down(be_key_t key);
API bool   be_mouse_button_down(be_mouse_button_t button);
API vec2_t be_mouse_position();
API vec2_t be_mouse_delta();
API vec2_t be_mouse_wheel();
API int    be_gamepad_count();
API bool   be_gamepad_button_down(int gamepad, be_gamepad_button_t button);
API float  be_gamepad_axis(int gamepad, be_gamepad_axis_t axis);
API int    be_touch_count();
API vec2_t be_touch_position(int index);

API void load_package_handlers(package_t *package);

API void draw_rectangle_lines(rect_t rec, float thick, color_t color);


