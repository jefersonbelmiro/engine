#include "core/arena.h"
#include "core/defs.h"
#include "core/package.h"
#include "core/string.h"

API char *platform_binary_path()
{
  return "./";
}

static arena_t *g_arena;

void show_cmd_line_help()
{
  printn(
    "core_pack\n"
    " usage   : core_pack [options]\n"
    " options :\n"
    "    -h  --help          : show command line usage help\n"
  );
}

bool generate()
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
  package_def_append_texture(&def, "TEXTURE_001", "resources/texture/001.jpg");
  package_def_append_atlas(&def, "ATLAS_01_64", "resources/texture/atlas_01_64.png", vec2(64, 64));
  package_def_append_font(&def, "FONT_MONOGRAM", "resources/font/monogram.ttf");
  package_def_append_sound(&def, "SOUND_POWERUP_01", "resources/sounds/sfx/sfx_powerup_01.wav", 0.5, 2);
  package_def_append_music(&def, "MUSIC_MENU_01", "resources/sounds/music/menu_01.mp3", 0.5);

  package_def_write_header(&def, "core");
  package_def_make(&def, &pkg, g_arena);
  package_write(&pkg, "core", g_arena);

  return true;
}

int main(int argc, char **argv)
{
  g_arena = arena_create(MB(8), "core_pack");

  for (int i = 0; i < argc; i++) {
    if (str_eq(argv[i], "-h") || str_eq(argv[i], "--help")) {
      show_cmd_line_help();
      return 0;
    }
  }
  
  if (!generate()) {
    return 1;
  }
  return 0;
}

