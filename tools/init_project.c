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

// Monta os blocos "case SCENE_X:" de uma funcao de dispatch, iterando a lista de cenas.
// body_fmt recebe o nome da cena (ex.: "return %s_scene_entering(app->scene_state);").
void build_cases(char *dst, size_t cap, const char *body_fmt, bool with_break)
{
  size_t used = 0;
  for (u32 i = 0; i < countof(scenes); i++) {
    char upper[32];
    to_upper_copy(upper, sizeof(upper), scenes[i]);
    used += (size_t)snprintf(dst + used, cap - used, "    case SCENE_%s:\n", upper);
    used += (size_t)snprintf(dst + used, cap - used, "      ");
    used += (size_t)snprintf(dst + used, cap - used, body_fmt, scenes[i]);
    used += (size_t)snprintf(dst + used, cap - used, "\n");
    if (with_break) {
      used += (size_t)snprintf(dst + used, cap - used, "    break;\n");
    }
  }
}

void write_scene_header(bool force, const char *name)
{
  char buf[KB(4)];
  snprintf(buf, sizeof(buf),
    "#pragma once\n"
    "\n"
    "#include \"core/engine.h\"\n"
    "#include \"core/arena.h\"\n"
    "\n"
    "typedef struct {\n"
    "  char pad;\n"
    "} %s_scene_t;\n"
    "\n"
    "API %s_scene_t* %s_scene_init()\n"
    "{\n"
    "  arena_t *arena = engine_scene_arena();\n"
    "  %s_scene_t *scene = arena_push(arena, %s_scene_t, 1);\n"
    "  return scene;\n"
    "}\n"
    "\n"
    "API bool %s_scene_exiting(%s_scene_t *scene)\n"
    "{\n"
    "  (void) scene;\n"
    "  return true;\n"
    "}\n"
    "\n"
    "API bool %s_scene_entering(%s_scene_t *scene)\n"
    "{\n"
    "  (void) scene;\n"
    "  return true;\n"
    "}\n"
    "\n"
    "API void %s_scene_sync(%s_scene_t *scene, sync_signal_type_t signal)\n"
    "{\n"
    "  (void) scene; (void) signal;\n"
    "}\n"
    "\n"
    "API void %s_scene_free(%s_scene_t *scene)\n"
    "{\n"
    "  (void) scene;\n"
    "}\n"
    "\n"
    "API void %s_scene_process(%s_scene_t *scene, float delta)\n"
    "{\n"
    "  (void) scene; (void) delta;\n"
    "}\n"
    "\n"
    "API void %s_scene_draw(%s_scene_t *scene)\n"
    "{\n"
    "  (void) scene;\n"
    "}\n",
    name, name, name, name, name,
    name, name,
    name, name,
    name, name,
    name, name,
    name, name,
    name, name
  );

  char *path = str_format("src/scenes/%s.h", name);
  write_file(path, buf, force);
}

