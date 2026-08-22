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
#include "platform/utils.h"
#include "project.h"

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
    "build_linux\n"
    " usage   : build_linux [options]\n"
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

char *get_backend_flags(u8 backend) {
  project_t *project = project_ptr();

  switch (backend) {
    case BACKEND_RAYLIB: {
      char *raylib_base = arena_push(g_arena, char, 256);
      so_resolve_home(project->lib_raylib_path, raylib_base);
      char *inc_path = str_format("%s/src", raylib_base);
      char *lib_path = str_format("%s/build/raylib", raylib_base);
      char *deps = "-lraylib -lm -lX11";
      return str_format("-include backend/raylib.h -I%s -L%s %s", inc_path, lib_path, deps);
    default:
      return NULL;
    }
  }
}

bool build_raylib()
{
  char *lib_path = raylib_lib_path(g_arena);
  char *build_path = str_format("%s/build_web");

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

  char *cmake_format = 
    "cmake -S %s -B %s"
    " -DCMAKE_TOOLCHAIN_FILE=\"%s\""
    " -DCMAKE_BUILD_TYPE=Release "
    " -DBUILD_EXAMPLES=OFF "
    " -DPLATFORM=Web "
    " -DCMAKE_C_FLAGS=\"-DSUPPORT_MODULE_RMODELS=0 -DSUPPORT_FILEFORMAT_XM=0 -DSUPPORT_FILEFORMAT_MOD=0 -DSUPPORT_FILEFORMAT_GIF=0 -DSUPPORT_FILEFORMAT_DDS=0 -DSUPPORT_FILEFORMAT_BMP=0 -DSUPPORT_FILEFORMAT_QOA=0 -DSUPPORT_SCREEN_CAPTURE=0 -DSUPPORT_AUTOMATION_EVENTS=0 -DSUPPORT_IMAGE_EXPORT=0 -DSUPPORT_IMAGE_GENERATION=0 -DSUPPORT_MESH_GENERATION=0 -DSUPPORT_CLIPBOARD_IMAGE=0 -DSUPPORT_TRACELOG=0\" "
  ;
  size_t cmake_len = strlen(cmake_format) + strlen(lib_path) + strlen(build_path) + strlen(toolchain_file);
  char *cmake_cmd = arena_push(g_arena, char, cmake_len);
  snprintf(cmake_cmd, cmake_len, cmake_format, lib_path, build_path, toolchain_file);

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
  char *lib_path = raylib_lib_path(g_arena);
  char *build_path = str_format("%s/build_web");

  if (!io_dir_exists(build_path) && !io_mkdir(build_path)) {
    return false;
  }

  char *emsdk_env = emsdk_env_path(g_arena);
  char *inc = str_format("-I./src -I%s/src -include platform/web.h -include backend/raylib.h ", lib_path);
  char *deps = str_format("-L%s/raylib -lraylib", build_path);
  char *defs = "-DPLATFORM=PLATFORM_WEB";
  char *cmd_format = 
    "source %s >/dev/null 2>&1 && "
    " emcc -Wall -std=gnu11 -O3 -flto=auto %s %s %s src/main.c"
    " -s USE_GLFW=3 "
    " -s ASYNCIFY "
    " -s ASYNCIFY_STACK_SIZE=16384 "
    " -s FORCE_FILESYSTEM=1 "
    " -lidbfs.js "
    " -s TOTAL_MEMORY=90MB "
    " -s ALLOW_MEMORY_GROWTH=0 "
    // @TODO
    //"--preload-file \"$RESOURCES_SOURCE_DIR\"@.\"$RESOURCES_TARGET_DIR\" "
    " --shell-file \"build/tmp/web/shell.html\" "
    " -o \"build/web/index.html\" "
  ;
  size_t cmd_len = strlen(cmd_format) + strlen(emsdk_env) + strlen(inc) + strlen(deps) + strlen(defs);
  char *cmd_buffer = arena_push(g_arena, char, cmd_len);
  snprintf(cmd_buffer, cmd_len, cmd_format, emsdk_env, inc, deps, defs);

  // printn("cmd:\n%s\n", cmd_buffer);
  // return true;

  if (!so_exec(cmd_buffer)) {
    printn("[error] compile main failed");
    return false;
  }

  return true;
}

