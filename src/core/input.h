#pragma once

#include "backend/api.h"
#include "backend/codes.h"
#include "core/arena.h"
#include "core/defs.h"
#include "core/math.h"

#define INPUT_MAX_ACTIONS 64
#define INPUT_MAX_BINDINGS 8

typedef u16 action_id_t;
#define ACTION_NONE ((action_id_t)-1)

typedef enum {
  INPUT_SOURCE_KEY,
  INPUT_SOURCE_MOUSE_BUTTON,
  INPUT_SOURCE_GAMEPAD_BUTTON,
  INPUT_SOURCE_GAMEPAD_AXIS,
  INPUT_SOURCE_TOUCH,
} input_source_t;

typedef struct {
  input_source_t source;
  int  code;      // key_code_t / mouse_button_t / gamepad_button_t / gamepad_axis_t
  int  gamepad;   // -1 = any connected gamepad
  float deadzone; // axes only
  float scale;    // -1 = invert axis, else binding strength
} input_binding_t;

typedef struct {
  const char *name;
  u8 binding_count;
  input_binding_t bindings[INPUT_MAX_BINDINGS];
  bool  pressed;
  bool  just_pressed;
  bool  just_released;
  float strength;
} input_action_t;

typedef struct {
  input_action_t actions[INPUT_MAX_ACTIONS];
  u16 count;
  input_layer_t layer;
} input_t;

GLOBAL input_t *g_input;

API input_t *input_ptr(void)
{
  return g_input;
}

API u32 input_memory_size(void)
{
  return sizeof(input_t);
}

API action_id_t input_register_action(const char *name)
{
  input_t *input = input_ptr();

  for (u16 i = 0; i < input->count; i++) {
    if (strcmp(input->actions[i].name, name) == 0) {
      return i;
    }
  }

  if (input->count >= INPUT_MAX_ACTIONS) {
    log_error("input: too many actions (%d max)", INPUT_MAX_ACTIONS);
    return ACTION_NONE;
  }

  action_id_t id = input->count++;
  input->actions[id].name = name;
  return id;
}

API action_id_t input_action_id(const char *name)
{
  input_t *input = input_ptr();

  for (u16 i = 0; i < input->count; i++) {
    if (strcmp(input->actions[i].name, name) == 0) {
      return i;
    }
  }

  return ACTION_NONE;
}

API input_action_t *input_action(action_id_t id)
{
  return &input_ptr()->actions[id];
}

API void input_map_clear(action_id_t id)
{
  input_action(id)->binding_count = 0;
}

API void input_map_bind(action_id_t id, input_binding_t binding)
{
  input_action_t *action = input_action(id);
  if (action->binding_count >= INPUT_MAX_BINDINGS) {
    log_warn("input: action '%s' max bindings reached", action->name);
    return;
  }
  action->bindings[action->binding_count++] = binding;
}

API bool input_action_pressed(action_id_t id)
{
  return input_action(id)->pressed;
}

API bool input_action_just_pressed(action_id_t id)
{
  return input_action(id)->just_pressed;
}

API bool input_action_just_released(action_id_t id)
{
  return input_action(id)->just_released;
}

API float input_action_strength(action_id_t id)
{
  return input_action(id)->strength;
}

API vec2_t input_vector(action_id_t left, action_id_t right, action_id_t up, action_id_t down)
{
  float x = input_action_strength(right) - input_action_strength(left);
  float y = input_action_strength(down) - input_action_strength(up);

  float magnitude = m_absf(x) + m_absf(y);
  if (magnitude > 1.0f) {
    x /= magnitude;
    y /= magnitude;
  }

  return (vec2_t){ .x = x, .y = y };
}

API bool input_layer_handled(input_layer_t mask)
{
  return (input_ptr()->layer & mask) != 0;
}

API bool input_layer_any_hud_handled()
{
  input_layer_t mask = INPUT_LAYER_HUD | INPUT_LAYER_HUD_FG;
  return (input_ptr()->layer & mask) != 0;
}

API void input_layer_set(input_layer_t mask)
{
  input_ptr()->layer |= mask;
}

API void input_process()
{
  input_t *input = input_ptr();

  input->layer = INPUT_LAYER_NONE;

  int gamepad_count = input_gamepad_count();

  for (u16 a = 0; a < input->count; a++) {
    input_action_t *action = &input->actions[a];
    bool prev = action->pressed;
    bool pressed = false;
    float strength = 0.0f;

    for (u8 b = 0; b < action->binding_count; b++) {
      input_binding_t *bind = &action->bindings[b];
      bool active = false;
      float s = 1.0f;

      switch (bind->source) {
        case INPUT_SOURCE_KEY:
          active = input_key_down((key_code_t)bind->code);
          break;

        case INPUT_SOURCE_MOUSE_BUTTON:
          active = input_mouse_button_down((mouse_button_t)bind->code);
          break;

        case INPUT_SOURCE_GAMEPAD_BUTTON:
          if (bind->gamepad < 0) {
            for (int g = 0; g < gamepad_count && !active; g++) {
              active = input_gamepad_button_down(g, (gamepad_button_t)bind->code);
            }
          } else if (bind->gamepad < gamepad_count) {
            active = input_gamepad_button_down(bind->gamepad, (gamepad_button_t)bind->code);
          }
          break;

        case INPUT_SOURCE_GAMEPAD_AXIS: {
          float magnitude = 0.0f;
          if (bind->gamepad < 0) {
            for (int g = 0; g < gamepad_count; g++) {
              float raw = input_gamepad_axis(g, (gamepad_axis_t)bind->code) * bind->scale;
              magnitude = max(magnitude, m_absf(raw));
            }
          } else if (bind->gamepad < gamepad_count) {
            float raw = input_gamepad_axis(bind->gamepad, (gamepad_axis_t)bind->code) * bind->scale;
            magnitude = m_absf(raw);
          }
          if (magnitude > bind->deadzone) {
            active = true;
            s = (magnitude - bind->deadzone) / (1.0f - bind->deadzone);
          }
          break;
        }

        case INPUT_SOURCE_TOUCH:
          active = input_touch_count() > 0;
          break;
      }

      if (active) {
        pressed = true;
        strength = max(strength, s);
      }
    }

    action->pressed = pressed;
    action->strength = strength;
    action->just_pressed = pressed && !prev;
    action->just_released = !pressed && prev;
  }
}

