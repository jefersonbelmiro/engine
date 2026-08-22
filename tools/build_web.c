#include <stddef.h>
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

char *get_backend_flags(u8 backend)
{
  switch (backend) {
    case BACKEND_RAYLIB:
      return "-include backend/raylib.h -I./../raylib/src -L./../raylib/build/raylib -lraylib -lm -lX11";
    default:
      return NULL;
  }
}

char *get_platform_flags()
{
  return "-include platform/web.h "
         " -DPLATFORM=PLATFORM_WEB";
}

bool compile(options_t *options)
{

  genenrate_shell();
  return 1;

  if (options->log_level >= LOG_LEVEL_DEBUG) {
    printn("project:");
    printn("  name     : %s", g_project.name);
    printn("  binary   : %s", g_project.binary);
  }

  if(str_is_empty(g_project.name) || str_is_empty(g_project.binary)) {
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

  char *cmd = str_format(
    "gcc %s src/main.c %s -o build/web/%s -I./src",
    platform_flags,
    backend_flags,
    g_project.binary
  );

  return so_exec(cmd);
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
  // if (options.log_level) {
  //   printn("executable created: ./build/web/%s", g_project.binary);
  // }
  // if (options.run && !so_exec("./build/web/%s", g_project.binary)) {
  //   return 1;
  // }
  return 0;
}

bool genenrate_shell()
{
  char format[] = 
"<!doctype html>"
"<html lang='en'>"
"  <head>"
"    <meta charset='utf-8' />"
"    <meta"
"      name='viewport'"
"      content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0'"
"    />"
"    <title>%s</title>"
"    <style>"
"      * {"
"        margin: 0;"
"        padding: 0;"
"        box-sizing: border-box;"
"      }"
""
"      html,"
"      body {"
"        background: #000;"
"        height: 100%;"
"        width: 100%;"
"        overflow: hidden;"
"        touch-action: none;"
"        user-select: none;"
"      }"
""
"      #canvas {"
"        display: block;"
"        position: fixed;"
"        top: 0;"
"        left: 0;"
"        width: 100%;"
"        height: 100%;"
"      }"
""
"      #loading-overlay {"
"        position: fixed; top: 0; left: 0;"
"        width: 100%; height: 100%;"
"        background: #000;"
"        display: flex; flex-direction: column;"
"        align-items: center; justify-content: center;"
"        z-index: 1000;"
"        color: #fff;"
"        font-family: monospace;"
"      }"
"      #loading-spinner {"
"        width: 48px; height: 48px;"
"        border: 4px solid #222;"
"        border-top-color: #ccc;"
"        border-radius: 50%;"
"        animation: spin .8s linear infinite;"
"        margin-bottom: 16px;"
"      }"
"      @keyframes spin { to { transform: rotate(360deg); } }"
"      #loading-text { font-size: 14px; opacity: .7; }"
"    </style>"
"  </head>"
""
"  <body>"
""
"    <div id='loading-overlay'>"
"      <div id='loading-spinner'></div>"
"      <div id='loading-text'>Loading...</div>"
"    </div>"
""
"    <canvas"
"      class='emscripten'"
"      id='canvas'"
"      tabindex='-1'"
"      oncontextmenu='return false'"
"    ></canvas>"
"    <script>"
"      document.addEventListener('contextmenu', function (event) {"
"        event.preventDefault();"
"      });"
"      document.addEventListener('keydown', function(e) {"
"        if (e.key === 'Alt' || e.key === 'Shift' || e.key === 'Tab' || e.key === 'F11') {"
"          e.preventDefault();"
"        }"
"      });"
"      document.addEventListener('keydown', function(e) {"
"        if (e.key === 'Alt' || e.key === 'Shift' || e.key === 'Tab' || e.key === 'F11') {"
"          e.preventDefault();"
"        }"
"      });"
""
"      var loadingText = document.getElementById('loading-text');"
""
"      var Module = {"
"        canvas: document.getElementById('canvas'),"
"        print: function (text) {"
"          console.log(text);"
"        },"
"        printErr: function (text) {"
"          console.warn(text);"
"        },"
"        setStatus: function(text) {"
"          if (text) loadingText.textContent = text;"
"        },"
"        postRun: [function() {"
"          loadingText = null;"
"        }]"
"      };"
"    </script>"
"    {{{ SCRIPT }}}"
"  </body>"
"</html>";

  size_t arena_offet = g_arena->offset;
  size_t len = strlen(format);
  char *buffer = arena_push(g_arena, char, len);
  snprintf(buffer, len, format, "main");

  if (!io_dir_exists("build/tmp/web") && !io_mkdir_recursive("build/tmp/web")) {
    goto fail;
  }

  if (!io_save_file_data("build/tmp/web/shell.html", buffer, len)) {
    goto fail;
  }

  arena_restore(g_arena, arena_offet);
  return true;

fail:
  arena_restore(g_arena, arena_offet);
  return false;
}
