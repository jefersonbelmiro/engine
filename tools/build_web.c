#include <stddef.h>
#define MAX_TEXTFORMAT_BUFFERS 8
#define MAX_TEXT_BUFFER_LENGTH 512
#define SO_TEXT_BUFFER_LENGTH 1024

#include "core/arena.h"
#include "core/defs.h"
#include "core/io.h"
#include "core/so.h"
#include "core/string.h"
#include "core/math.h"
#include "core/tpl_parser.h"
#include "platform/utils.h"
#include "project.h"

#define SHELL_TEMPLATE_PATH "engine/templates/web_shell.md"

arena_t *g_arena;
bool genenrate_shell();

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
    "build_web\n"
    " usage   : build_web [options]\n"
    " options :\n"
    "    -h  --help          : show command line usage help\n"
    "    -ll --log-level     : log level (1..5)\n"
    "    -t  --target=[TYPE] : target type: build | release\n"
    "        --release       : release build\n"
    "    -r  --run           : run web server\n"
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

bool build_raylib()
{
  char *lib_path = raylib_lib_path(g_arena);
  char *build_path = str_format("%s/build_web", lib_path);

  char *lib_file = str_format("%s/build_web/raylib/libraylib.a", lib_path);
  if (io_file_exists(lib_file)) {
    return true;
  }

  printn("[INFO] libraylib.a not found, building...");

  if (!io_dir_exists(build_path) && !io_mkdir(build_path)) {
    return false;
  }

  char *emsdk_path = emsdk_lib_path(g_arena);
  char *toolchain_file = str_format("%s/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake", emsdk_path);

  char *config_flags = project_ptr()->support_fileformat_jpg ? " -DSUPPORT_FILEFORMAT_JPG=1" : "";

  char *cmake_format = 
    "cmake -S %s -B %s"
    " -DCMAKE_TOOLCHAIN_FILE=\"%s\""
    " -DCMAKE_BUILD_TYPE=Release "
    " -DBUILD_EXAMPLES=OFF "
    " -DPLATFORM=Web "
    " -DCMAKE_C_FLAGS=\"-DSUPPORT_MODULE_RMODELS=0 -DSUPPORT_FILEFORMAT_XM=0 -DSUPPORT_FILEFORMAT_MOD=0 -DSUPPORT_FILEFORMAT_GIF=0 -DSUPPORT_FILEFORMAT_DDS=0 -DSUPPORT_FILEFORMAT_BMP=0 -DSUPPORT_FILEFORMAT_QOA=0 -DSUPPORT_SCREEN_CAPTURE=0 -DSUPPORT_AUTOMATION_EVENTS=0 -DSUPPORT_IMAGE_EXPORT=0 -DSUPPORT_IMAGE_GENERATION=0 -DSUPPORT_MESH_GENERATION=0 -DSUPPORT_CLIPBOARD_IMAGE=0 -DSUPPORT_TRACELOG=0%s\" "
  ;
  size_t cmake_len = strlen(cmake_format) + strlen(lib_path) + strlen(build_path) + strlen(toolchain_file) + strlen(config_flags);
  char *cmake_cmd = arena_push(g_arena, char, cmake_len);
  snprintf(cmake_cmd, cmake_len, cmake_format, lib_path, build_path, toolchain_file, config_flags);

  if (!so_exec(cmake_cmd)) {
    printn("[error] cmake failed");
    return false;
  }

  char *make_cmd = str_format("make -C %s -j$(nproc)", build_path);
  if (!so_exec(make_cmd)) {
    printn("[error] make failed");
    return false;
  }

  return true;
}

bool compile_main()
{
  char *emsdk_env = emsdk_env_path(g_arena);
  char *flags = get_compile_flags(BACKEND_RAYLIB, PLATFORM_WEB, g_arena);

  char *cmd_format = 
    "source %s >/dev/null 2>&1 && "
    " emcc  -Wall -std=gnu11 -O3 -flto=auto -I./engine/src -I./src %s src/main.c"
    " -s USE_GLFW=3 "
    " -s ASYNCIFY "
    " -s ASYNCIFY_STACK_SIZE=16384 "
    " -s FORCE_FILESYSTEM=1 "
    " -lidbfs.js "
    " -s TOTAL_MEMORY=90MB "
    " -s ALLOW_MEMORY_GROWTH=0 "
    // @TODO
    //"--preload-file \"$RESOURCES_SOURCE_DIR\"@.\"$RESOURCES_TARGET_DIR\" "
    " --preload-file \"resources/packages/\"@.\"./\" "
    " --shell-file \"build/tmp/web/shell.html\" "
    " -o \"build/web/index.html\" "
  ;
  size_t cmd_len = strlen(cmd_format) + strlen(emsdk_env) + strlen(flags);
  char *cmd_buffer = arena_push(g_arena, char, cmd_len);
  snprintf(cmd_buffer, cmd_len, cmd_format, emsdk_env, flags);

  // printn("cmd:\n%s\n", cmd_buffer);
  // return true;

  if (!so_exec(cmd_buffer)) {
    printn("[error] compile main failed");
    return false;
  }

  return true;
}

bool compile(options_t *options)
{
  project_t *project = project_ptr();
  if (options->log_level >= LOG_LEVEL_DEBUG) {
    printn("project:");
    printn("  name     : %s", project->name);
  }

  if(str_is_empty(project->name)) {
    log_error("project not defined name");
    return false;
  }

  if (options->log_level >= LOG_LEVEL_DEBUG) {
    printn("compile:");
    printn("  target    : %s", target_to_str(options->target));
    printn("  run       : %s", options->run ? "true" : "false");
    printn("  log_level : %d", options->log_level);
  }

  if (!build_raylib()) {
    return false;
  }

  if (!genenrate_shell()) {
    return false;
  }

  if (!compile_main()) {
    return false;
  }

  return true;
}

bool run_server()
{
  return so_exec("cd ./build/web && python -m http.server 8080");
}

int main(int argc, char **argv)
{
  options_t options = {0};
  g_arena = arena_create(KB(64), "build_web");

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

  if (!io_dir_exists("build/web") && !io_mkdir_recursive("build/web")) {
    return 1;
  }

  if (!compile(&options)) {
    return 1;
  }
  if (options.log_level) {
    printn("build created: ./build/web/index.html");
  }
  if (options.run && run_server()) {
    return 1;
  }
  return 0;
}

bool genenrate_shell()
{
  project_t *project = project_ptr();

  int data_size = 0;
  unsigned char *data = io_load_file_data(SHELL_TEMPLATE_PATH, &data_size, g_arena);
  if (!data) {
    printn("[error] fail to load %s", SHELL_TEMPLATE_PATH);
    return false;
  }

  tpl_var_t vars[] = {
    { "NAME", project->name },
  };

  tpl_file_t *files = NULL;
  int file_count = tpl_parse(g_arena, (char *)data, &files);

  for (int i = 0; i < file_count; i++) {
    char *content = tpl_render(g_arena, files[i].content, vars, countof(vars));

    if (!io_dir_exists("build/tmp/web") && !io_mkdir_recursive("build/tmp/web")) {
      return false;
    }
    if (!io_save_file_data(files[i].path, content, (int)strlen(content))) {
      return false;
    }
  }

  return true;
}