char *get_platform_flags()
{
  return "-include platform/web.h "
         " -DPLATFORM=PLATFORM_WEB";
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
  g_arena = arena_create(KB(64), "build_linux");

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
  char format[] = 
    "<!doctype html>\n"
    "<html lang='en'>\n"
    "  <head>\n"
    "    <meta charset='utf-8' />\n"
    "    <meta\n"
    "      name='viewport'\n"
    "      content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0'\n"
    "    />\n"
    "    <title>%s</title>\n"
    "    <style>\n"
    "      * {\n"
    "        margin: 0;\n"
    "        padding: 0;\n"
    "        box-sizing: border-box;\n"
    "      }\n"
    "\n"
    "      html,\n"
    "      body {\n"
    "        background: #000;\n"
    "        height: 100%;\n"
    "        width: 100%;\n"
    "        overflow: hidden;\n"
    "        touch-action: none;\n"
    "        user-select: none;\n"
    "      }\n"
    "\n"
    "      #canvas {\n"
    "        display: block;\n"
    "        position: fixed;\n"
    "        top: 0;\n"
    "        left: 0;\n"
    "        width: 100%;\n"
    "        height: 100%;\n"
    "      }\n"
    "\n"
    "      #loading-overlay {\n"
    "        position: fixed; top: 0; left: 0;\n"
    "        width: 100%; height: 100%;\n"
    "        background: #000;\n"
    "        display: flex; flex-direction: column;\n"
    "        align-items: center; justify-content: center;\n"
    "        z-index: 1000;\n"
    "        color: #fff;\n"
    "        font-family: monospace;\n"
    "      }\n"
    "      #loading-spinner {\n"
    "        width: 48px; height: 48px;\n"
    "        border: 4px solid #222;\n"
    "        border-top-color: #ccc;\n"
    "        border-radius: 50%;\n"
    "        animation: spin .8s linear infinite;\n"
    "        margin-bottom: 16px;\n"
    "      }\n"
    "      @keyframes spin { to { transform: rotate(360deg); } }\n"
    "      #loading-text { font-size: 14px; opacity: .7; }\n"
    "    </style>\n"
    "  </head>\n"
    "\n"
    "  <body>\n"
    "\n"
    "    <div id='loading-overlay'>\n"
    "      <div id='loading-spinner'></div>\n"
    "      <div id='loading-text'>Loading...</div>\n"
    "    </div>\n"
    "\n"
    "    <canvas\n"
    "      class='emscripten'\n"
    "      id='canvas'\n"
    "      tabindex='-1'\n"
    "      oncontextmenu='return false'\n"
    "    ></canvas>\n"
    "    <script>\n"
    "      document.addEventListener('contextmenu', function (event) {\n"
    "        event.preventDefault();\n"
    "      });\n"
    "      document.addEventListener('keydown', function(e) {\n"
    "        if (e.key === 'Alt' || e.key === 'Shift' || e.key === 'Tab' || e.key === 'F11') {\n"
    "          e.preventDefault();\n"
    "        }\n"
    "      });\n"
    "      document.addEventListener('keydown', function(e) {\n"
    "        if (e.key === 'Alt' || e.key === 'Shift' || e.key === 'Tab' || e.key === 'F11') {\n"
    "          e.preventDefault();\n"
    "        }\n"
    "      });\n"
    "\n"
    "      var loadingText = document.getElementById('loading-text');\n"
    "\n"
    "      var Module = {\n"
    "        canvas: document.getElementById('canvas'),\n"
    "        print: function (text) {\n"
    "          console.log(text);\n"
    "        },\n"
    "        printErr: function (text) {\n"
    "          console.warn(text);\n"
    "        },\n"
    "        setStatus: function(text) {\n"
    "          if (text) loadingText.textContent = text;\n"
    "        },\n"
    "        postRun: [function() {\n"
    "          loadingText = null;\n"
    "        }]\n"
    "      };\n"
    "    </script>\n"
    "    {{{ SCRIPT }}}\n"
    "  </body>\n"
    "</html>\n"
    ;

  project_t *project = project_ptr();
  size_t arena_offet = g_arena->offset;
  size_t len = strlen(format) + strlen(project->name);
  char *buffer = arena_push(g_arena, char, len);
  snprintf(buffer, len, format, project->name);

  if (!io_dir_exists("build/tmp/web") && !io_mkdir_recursive("build/tmp/web")) {
    goto fail;
  }

  if (!io_save_file_data("build/tmp/web/shell.html", buffer, strlen(buffer))) {
    goto fail;
  }

  arena_restore(g_arena, arena_offet);
  return true;

fail:
  arena_restore(g_arena, arena_offet);
  return false;
}
