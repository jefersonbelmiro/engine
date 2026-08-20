#include "core/defs.h"
#include "core/io.h"
#include "core/so.h"
#include "core/string.h"
#include "entry.h"

void show_cmd_line_help()
{
  printn(
    "cli\n"
    " usage   : cli [tool] [options]\n"
    " options :\n"
    "    -h  --help           : show command line usage help\n"
    "    --self               : rebuild itself\n"
  );
  printn("tools:");
  tool_array_t *tools = tools_entries();
  for (int i = 0; i < tools->count; i++) {
    tool_t tool = tools->array[i];
    printn(" - %-20s : %s", tool.name, tool.description);
  }
}

void build_self() 
{
  if (!so_exec("gcc tools/cli.c -o build/cli -I./include")) {
    log_error("build self error");
    return;
  }
  log_info("build self done!");
}

int tool_execute(tool_t *tool, int arg_index, int arg_count, char **main_argv) 
{
  if (!io_file_exists(tool->source_path)) {
    log_error("file not exits %s", tool->source_path);
    return 1;
  }
  so_exec("gcc %s -o build/%s -I./include", tool->source_path, tool->name);
  char *cmd = (char*)str_format("build/%s", tool->name);
  int argc = arg_count - (arg_index + 1) + 1;

  assert(argc > 0);

  char *argv[argc];
  argv[0] = cmd;
  for (int i = 0; i < argc; i++) {
    argv[i] = main_argv[arg_index + i];
  }
  argv[argc] = NULL;

  so_execv(cmd, argv);
  return 0;
}

int main(int argc, char **argv)
{
  if (argc == 1) {
    show_cmd_line_help();
    return 0;
  }

  tool_array_t *tools = tools_entries();

  for (int i = 1; i < argc; i++) {
    if (str_eq(argv[i], "-h") || str_eq(argv[i], "--help")) {
      show_cmd_line_help();
      return 0;
    }
    if (str_eq(argv[i], "--self")) {
      build_self();
      return 0;
    }

    for (int tool_index = 0; tool_index < tools->count; tool_index++) {
      tool_t tool = tools->array[tool_index];
      if (str_eq(tool.name, argv[i])) {
        return tool_execute(&tool, i, argc, argv);
      }
    }
  }

  return 0;
}
