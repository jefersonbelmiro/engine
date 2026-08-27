#define MAX_TEXTFORMAT_BUFFERS 8
#define MAX_TEXT_BUFFER_LENGTH 256

#include "core/arena.h"
#include "core/defs.h"
#include "core/io.h"
#include "core/so.h"
#include "core/string.h"
#include "core/tpl_parser.h"

arena_t *g_arena;

#define TEMPLATES_PATH "engine/templates/init_project.md"

void show_cmd_line_help()
{
  printn(
    "init_project\n"
    " usage   : init_project [options]\n"
    " options :\n"
    "    -h  --help          : show command line usage help\n"
    "    -f  --force         : overwrite files if they already exist\n"
    "    -n  --name=[NAME]   : project name (default: main)\n"
  );
}

bool write_file(const char *path, const char *data, bool force)
{
  if (io_file_exists(path) && !force) {
    printn("[error] %s already exists (use -f to overwrite)", path);
    return false;
  }
  if (!io_save_file_data(path, data, (int)strlen(data))) {
    printn("[error] fail to write %s", path);
    return false;
  }
  return true;
}

bool generate(bool force, char *name)
{
  if (str_is_empty(name)) {
    name = "main";
  }

  if (!io_dir_exists("src") && !io_mkdir_recursive("src")) {
    return false;
  }
  if (!io_dir_exists("src/scenes") && !io_mkdir_recursive("src/scenes")) {
    return false;
  }
  if (!io_dir_exists("tools") && !io_mkdir_recursive("tools")) {
    return false;
  }

  int data_size = 0;
  unsigned char *data = io_load_file_data(TEMPLATES_PATH, &data_size, g_arena);
  if (!data) {
    printn("[error] fail to load %s", TEMPLATES_PATH);
    return false;
  }

  char raylib_abs[SO_PATH_MAX];
  char emsdk_abs[SO_PATH_MAX];
  so_resolve_home("~/dev/libs/raylib", raylib_abs);
  so_resolve_home("~/dev/libs/emsdk", emsdk_abs);

  tpl_var_t vars[] = {
    { "NAME",        name },
    { "BINARY",      name },
    { "VERSION",     "0.0.1" },
    { "RAYLIB_PATH", raylib_abs },
    { "EMSDK_PATH",  emsdk_abs },
  };

  tpl_file_t *files = NULL;
  int file_count = tpl_parse(g_arena, (char *)data, &files);

  for (int i = 0; i < file_count; i++) {
    char *content = tpl_render(g_arena, files[i].content, vars, countof(vars));
    if (!write_file(files[i].path, content, force)) {
      return false;
    }
  }

  printn("project base created (name: %s)", name);
  return true;
}

int main(int argc, char **argv)
{
  g_arena = arena_create(KB(64), "init_project");

  bool force = false;
  char *name = "";

  for (int i = 0; i < argc; i++) {
    if (str_eq(argv[i], "-h") || str_eq(argv[i], "--help")) {
      show_cmd_line_help();
      return 0;
    }
    if (str_eq(argv[i], "-f") || str_eq(argv[i], "--force")) {
      force = true;
    }
    else if (str_eq(argv[i], "-n") || str_eq(argv[i], "--name")) {
      if (i + 1 < argc) name = argv[++i];
    }
    else if (str_start_with(argv[i], "-n=")) {
      name = argv[i] + 3;
    }
    else if (str_start_with(argv[i], "--name=")) {
      name = argv[i] + 8;
    }
  }

  if (!generate(force, name)) {
    return 1;
  }
  return 0;
}
