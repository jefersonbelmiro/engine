#define MAX_TEXTFORMAT_BUFFERS 8
#define MAX_TEXT_BUFFER_LENGTH 256

#include "core/arena.h"
#include "core/defs.h"
#include "core/io.h"
#include "core/so.h"
#include "core/string.h"

arena_t *g_arena;

static const char *scenes[] = {
  "main",
  "menu",
};

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

void to_upper_copy(char *dst, size_t dst_cap, const char *src)
{
  size_t i = 0;
  for (; src[i] && i < dst_cap - 1; i++) {
    char c = src[i];
    dst[i] = (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
  }
  dst[i] = '\0';
}

void buf_append(char *buf, size_t cap, size_t *used, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int n = vsnprintf(buf + *used, cap - *used, fmt, args);
  va_end(args);
  if (n > 0) {
    *used += (size_t)n;
  }
  if (*used >= cap) {
    buf[cap - 1] = '\0';
    *used = cap - 1;
  }
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

void write_scene_header(bool force, const char *name)
{
  size_t cap = KB(8);
  char *buf = arena_push(g_arena, char, cap);
  size_t used = 0;

  buf_append(buf, cap, &used, "#pragma once\n");
  buf_append(buf, cap, &used, "\n");
  buf_append(buf, cap, &used, "#include \"core/engine.h\"\n");
  buf_append(buf, cap, &used, "#include \"core/arena.h\"\n");
  buf_append(buf, cap, &used, "\n");
  buf_append(buf, cap, &used, "typedef struct {\n");
  buf_append(buf, cap, &used, "  char pad;\n");
  buf_append(buf, cap, &used, "} %s_scene_t;\n", name);
  buf_append(buf, cap, &used, "\n");
  buf_append(buf, cap, &used, "API %s_scene_t* %s_scene_init()\n", name, name);
  buf_append(buf, cap, &used, "{\n");
  buf_append(buf, cap, &used, "  arena_t *arena = engine_scene_arena();\n");
  buf_append(buf, cap, &used, "  %s_scene_t *scene = arena_push(arena, %s_scene_t, 1);\n", name, name);
  buf_append(buf, cap, &used, "  return scene;\n");
  buf_append(buf, cap, &used, "}\n");
  buf_append(buf, cap, &used, "\n");
  buf_append(buf, cap, &used, "API bool %s_scene_exiting(%s_scene_t *scene)\n", name, name);
  buf_append(buf, cap, &used, "{\n");
  buf_append(buf, cap, &used, "  (void) scene;\n");
  buf_append(buf, cap, &used, "  return true;\n");
  buf_append(buf, cap, &used, "}\n");
  buf_append(buf, cap, &used, "\n");
  buf_append(buf, cap, &used, "API bool %s_scene_entering(%s_scene_t *scene)\n", name, name);
  buf_append(buf, cap, &used, "{\n");
  buf_append(buf, cap, &used, "  (void) scene;\n");
  buf_append(buf, cap, &used, "  return true;\n");
  buf_append(buf, cap, &used, "}\n");
  buf_append(buf, cap, &used, "\n");
  buf_append(buf, cap, &used, "API void %s_scene_sync(%s_scene_t *scene, sync_signal_type_t signal)\n", name, name);
  buf_append(buf, cap, &used, "{\n");
  buf_append(buf, cap, &used, "  (void) scene; (void) signal;\n");
  buf_append(buf, cap, &used, "}\n");
  buf_append(buf, cap, &used, "\n");
  buf_append(buf, cap, &used, "API void %s_scene_free(%s_scene_t *scene)\n", name, name);
  buf_append(buf, cap, &used, "{\n");
  buf_append(buf, cap, &used, "  (void) scene;\n");
  buf_append(buf, cap, &used, "}\n");
  buf_append(buf, cap, &used, "\n");
  buf_append(buf, cap, &used, "API void %s_scene_process(%s_scene_t *scene, float delta)\n", name, name);
  buf_append(buf, cap, &used, "{\n");
  buf_append(buf, cap, &used, "  (void) scene; (void) delta;\n");
  buf_append(buf, cap, &used, "}\n");
  buf_append(buf, cap, &used, "\n");
  buf_append(buf, cap, &used, "API void %s_scene_draw(%s_scene_t *scene)\n", name, name);
  buf_append(buf, cap, &used, "{\n");
  buf_append(buf, cap, &used, "  (void) scene;\n");
  buf_append(buf, cap, &used, "}\n");

  char *path = str_format("src/scenes/%s.h", name);
  write_file(path, buf, force);
}

void write_scene_entry(bool force)
{
  u32 scene_count = countof(scenes);
  size_t cap = KB(16);
  char *buf = arena_push(g_arena, char, cap);
  size_t used = 0;

  buf_append(buf, cap, &used, "#pragma once\n\n");
  buf_append(buf, cap, &used, "#include \"core/engine.h\"\n");
  for (u32 i = 0; i < scene_count; i++) {
    buf_append(buf, cap, &used, "#include \"scenes/%s.h\"\n", scenes[i]);
  }
  buf_append(buf, cap, &used, "\n");

  buf_append(buf, cap, &used, "typedef enum scene_type_t {\n");
  buf_append(buf, cap, &used, "  SCENE_NONE,\n");
  for (u32 i = 0; i < scene_count; i++) {
    char upper[32];
    to_upper_copy(upper, sizeof(upper), scenes[i]);
    buf_append(buf, cap, &used, "  SCENE_%s,\n", upper);
  }
  buf_append(buf, cap, &used, "  SCENE_COUNT,\n");
  buf_append(buf, cap, &used, "} scene_type_t;\n\n");

  buf_append(buf, cap, &used, "API bool engine_scene_entering() \n");
  buf_append(buf, cap, &used, "{\n");
  buf_append(buf, cap, &used, "  engine_t *app = engine_ptr();\n");
  buf_append(buf, cap, &used, "  switch (app->scene) {\n");
  for (u32 i = 0; i < scene_count; i++) {
    char upper[32];
    to_upper_copy(upper, sizeof(upper), scenes[i]);
    buf_append(buf, cap, &used, "    case SCENE_%s:\n", upper);
    buf_append(buf, cap, &used, "      return %s_scene_entering(app->scene_state);\n", scenes[i]);
  }
  buf_append(buf, cap, &used, "    default: return true;\n");
  buf_append(buf, cap, &used, "  }\n");
  buf_append(buf, cap, &used, "}\n\n");

  buf_append(buf, cap, &used, "API bool engine_scene_exiting() \n");
  buf_append(buf, cap, &used, "{\n");
  buf_append(buf, cap, &used, "  engine_t *app = engine_ptr();\n");
  buf_append(buf, cap, &used, "  switch (app->scene) {\n");
  for (u32 i = 0; i < scene_count; i++) {
    char upper[32];
    to_upper_copy(upper, sizeof(upper), scenes[i]);
    buf_append(buf, cap, &used, "    case SCENE_%s:\n", upper);
    buf_append(buf, cap, &used, "      return %s_scene_exiting(app->scene_state);\n", scenes[i]);
  }
  buf_append(buf, cap, &used, "    default: return true;\n");
  buf_append(buf, cap, &used, "  }\n");
  buf_append(buf, cap, &used, "}\n\n");

  buf_append(buf, cap, &used, "API void engine_scene_init() \n");
  buf_append(buf, cap, &used, "{\n");
  buf_append(buf, cap, &used, "  engine_t *app = engine_ptr();\n");
  buf_append(buf, cap, &used, "  switch (app->scene) {\n");
  for (u32 i = 0; i < scene_count; i++) {
    char upper[32];
    to_upper_copy(upper, sizeof(upper), scenes[i]);
    buf_append(buf, cap, &used, "    case SCENE_%s:\n", upper);
    buf_append(buf, cap, &used, "      app->scene_state = %s_scene_init();\n", scenes[i]);
    buf_append(buf, cap, &used, "    break;\n");
  }
  buf_append(buf, cap, &used, "    default: break;\n");
  buf_append(buf, cap, &used, "  }\n");
  buf_append(buf, cap, &used, "}\n\n");

  buf_append(buf, cap, &used, "API void engine_scene_process() \n");
  buf_append(buf, cap, &used, "{\n");
  buf_append(buf, cap, &used, "  engine_t *app = engine_ptr();\n");
  buf_append(buf, cap, &used, "  float delta = engine_delta_time();\n");
  buf_append(buf, cap, &used, "  switch (app->scene) {\n");
  for (u32 i = 0; i < scene_count; i++) {
    char upper[32];
    to_upper_copy(upper, sizeof(upper), scenes[i]);
    buf_append(buf, cap, &used, "    case SCENE_%s:\n", upper);
    buf_append(buf, cap, &used, "      %s_scene_process(app->scene_state, delta);\n", scenes[i]);
    buf_append(buf, cap, &used, "    break;\n");
  }
  buf_append(buf, cap, &used, "    default: break;\n");
  buf_append(buf, cap, &used, "  }\n");
  buf_append(buf, cap, &used, "}\n\n");

  buf_append(buf, cap, &used, "API void engine_scene_draw() \n");
  buf_append(buf, cap, &used, "{\n");
  buf_append(buf, cap, &used, "  engine_t *app = engine_ptr();\n");
  buf_append(buf, cap, &used, "  switch (app->scene) {\n");
  for (u32 i = 0; i < scene_count; i++) {
    char upper[32];
    to_upper_copy(upper, sizeof(upper), scenes[i]);
    buf_append(buf, cap, &used, "    case SCENE_%s:\n", upper);
    buf_append(buf, cap, &used, "      %s_scene_draw(app->scene_state);\n", scenes[i]);
    buf_append(buf, cap, &used, "    break;\n");
  }
  buf_append(buf, cap, &used, "    default: break;\n");
  buf_append(buf, cap, &used, "  }\n");
  buf_append(buf, cap, &used, "}\n\n");

  buf_append(buf, cap, &used, "API void engine_scene_free() \n");
  buf_append(buf, cap, &used, "{\n");
  buf_append(buf, cap, &used, "  engine_t *app = engine_ptr();\n");
  buf_append(buf, cap, &used, "  switch (app->scene) {\n");
  for (u32 i = 0; i < scene_count; i++) {
    char upper[32];
    to_upper_copy(upper, sizeof(upper), scenes[i]);
    buf_append(buf, cap, &used, "    case SCENE_%s:\n", upper);
    buf_append(buf, cap, &used, "      %s_scene_free(app->scene_state);\n", scenes[i]);
    buf_append(buf, cap, &used, "    break;\n");
  }
  buf_append(buf, cap, &used, "    default: break;\n");
  buf_append(buf, cap, &used, "  }\n");
  buf_append(buf, cap, &used, "}\n\n");

  buf_append(buf, cap, &used, "API void engine_scene_sync(u8 scene, sync_signal_type_t signal)\n");
  buf_append(buf, cap, &used, "{\n");
  buf_append(buf, cap, &used, "  switch (scene) {\n");
  for (u32 i = 0; i < scene_count; i++) {
    char upper[32];
    to_upper_copy(upper, sizeof(upper), scenes[i]);
    buf_append(buf, cap, &used, "    case SCENE_%s:\n", upper);
    buf_append(buf, cap, &used, "      %s_scene_sync(engine_ptr()->scene_state, signal);\n", scenes[i]);
    buf_append(buf, cap, &used, "    break;\n");
  }
  buf_append(buf, cap, &used, "    default: break;\n");
  buf_append(buf, cap, &used, "  }\n");
  buf_append(buf, cap, &used, "}\n");

  write_file("src/scenes/entry.h", buf, force);
}

void write_project_h(bool force, const char *name)
{
  size_t cap = KB(8);
  char *buf = arena_push(g_arena, char, cap);
  size_t used = 0;

  buf_append(buf, cap, &used, "#pragma once\n");
  buf_append(buf, cap, &used, "\n");
  buf_append(buf, cap, &used, "#include \"core/defs.h\"\n");
  buf_append(buf, cap, &used, "\n");
  buf_append(buf, cap, &used, "typedef struct {\n");
  buf_append(buf, cap, &used, "  char *name;\n");
  buf_append(buf, cap, &used, "  char *version;\n");
  buf_append(buf, cap, &used, "  char *binary;\n");
  buf_append(buf, cap, &used, "  char *lib_raylib_path;\n");
  buf_append(buf, cap, &used, "  char *lib_emsdk_path;\n");
  buf_append(buf, cap, &used, "  bool support_fileformat_jpg;\n");
  buf_append(buf, cap, &used, "} project_t;\n");
  buf_append(buf, cap, &used, "\n");
  buf_append(buf, cap, &used, "static project_t g_project = {\n");
  buf_append(buf, cap, &used, "  .name = \"%s\",\n", name);
  buf_append(buf, cap, &used, "  .version = \"0.0.1\",\n");
  buf_append(buf, cap, &used, "  .binary = \"%s\",\n", name);
  buf_append(buf, cap, &used, "  .lib_raylib_path = \"~/dev/libs/raylib\",\n");
  buf_append(buf, cap, &used, "  .lib_emsdk_path = \"~/dev/libs/emsdk\",\n");
  buf_append(buf, cap, &used, "  .support_fileformat_jpg = true,\n");
  buf_append(buf, cap, &used, "};\n");
  buf_append(buf, cap, &used, "\n");
  buf_append(buf, cap, &used, "API project_t *project_ptr()\n");
  buf_append(buf, cap, &used, "{\n");
  buf_append(buf, cap, &used, "  return &g_project;\n");
  buf_append(buf, cap, &used, "}\n");

  write_file("src/project.h", buf, force);
}

void write_main_c(bool force)
{
  write_file("src/main.c",
    "#include \"core/engine.c\"\n"
    "#include \"scenes/entry.h\"\n"
    "\n"
    "int main()\n"
    "{\n"
    "  printn(\"main()\");\n"
    "\n"
    "  engine_init();\n"
    "  engine_set_scene(SCENE_MENU);\n"
    "  engine_start();\n"
    "\n"
    "  return 0;\n"
    "}\n",
    force);
}

void write_compile_flags(bool force)
{
  char raylib_abs[SO_PATH_MAX];
  char emsdk_abs[SO_PATH_MAX];
  so_resolve_home("~/dev/libs/raylib", raylib_abs);
  so_resolve_home("~/dev/libs/emsdk", emsdk_abs);

  size_t cap = KB(8);
  char *buf = arena_push(g_arena, char, cap);
  size_t used = 0;

  buf_append(buf, cap, &used, "-std=gnu11\n");
  buf_append(buf, cap, &used, "-I./engine/src\n");
  buf_append(buf, cap, &used, "-I./src\n");
  buf_append(buf, cap, &used, "-I%s/src\n", raylib_abs);
  buf_append(buf, cap, &used, "-I%s/upstream/emscripten/cache/sysroot/include\n", emsdk_abs);
  buf_append(buf, cap, &used, "-DDEBUG\n");
  buf_append(buf, cap, &used, "-DLOG_LEVEL=5\n");
  buf_append(buf, cap, &used, "-DDEBUG_MEMORY_USAGE\n");
  buf_append(buf, cap, &used, "-DARENA_FALLBACK_MALLOC\n");
  buf_append(buf, cap, &used, "-DHOT_RELOAD\n");
  buf_append(buf, cap, &used, "-DMODULE_BUILD\n");
  buf_append(buf, cap, &used, "-DPLATFORM=PLATFORM_LINUX\n");
  buf_append(buf, cap, &used, "-DBACKEND=BACKEND_RAYLIB\n");
  buf_append(buf, cap, &used, "-Wall\n");
  buf_append(buf, cap, &used, "-Wextra");

  write_file("compile_flags.txt", buf, force);
}

void write_tools_entry(bool force)
{
  write_file("tools/entry.h",
    "#pragma once\n"
    "\n"
    "#include \"core/defs.h\"\n"
    "\n"
    "static tool_t tools[] = {\n"
    "  // { \n"
    "  //   .name = \"build_linux\",\n"
    "  //   .description = \"build platform target linux (user override)\",\n"
    "  //   .source_path = \"src/tools/build_linux.c\",\n"
    "  // },\n"
    "};\n"
    "\n"
    "API tool_array_t *tools_entries()\n"
    "{\n"
    "  static tool_array_t array = {\n"
    "    .array = tools,\n"
    "    .count = countof(tools)\n"
    "  };\n"
    "  return &array;\n"
    "}\n",
    force);
}

void write_gitignore(bool force)
{
  write_file(".gitignore", "/build\n", force);
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

  write_scene_entry(force);
  for (u32 i = 0; i < countof(scenes); i++) {
    write_scene_header(force, scenes[i]);
  }
  write_project_h(force, name);
  write_main_c(force);
  write_compile_flags(force);
  write_tools_entry(force);
  write_gitignore(force);

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
