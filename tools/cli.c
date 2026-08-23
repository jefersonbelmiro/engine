#include "core/arena.h"
#include "core/defs.h"
#include "core/io.h"
#include "core/math.h"
#include "core/mem.h"
#include "core/so.h"
#include "core/string.h"
#include <dlfcn.h>
#include <stddef.h>

#define BINARY_OUTPUT_PATH "./bin"
#define TOOLS_OUTPUT_PATH "./build/cli"

arena_t *g_arena = NULL;
tool_array_t *load_merged_entries();

void show_cmd_line_help()
{
  printn(
    "cli\n"
    " usage   : cli [tool] [options]\n"
    " options :\n"
    "    -h  --help          : show command line usage help\n"
    "    -r  --rebuild       : rebuild cli and entries\n"
  );

  tool_array_t *tools = load_merged_entries();

  if (!tools) {
    return;
  }
  printn("tools:");
  for (int i = 0; i < tools->count; i++) {
    tool_t tool = tools->array[i];
    printn(" - %-20s : %s", tool.name, tool.description);
  }
}

tool_array_t *compile_entries(char *path)
{
  if (!io_file_exists(path)) {
    return NULL;
  }

  if (!io_dir_exists("build/tmp/hot") && !io_mkdir_recursive("build/tmp/hot")) {
    printn("[error] fail to create build/tmp/hot directoreis");
    return NULL;
  }

  char entry_name[strlen(path) + 1];
  strcpy(entry_name, path);
  entry_name[strlen(entry_name) - 2] = 0x0;
  str_slugify(entry_name, '_');

  char *entrie_out = str_format("build/tmp/hot/%s.so", entry_name);

  bool compiled = so_exec(
    "gcc -shared -fPIC -x c %s %s -o %s"
    " -DHOT_RELOAD=1 -DMODULE_BUILD=1 -DDEBUG "
    " -Wno-pragma-once-outside-header",
    "-I./src -I./engine/src",
    path,
    entrie_out
  );
  if (!compiled) {
    return NULL;
  }

  void *lib = dlopen(entrie_out, RTLD_NOW | RTLD_GLOBAL);
  if (!lib) {
    printn("[hot] dlopen %s", dlerror());
    return NULL;
  }

  tool_array_t* (*tools_entries)() = dlsym(lib, "tools_entries");
  if (!tools_entries) {
    printn("[error] fail on get tools_entries() from %s", path);
    dlclose(lib);
    return NULL;
  }

  tool_array_t *src = tools_entries();
  tool_array_t *copy = arena_push(g_arena, tool_array_t, 1);
  copy->count = src->count;
  copy->array = arena_push(g_arena, tool_t, src->count);
  mem_copy(src->array, copy->array, src->count * sizeof(tool_t));

  dlclose(lib);
  return copy;
}

bool compile_merged_entries(char *path)
{
  if (!path) {
    path = (char*)str_format("%s/entries.data", TOOLS_OUTPUT_PATH);
  }
  if (!io_dir_exists(TOOLS_OUTPUT_PATH) && !io_mkdir_recursive(TOOLS_OUTPUT_PATH)) {
    printn("[error] error on create directory TOOLS_OUTPUT_PATH(%s)", TOOLS_OUTPUT_PATH);
    return false;
  }

  size_t before = g_arena->offset;

  tool_array_t *engine_tools = compile_entries("engine/tools/entry.h");
  tool_array_t *app_tools = compile_entries("tools/entry.h");

  u16 total = engine_tools->count + (app_tools ? app_tools->count : 0);
  tool_array_t *tools = arena_push(g_arena, tool_array_t, 1);
  tools->array = arena_push(g_arena, tool_t, total);
  tools->count = engine_tools->count;
  mem_copy(engine_tools->array, tools->array, engine_tools->count * sizeof(tool_t));

  if (app_tools) {
    for (u16 i = 0; i < app_tools->count; i++) {
      tool_t *tool = &app_tools->array[i];
      u16 index = tools->count;
      bool found = false;
      for (u16 ei = 0; ei < engine_tools->count; ei++) {
        if (str_eq(engine_tools->array[ei].name, tool->name)) {
          index = ei;
          found = true;
          break;
        }
      }
      tools->array[index] = *tool;
      if (!found) {
        tools->count++;
      }
    }
  }

  size_t bytes = sizeof(u16) + tools->count * sizeof(tool_t);
  unsigned char *buffer = arena_push(g_arena, unsigned char, bytes);
  mem_copy(&tools->count, buffer, sizeof(u16));
  mem_copy(tools->array, buffer + sizeof(u16), tools->count * sizeof(tool_t));

  bool ok = io_save_file_data(path, buffer, (int)bytes);

  arena_restore(g_arena, before);
  return ok;
}

