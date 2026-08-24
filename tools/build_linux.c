#define MAX_TEXTFORMAT_BUFFERS 4
#define MAX_TEXT_BUFFER_LENGTH 256

#include "core/arena.h"
#include "core/defs.h"
#include "core/io.h"
#include "core/so.h"
#include "core/string.h"
#include "core/math.h"
#include "project.h"

arena_t *g_arena;

typedef enum {
  TARGET_NONE,
  TARGET_DEBUG,
  TARGET_RELEASE,
} target_type_t;

typedef struct {
  target_type_t target;
  u8            backend;
  u8            log_level;
  bool          run;
} options_t;

void show_cmd_line_help()
{
  printn(
    "build_linux\n"
    " usage   : build_linux [options]\n"
    " options :\n"
    "    -h  --help          : show command line usage help\n"
    "    -ll --log-level     : log level (1..5)\n"
    "    -t  --target=[TYPE] : target type: debug | release\n"
    "        --release       : release build\n"
    "    -r  --run           : run binary\n"
  );
}

target_type_t str_to_target(char *str)
{
  if (str_eq(str, "debug")) {
    return TARGET_DEBUG;
  }
  else if (str_eq(str, "release")) {
    return TARGET_RELEASE;
  }
  return TARGET_NONE;
}

char *target_to_str(target_type_t type)
{
  switch (type) {
    case TARGET_NONE:
      return NULL;
    case TARGET_RELEASE:
      return "release";
    case TARGET_DEBUG:
      return "debug";
  }
}

char *get_backend_flags(u8 backend) {
  project_t *project = project_ptr();

  switch (backend) {
    case BACKEND_RAYLIB: {
      char *raylib_base = arena_push(g_arena, char, 256);
      so_resolve_home(project->lib_raylib_path, raylib_base);
      char *inc_path = str_format("%s/src", raylib_base);
      char *lib_path = str_format("%s/build/raylib", raylib_base);
      char *deps = "-lraylib -lm -lX11";
      return str_format("-DBACKEND=BACKEND_RAYLIB -include backend/raylib.h -I%s -L%s %s", inc_path, lib_path, deps);
    default:
      return NULL;
    }
  }
}

char *get_platform_flags()
{
  return "-include platform/linux.h "
         "-DPLATFORM=PLATFORM_LINUX";
}

// void get_sources_line()
// {
//   char **source_files = arena_push(g_arena, char*, 1);
//   u16    source_count = 0;
//   io_find_files("src", ".c", source_files, &source_count, g_arena);
//
//   u16 total_len = 0;
//   for (u16 i = 0; i < source_count; i++) {
//     total_len += strlen(source_files[i]) + 1; // +1 for space
//     printn(" source: %s", source_files[i]);
//   }
//
//   char *sources_line = arena_push(g_arena, char, total_len + 1);
//   sources_line[0] = 0x0;
//
//   char *line_ptr = sources_line;
//   for (u16 i = 0; i < source_count; i++) {
//     u16 len = strlen(source_files[i]);
//
//     mem_copy(source_files[i], line_ptr, len);
//     line_ptr += len;
//
//     if (i < source_count - 1) {
//       *line_ptr = ' ';
//       line_ptr++;
//     }
//   }
//
//   line_ptr = 0x0;
//
//   printn("sources_line: %s", sources_line);
// }

bool compile(options_t *options)
{
  project_t *project = project_ptr();
  if (options->log_level >= LOG_LEVEL_DEBUG) {
    printn("project:");
    printn("  name     : %s", project->name);
    printn("  binary   : %s", project->binary);
  }

  if(str_is_empty(project->name) || str_is_empty(project->binary)) {
    log_error("project not defined name or binary");
    return false;
  }

  if (options->log_level >= LOG_LEVEL_DEBUG) {
    printn("compile:");
    printn("  target    : %s", target_to_str(options->target));
    printn("  run       : %s", options->run ? "true" : "false");
    printn("  log_level : %d", options->log_level);
  }

  char *backend_flags = get_backend_flags(BACKEND_RAYLIB);
  char *platform_flags = get_platform_flags();

  char *target_flags = "";
  switch(options->target) {
    case TARGET_DEBUG:
      target_flags = "-DDEBUG=1 -DDEBUG_MEMORY_USAGE=1 -g -Wall -Wextra -std=c11 -O0 -pedantic";
      break;
    case TARGET_RELEASE:
      target_flags = "-DRELEASE=1 -Wall -Wextra -std=c11 -flto=auto -O3 -pedantic";
      break;
      default: break;
  }

  char *cmd_format = "gcc src/main.c %s -I./engine/src -I./src %s %s "
                     " -o build/linux/%s.x86_64";

  size_t cmd_len = strlen(cmd_format) + strlen(target_flags) +
                   strlen(platform_flags) + strlen(backend_flags) +
                   strlen(project->binary);
  char *cmd_buffer = arena_push(g_arena, char, cmd_len);
  snprintf(
    cmd_buffer, cmd_len, cmd_format, 
    target_flags, platform_flags, backend_flags, project->binary
  );

  return so_exec(cmd_buffer);
}

int main(int argc, char **argv)
{
  options_t options = {0};
  g_arena = arena_create(KB(64), "build_linux");
  project_t *project = project_ptr();

  for (int i = 0; i < argc; i++) {
    bool is_last = i == argc - 1;
    if (str_eq(argv[i], "-h") || str_eq(argv[i], "--help")) {
      show_cmd_line_help();
      return 0;
    }

    // log level
    if ((str_eq(argv[i], "-ll") || str_eq(argv[i], "--log-level")) && !is_last) {
      options.log_level = min(5, (u16)atoi(argv[i + 1]));
      i++;
    }

    // target build type
    // -t, --target [TYPE] -t=[TYPE] --target=[TYPE]
    if ((str_eq(argv[i], "-t") || str_eq(argv[i], "--target")) && !is_last) {
      options.target = str_to_target(argv[i + 1]);
      if (options.target == TARGET_NONE) {
        printn("[error]: invalid target: %s", argv[i] + 3);
        return 1;
      }
      i++;
    }
    else if (str_start_with(argv[i], "-t=")) {
      options.target = str_to_target(argv[i] + 3);
      if (options.target == TARGET_NONE) {
        printn("[error]: invalid target: %s", argv[i] + 3);
        return 1;
      }
    }
    else if (str_start_with(argv[i], "--target=")) {
      options.target = str_to_target(argv[i] + 9);
      if (options.target == TARGET_NONE) {
        printn("[error]: invalid target: %s", argv[i] + 3);
        return 1;
      }
    }

    // run binary
    if (str_eq(argv[i], "-r") || str_eq(argv[i], "--run")) {
      options.run = true;
    }
  }

  if (!io_dir_exists("build/linux") && !io_mkdir_recursive("build/linux")) {
    return 1;
  }

  if (!compile(&options)) {
    return 1;
  }
  if (options.log_level) {
    printn("executable created: ./build/linux/%s.x86_64", project->binary);
  }
  if (options.run && !so_exec("./build/linux/%s.x86_64", project->binary)) {
    return 1;
  }
  return 0;
}
