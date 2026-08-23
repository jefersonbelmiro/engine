#include "core/arena.h"
#include "core/defs.h"
#include "core/io.h"
#include "core/math.h"
#include "core/mem.h"
#include "core/so.h"
#include "core/string.h"
#include "entry.h"
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

  char entry_name[strlen(path)];
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
    return NULL;
  }
  
  return tools_entries();
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

  tool_array_t *engine_tools = compile_entries("engine/tools/entry.h");
  tool_array_t *app_tools = compile_entries("tools/entry.h");

  printn("engine_tools: %d", engine_tools->count);
  printn("app_tools: %d", !!app_tools);

  size_t total_count = engine_tools->count; 
  if (app_tools) {
    total_count += app_tools->count;
  }
  printn("total_count: %d -> bytes: %d", total_count, total_count * sizeof(tool_t));

  arena_t *arena = arena_create_sub(g_arena, KB(4), "compile_entries");

  tool_array_t *tools = arena_push(arena, tool_array_t, 1);
  *tools = (tool_array_t) {
    .array = arena_push(arena, tool_t, total_count),
    .count = 0,
  };

  for (u16 i = 0; i < engine_tools->count; i++) {
    tools->array[tools->count++] = engine_tools->array[i];
  }

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

  size_t bytes = sizeof(tools->count);
  for (u16 i = 0; i < tools->count; i++) {
    tool_t *tool = &tools->array[i];
    bytes += strlen(tool->name) + sizeof(u16) + 1;
    bytes += strlen(tool->description) + sizeof(u16) + 1;
    bytes += strlen(tool->source_path) + sizeof(u16) + 1;
  }

  unsigned char *buffer = arena_push(arena, unsigned char, bytes);
  mem_copy(&tools->count, buffer, sizeof(tools->count));
  size_t offset = sizeof(tools->count);
  for (u16 i = 0; i < tools->count; i++) {
    tool_t *tool = &tools->array[i];

    u16 name_len = strlen(tool->name) + 1;
    u16 description_len = strlen(tool->description) + 1;
    u16 source_len = strlen(tool->source_path) + 1;

    mem_copy(&name_len, buffer + offset, sizeof(u16));
    offset += sizeof(u16);
    mem_copy(tool->name, buffer + offset, name_len);
    offset += name_len;

    mem_copy(&description_len, buffer + offset, sizeof(u16));
    offset += sizeof(u16);
    mem_copy(tool->description, buffer + offset, description_len);
    offset += description_len;

    mem_copy(&source_len, buffer + offset, sizeof(u16));
    offset += sizeof(u16);
    mem_copy(tool->source_path, buffer + offset, source_len);
    offset += source_len;
  }

  printn("bytes: %d", bytes);
  printn(" - count: %d", tools->count);
  printn(" - count sizeof: %d", sizeof(tools->count));

  return io_save_file_data(path, buffer, bytes);
}

tool_array_t *load_merged_entries()
{
  char *path = (char*)str_format("%s/entries.data", TOOLS_OUTPUT_PATH);

  if (!io_file_exists(path) && !compile_merged_entries(path)) {
    return NULL;
  }

  int data_size = 0;
  unsigned char *buffer = io_load_file_data(path, &data_size, g_arena);

  if (!data_size || !buffer) {
    printn("[error] fail to load compiled entries data");
    return NULL;
  }

  tool_array_t *tools = arena_push(g_arena, tool_array_t, 1);

  mem_copy(buffer, &tools->count, sizeof(tools->count));

  *tools = (tool_array_t){
    .array = arena_push(g_arena, tool_t, tools->count),
    .count = tools->count,
  };

  printn("load_merged_entries: %d", data_size);
  printn("tools count: %d", tools->count);

  u16 offset = sizeof(tools->count);
  for (u16 i = 0; i < tools->count; i++) {
    tool_t *tool = &tools->array[i];

    u16 len = 0;
    mem_copy(buffer + offset, &len, sizeof(u16));
    offset += sizeof(u16);
    tool->name = arena_push(g_arena, char, sizeof(char) * len);
    mem_copy(buffer + offset, tool->name, sizeof(char) * len);
    offset += sizeof(char) * len;

    mem_copy(buffer + offset, &len, sizeof(u16));
    offset += sizeof(u16);
    tool->description = arena_push(g_arena, char, sizeof(char) * len);
    mem_copy(buffer + offset, tool->description, sizeof(char) * len);
    offset += sizeof(char) * len;

    mem_copy(buffer + offset, &len, sizeof(u16));
    offset += sizeof(u16);
    tool->source_path = arena_push(g_arena, char, sizeof(char) * len);
    mem_copy(buffer + offset, tool->source_path, sizeof(char) * len);
    offset += sizeof(char) * len;

    printn(" - %-20s : %s", tool->name, tool->description);
  }
  exit(1);
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
  so_exec("gcc %s -o %s/%s -I./src -std=gnu11 -D_GNU_SOURCE=1", tool->source_path, TOOLS_OUTPUT_PATH,  tool->name);
  char *cmd = (char*)str_format("%s/%s", TOOLS_OUTPUT_PATH, tool->name);
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
  g_arena = arena_create(KB(64), "cli");
  if (!g_arena) {
    printn("[error] fail to create arena memory");
    return 1;
  }

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
    if (str_eq(argv[i], "-r") || str_eq(argv[i], "--rebuild")) {
      rebuild(i, argc, argv);
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
