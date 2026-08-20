#include "core/defs.h"
#include "core/io.h"
#include "core/so.h"

void show_cmd_line_help()
{
  printn(
    "build_linux\n"
    " usage   : build_linux [options]\n"
    " options :\n"
    "    -h  --help           : show command line usage help\n"
  );
}

int main(int argc, char **argv)
{
  if (argc == 1) {
    show_cmd_line_help();
    return 0;
  }

  printn("argc: %d", argc);
  for (int i = 0; i < argc; i++) {
    printn(" - argv: %s", argv[i]);
  }

  if (!io_dir_exists("build/linux") && !io_mkdir_recursive("build/linux")) {
    return 1;
  }

  if (!so_exec("gcc src/main.c -o build/linux/main -I./include")) {
    return 1;
  }
  printn("build done");
  printn("executable created build/linux/main");
  return 0;
}