API void input_register_ui_defaults()
{
  input_register_action("ui_accept");
  input_register_action("ui_cancel");
  input_register_action("ui_left");
  input_register_action("ui_right");
  input_register_action("ui_up");
  input_register_action("ui_down");

  input_map_bind(input_action_id("ui_accept"), (input_binding_t){ .source = INPUT_SOURCE_KEY, .code = INPUT_KEY_ENTER });
  input_map_bind(input_action_id("ui_accept"), (input_binding_t){ .source = INPUT_SOURCE_KEY, .code = INPUT_KEY_SPACE });
  input_map_bind(input_action_id("ui_accept"), (input_binding_t){ .source = INPUT_SOURCE_GAMEPAD_BUTTON, .code = INPUT_GAMEPAD_BUTTON_RIGHT_FACE_DOWN, .gamepad = -1 });

  input_map_bind(input_action_id("ui_cancel"), (input_binding_t){ .source = INPUT_SOURCE_KEY, .code = INPUT_KEY_ESCAPE });
  input_map_bind(input_action_id("ui_cancel"), (input_binding_t){ .source = INPUT_SOURCE_GAMEPAD_BUTTON, .code = INPUT_GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, .gamepad = -1 });

  input_map_bind(input_action_id("ui_left"), (input_binding_t){ .source = INPUT_SOURCE_KEY, .code = INPUT_KEY_LEFT });
  input_map_bind(input_action_id("ui_left"), (input_binding_t){ .source = INPUT_SOURCE_GAMEPAD_BUTTON, .code = INPUT_GAMEPAD_BUTTON_LEFT_FACE_LEFT, .gamepad = -1 });
  input_map_bind(input_action_id("ui_left"), (input_binding_t){ .source = INPUT_SOURCE_GAMEPAD_AXIS, .code = INPUT_GAMEPAD_AXIS_LEFT_X, .gamepad = -1, .scale = -1.0f, .deadzone = 0.2f });

  input_map_bind(input_action_id("ui_right"), (input_binding_t){ .source = INPUT_SOURCE_KEY, .code = INPUT_KEY_RIGHT });
  input_map_bind(input_action_id("ui_right"), (input_binding_t){ .source = INPUT_SOURCE_GAMEPAD_BUTTON, .code = INPUT_GAMEPAD_BUTTON_LEFT_FACE_RIGHT, .gamepad = -1 });
  input_map_bind(input_action_id("ui_right"), (input_binding_t){ .source = INPUT_SOURCE_GAMEPAD_AXIS, .code = INPUT_GAMEPAD_AXIS_LEFT_X, .gamepad = -1, .deadzone = 0.2f });

  input_map_bind(input_action_id("ui_up"), (input_binding_t){ .source = INPUT_SOURCE_KEY, .code = INPUT_KEY_UP });
  input_map_bind(input_action_id("ui_up"), (input_binding_t){ .source = INPUT_SOURCE_GAMEPAD_BUTTON, .code = INPUT_GAMEPAD_BUTTON_LEFT_FACE_UP, .gamepad = -1 });
  input_map_bind(input_action_id("ui_up"), (input_binding_t){ .source = INPUT_SOURCE_GAMEPAD_AXIS, .code = INPUT_GAMEPAD_AXIS_LEFT_Y, .gamepad = -1, .scale = -1.0f, .deadzone = 0.2f });

  input_map_bind(input_action_id("ui_down"), (input_binding_t){ .source = INPUT_SOURCE_KEY, .code = INPUT_KEY_DOWN });
  input_map_bind(input_action_id("ui_down"), (input_binding_t){ .source = INPUT_SOURCE_GAMEPAD_BUTTON, .code = INPUT_GAMEPAD_BUTTON_LEFT_FACE_DOWN, .gamepad = -1 });
  input_map_bind(input_action_id("ui_down"), (input_binding_t){ .source = INPUT_SOURCE_GAMEPAD_AXIS, .code = INPUT_GAMEPAD_AXIS_LEFT_Y, .gamepad = -1, .deadzone = 0.2f });
}

API void input_init(arena_t *arena)
{
  g_input = arena_push_zero(arena, input_t, 1);

  input_register_ui_defaults();
}
