#include "core/arena.h"
#include "core/defs.h"
#include "core/io.h"
#include "core/string.h"
#include "platform/utils.h"
#include "stdlib.h"

arena_t *g_arena;
void show_cmd_line_help()
{
  printn(
    "gen_compile_flags\n"
    " usage   : gen_compile_flags [options]\n"
    " options :\n"
    "    -h  --help          : show command line usage help\n"
    "    -f  --force         : overwrite if file exists\n"
  );
}

bool generate(bool overwrite)
{
  char format[] = "-std=gnu11\n"
                  "-I./src\n"
                  "%s\n"
                  "-DDEBUG\n"
                  "-DLOG_LEVEL=5\n"
                  "-DDEBUG_MEMORY_USAGE\n"
                  "-DARENA_FALLBACK_MALLOC\n"
                  "-DHOT_RELOAD\n"
                  "-DMODULE_BUILD\n"
                  "-DPLATFORM=PLATFORM_LINUX\n"
                  "-Wall\n"
                  "-Wextra";

  size_t arena_offet = g_arena->offset;

  char *raylib_inc = raylib_inc_path(g_arena);
  char *emsdk_inc = emsdk_inc_path(g_arena);

  char *lib_includes = str_format("-I%s\n-I%s", raylib_inc, emsdk_inc);
  size_t len = strlen(format) + strlen(lib_includes);
  char *buffer = arena_push(g_arena, char, len);
  snprintf(buffer, len, format, lib_includes);

  printn("output:");
  printn("%s", buffer);

  if (!overwrite && io_file_exists("compile_flags.txt")) {
    printn("[error] compile_flags.txt already exists");
    goto fail;
  }

  if (!io_save_file_data("compile_flags.txt", buffer, strlen(buffer))) {
    goto fail;
  }

  arena_restore(g_arena, arena_offet);
  return true;

fail:
  arena_restore(g_arena, arena_offet);
  return false;
}

int main(int argc, char **argv)
{
  g_arena = arena_create(KB(64), "build_linux");

  bool overwrite = false;
  for (int i = 0; i < argc; i++) {
    if (str_eq(argv[i], "-h") || str_eq(argv[i], "--help")) {
      show_cmd_line_help();
      return 0;
    }
    if (str_eq(argv[i], "-f") || str_eq(argv[i], "--force")) {
      overwrite = true;
    }
  }
  
  if (!generate(overwrite)) {
    return 1;
  }
  return 0;
}

