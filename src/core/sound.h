#pragma once

#include "core/arena.h"
#include "core/defs.h"

typedef enum {
  SOUND_POLICY_MULTI,
  SOUND_POLICY_RESET,
} sound_policy_t;

typedef struct {
  void          *sound;
  sound_policy_t policy;
  float          volume;
  u8             max_active;
  u8             active;
} sound_entry_t;

typedef struct {
  void          *music;
  float          volume;
} music_entry_t;

typedef struct {
  char pad;
  // sound_entry_t sounds[RESOURCE_SOUND_COUNT];
  // music_entry_t musics[RESOURCE_MUSIC_COUNT];
  // Music *current_music;
} sound_manager_t;

GLOBAL sound_manager_t *g_sound_manager;

API sound_manager_t *sound_ptr(void) { return g_sound_manager; }

u32 sound_memory_size()
{
  u32 size = 0;
  size += sizeof(sound_manager_t);
  return size;
}

API void sound_config(u16 id, sound_policy_t policy, float volume, u8 max_active)
{
  (void) id; (void) policy; (void) volume; (void) max_active;
  // sound_entry_t *e = &g_sound_manager->sounds[id];
  // e->sound      = resource_sound(id);
  // e->policy     = policy;
  // e->volume     = volume;
  // e->max_active = max_active;
  // e->active     = 0;
  // SetSoundVolume(e->sound, volume);
}

API void music_config(u16 id, float volume)
{
  (void) id; (void) volume;
  // music_entry_t *e = &g_sound_manager->musics[id];
  // e->music      = resource_music(id);
  // e->volume     = volume;
  // SetMusicVolume(e->music, volume);
}

API void sound_process(void)
{
  // Music *music = g_sound_manager->current_music;
  // if (music) {
  //   UpdateMusicStream(*music);
  // }
  // for (u16 i = 0; i < RESOURCE_SOUND_COUNT; i++) {
  //   g_sound_manager->sounds[i].active = 0;
  // }
}

API void sound__play_entry(sound_entry_t *e)
{
  (void) e;
  // if (e->policy == SOUND_POLICY_MULTI) {
  //   if (e->active < e->max_active) {
  //     PlaySound(e->sound);
  //     e->active++;
  //   }
  // } else { // SOUND_POLICY_RESET
  //   if (e->active > 0) StopSound(e->sound);
  //   PlaySound(e->sound);
  //   e->active = 1;
  // }
}

API void sound_play(u16 id)
{
  (void) id;
  // sound__play_entry(&g_sound_manager->sounds[id]);
}

API void music_play(u16 id)
{
  (void) id;
  // if (g_sound_manager->current_music) {
  //   StopMusicStream(*g_sound_manager->current_music);
  // }
  // g_sound_manager->current_music = &g_sound_manager->musics[id].music;
  // PlayMusicStream(*g_sound_manager->current_music);
}

API void music_stop()
{
  // if (g_sound_manager->current_music) {
  //   StopMusicStream(*g_sound_manager->current_music);
  //   g_sound_manager->current_music =  NULL;
  // }
}

API void sound_init(arena_t *arena)
{
  g_sound_manager = arena_push_zero(arena, sound_manager_t, 1);
}

API void sound_start()
{
  // for (u8 i = 0; i < RESOURCE_SOUND_COUNT; i++) {
  //   resource_sound_config_t config = g_sound_config[i];
  //   sound_policy_t policy = config.max_active > 1 ? SOUND_POLICY_MULTI : SOUND_POLICY_RESET; 
  //   sound_config(i,  policy, config.volume, config.max_active);
  // }
  //
  // for (u8 i = 0; i < RESOURCE_MUSIC_COUNT; i++) {
  //   resource_music_config_t config = g_music_config[i];
  //   music_config(i, config.volume);
  // }
}