void write_scene_entry(bool force)
{
  char includes[256]  = {0};
  char enum_members[256] = {0};
  char cases_entering[1024] = {0};
  char cases_exiting[1024]  = {0};
  char cases_init[1024]     = {0};
  char cases_process[1024]  = {0};
  char cases_draw[1024]     = {0};
  char cases_free[1024]     = {0};
  char cases_sync[1024]     = {0};

  size_t used = 0;
  for (u32 i = 0; i < countof(scenes); i++) {
    char upper[32];
    to_upper_copy(upper, sizeof(upper), scenes[i]);
    used += (size_t)snprintf(includes + used, sizeof(includes) - used, "#include \"scenes/%s.h\"\n", scenes[i]);
  }
  used = 0;
  for (u32 i = 0; i < countof(scenes); i++) {
    char upper[32];
    to_upper_copy(upper, sizeof(upper), scenes[i]);
    used += (size_t)snprintf(enum_members + used, sizeof(enum_members) - used, "  SCENE_%s,\n", upper);
  }

  build_cases(cases_entering, sizeof(cases_entering), "return %s_scene_entering(app->scene_state);", false);
  build_cases(cases_exiting,  sizeof(cases_exiting),  "return %s_scene_exiting(app->scene_state);",  false);
  build_cases(cases_init,     sizeof(cases_init),     "app->scene_state = %s_scene_init();",          true);
  build_cases(cases_process,  sizeof(cases_process),  "%s_scene_process(app->scene_state, delta);",   true);
  build_cases(cases_draw,     sizeof(cases_draw),     "%s_scene_draw(app->scene_state);",             true);
  build_cases(cases_free,     sizeof(cases_free),     "%s_scene_free(app->scene_state);",             true);
  build_cases(cases_sync,     sizeof(cases_sync),     "%s_scene_sync(engine_ptr()->scene_state, signal);", true);

  char buf[KB(8)];
  snprintf(buf, sizeof(buf),
    "#pragma once\n"
    "\n"
    "#include \"core/engine.h\"\n"
    "%s"
    "\n"
    "typedef enum scene_type_t {\n"
    "  SCENE_NONE,\n"
    "%s"
    "  SCENE_COUNT,\n"
    "} scene_type_t;\n"
    "\n"
    "API bool engine_scene_entering() \n"
    "{\n"
    "  engine_t *app = engine_ptr();\n"
    "  switch (app->scene) {\n"
    "%s"
    "    default: return true;\n"
    "  }\n"
    "}\n"
    "\n"
    "API bool engine_scene_exiting() \n"
    "{\n"
    "  engine_t *app = engine_ptr();\n"
    "  switch (app->scene) {\n"
    "%s"
    "    default: return true;\n"
    "  }\n"
    "}\n"
    "\n"
    "API void engine_scene_init() \n"
    "{\n"
    "  engine_t *app = engine_ptr();\n"
    "  switch (app->scene) {\n"
    "%s"
    "    default: break;\n"
    "  }\n"
    "}\n"
    "\n"
    "API void engine_scene_process() \n"
    "{\n"
    "  engine_t *app = engine_ptr();\n"
    "  float delta = engine_delta_time();\n"
    "  switch (app->scene) {\n"
    "%s"
    "    default: break;\n"
    "  }\n"
    "}\n"
    "\n"
    "API void engine_scene_draw() \n"
    "{\n"
    "  engine_t *app = engine_ptr();\n"
    "  switch (app->scene) {\n"
    "%s"
    "    default: break;\n"
    "  }\n"
    "}\n"
    "\n"
    "API void engine_scene_free() \n"
    "{\n"
    "  engine_t *app = engine_ptr();\n"
    "  switch (app->scene) {\n"
    "%s"
    "    default: break;\n"
    "  }\n"
    "}\n"
    "\n"
    "API void engine_scene_sync(u8 scene, sync_signal_type_t signal)\n"
    "{\n"
    "  switch (scene) {\n"
    "%s"
    "    default: break;\n"
    "  }\n"
    "}\n",
    includes, enum_members,
    cases_entering, cases_exiting, cases_init,
    cases_process, cases_draw, cases_free, cases_sync
  );

  write_file("src/scenes/entry.h", buf, force);
}

void write_project_h(bool force, const char *name)
{
  char buf[KB(4)];
  snprintf(buf, sizeof(buf),
    "#pragma once\n"
    "\n"
    "#include \"core/defs.h\"\n"
    "\n"
    "typedef struct {\n"
    "  char *name;\n"
    "  char *version;\n"
    "  char *binary;\n"
    "  char *lib_raylib_path;\n"
    "  char *lib_emsdk_path;\n"
    "  bool support_fileformat_jpg;\n"
    "} project_t;\n"
    "\n"
    "static project_t g_project = {\n"
    "  .name = \"%s\",\n"
    "  .version = \"0.0.1\",\n"
    "  .binary = \"%s\",\n"
    "  .lib_raylib_path = \"~/dev/libs/raylib\",\n"
    "  .lib_emsdk_path = \"~/dev/libs/emsdk\",\n"
    "  .support_fileformat_jpg = true,\n"
    "};\n"
    "\n"
    "API project_t *project_ptr()\n"
    "{\n"
    "  return &g_project;\n"
    "}\n",
    name, name
  );
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

  char buf[KB(4)];
  snprintf(buf, sizeof(buf),
    "-std=gnu11\n"
    "-I./engine/src\n"
    "-I./src\n"
    "-I%s/src\n"
    "-I%s/upstream/emscripten/cache/sysroot/include\n"
    "-DDEBUG\n"
    "-DLOG_LEVEL=5\n"
    "-DDEBUG_MEMORY_USAGE\n"
    "-DARENA_FALLBACK_MALLOC\n"
    "-DHOT_RELOAD\n"
    "-DMODULE_BUILD\n"
    "-DPLATFORM=PLATFORM_LINUX\n"
    "-DBACKEND=BACKEND_RAYLIB\n"
    "-Wall\n"
    "-Wextra",
    raylib_abs, emsdk_abs
  );
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