tool_array_t *load_merged_entries()
{
  static tool_array_t *cached = NULL;
  if (cached) {
    return cached;
  }

  char *path = (char*)str_format("%s/entries.data", TOOLS_OUTPUT_PATH);

  if (!io_file_exists(path) && !compile_merged_entries(path)) {
    return NULL;
  }

  size_t before = g_arena->offset;
  int data_size = 0;
  unsigned char *buffer = io_load_file_data(path, &data_size, g_arena);

  if (!data_size || !buffer) {
    printn("[error] fail to load compiled entries data");
    return NULL;
  }

  u16 count = 0;
  mem_copy(buffer, &count, sizeof(u16));

  if (data_size != (int)(sizeof(u16) + count * sizeof(tool_t))) {
    arena_restore(g_arena, before);
    if (!compile_merged_entries(path)) {
      return NULL;
    }
    return load_merged_entries();
  }

  tool_array_t *tools = arena_push(g_arena, tool_array_t, 1);
  tools->array = (tool_t *)(buffer + sizeof(u16));
  tools->count = count;

  cached = tools;
  return tools;
}

void rebuild(int arg_index, int arg_count, char **main_argv) 
{
  if (!io_dir_exists(BINARY_OUTPUT_PATH) && io_mkdir(BINARY_OUTPUT_PATH)) {
    printn("[error] error on create binary output directory: %s", BINARY_OUTPUT_PATH);
    return;
  }
  if (!so_exec("gcc engine/tools/cli.c -g -I./engine/src -I./src -o %s/cli", BINARY_OUTPUT_PATH)) {
    printn("[error] build self error");
    return;
  }
  log_info("build self done!");

  if (!compile_merged_entries(NULL)) {
    printn("[error] build self error");
    return;
  }

  char *cmd = str_format("%s/cli", BINARY_OUTPUT_PATH);

  int argc = max(0, arg_count - (arg_index + 1)) + 1;
  arg_index += 1;

  char *argv[argc];
  argv[0] = cmd;
  for (int i = 0; i < argc; i++) {
    argv[i + 1] = main_argv[arg_index + i];
  }
  argv[argc] = NULL;

  so_execv(cmd, argv);
}

int tool_execute(tool_t *tool, int arg_index, int arg_count, char **main_argv) 
{
  if (!io_file_exists(tool->source_path)) {
    printn("[error] file not exits %s", tool->source_path);
    return 1;
  }
  if (!io_dir_exists(TOOLS_OUTPUT_PATH) && !io_mkdir_recursive(TOOLS_OUTPUT_PATH)) {
    printn("[error] error on create directory TOOLS_OUTPUT_PATH(%s)", TOOLS_OUTPUT_PATH);
    return 1;
  }
  so_exec("gcc %s -o %s/%s -I./engine/src -I./src -std=gnu11 -D_GNU_SOURCE=1", tool->source_path, TOOLS_OUTPUT_PATH,  tool->name);

  char *cmd = (char*)str_format("%s/%s", TOOLS_OUTPUT_PATH, tool->name);
  int argc = max(0, arg_count - (arg_index + 1)) + 1;
  arg_index += 1;

  char *argv[argc];
  argv[0] = cmd;
  for (int i = 0; i < argc; i++) {
    argv[i + 1] = main_argv[arg_index + i];
  }
  argv[argc] = NULL;

  so_execv(cmd, argv);
  return 0;
}

int main(int argc, char **argv)
{
  g_arena = arena_create(KB(64), "cli");
  if (!g_arena) {
    printn("[error] fail to create arena memory");
    return 1;
  }

  if (argc == 1) {
    show_cmd_line_help();
    return 0;
  }

  if (argc > 1 && (str_eq(argv[1], "-r") || str_eq(argv[1], "--rebuild"))) {
    rebuild(1, argc, argv);
    return 0;
  }
  if (argc > 1 && (str_eq(argv[1], "-h") || str_eq(argv[1], "--help"))) {
    show_cmd_line_help();
    return 0;
  }

  tool_array_t *tools = load_merged_entries();
  if (!tools) {
    printn("[error] fail to load merged entries");
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    for (int tool_index = 0; tool_index < tools->count; tool_index++) {
      tool_t tool = tools->array[tool_index];
      if (str_eq(tool.name, argv[i])) {
        return tool_execute(&tool, i, argc, argv);
      }
    }
  }

  printn("[error] invalid arguments. see avaliable arguments with %s -h", argv[0]);
  return 1;
}
