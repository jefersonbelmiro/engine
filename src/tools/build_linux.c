#define MAX_TEXTFORMAT_BUFFERS 8

#include "core/defs.h"
#include "core/io.h"
#include "core/so.h"
#include "core/string.h"
#include "core/math.h"
#include "project.h"

typedef enum {
  TARGET_NONE,
  TARGET_DEBUG,
  TARGET_RELEASE,
} target_type_t;

typedef struct {
  target_type_t target;
  bool run;
  u16 log_level;
} options_t;

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

const char *target_to_str(target_type_t type)
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

bool compile(options_t *options)
{
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

  const char *cmd = str_format(
    "gcc src/main.c -o build/linux/%s -I./src",
    g_project.binary
  );

  if (!so_exec(cmd)) {
    return false;
  }
  return true;
}

int main(int argc, char **argv)
{
  options_t options = {0};

  for (int i = 0; i < argc; i++) {
    bool is_last = i == argc - 1;
    if (str_eq(argv[i], "-h") || str_eq(argv[i], "--help")) {
      show_cmd_line_help();
      return 0;
    }
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
    printn("executable created: ./build/linux/%s", g_project.binary);
  }
  if (options.run && !so_exec("./build/linux/%s", g_project.binary)) {
    return 1;
  }
  return 0;
}
