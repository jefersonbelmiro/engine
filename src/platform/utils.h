#include "core/arena.h"
#include "core/defs.h"
#include "core/so.h"
#include "project.h"

API char *raylib_lib_path(arena_t *arena)
{
  project_t *project = project_ptr();
  char *path = arena_push(arena, char, SO_PATH_MAX);
  so_resolve_home(project->lib_raylib_path, path);
  return path;
}

API char *emsdk_lib_path(arena_t *arena)
{
  project_t *project = project_ptr();
  char *path = arena_push(arena, char, SO_PATH_MAX);
  so_resolve_home(project->lib_emsdk_path, path);
  return path;
}

API char *emsdk_env_path(arena_t *arena)
{
  project_t *project = project_ptr();
  char *emsdk_base = arena_push(arena, char, SO_PATH_MAX);
  so_resolve_home(project->lib_emsdk_path, emsdk_base);
  char *format = "%s/emsdk_env.sh";
  char *path = arena_push(arena, char, SO_PATH_MAX);
  snprintf(path, strlen(emsdk_base) + strlen(format), format, emsdk_base);
  return path;
}

API char *emsdk_inc_path(arena_t *arena)
{
  project_t *project = project_ptr();
  char *emsdk_base = arena_push(arena, char, SO_PATH_MAX);
  so_resolve_home(project->lib_emsdk_path, emsdk_base);
  char *format = "%s/upstream/emscripten/cache/sysroot/include";
  char *path = arena_push(arena, char, SO_PATH_MAX);
  snprintf(path, strlen(emsdk_base) + strlen(format), format, emsdk_base);
  return path;
}

API char *raylib_inc_path(arena_t *arena)
{
  project_t *project = project_ptr();
  char *base_path = arena_push(arena, char, SO_PATH_MAX);
  so_resolve_home(project->lib_raylib_path, base_path);
  char *format = "%s/src";
  char *path = arena_push(arena, char, SO_PATH_MAX);
  snprintf(path, strlen(base_path) + strlen(format), format, base_path);
  return path;
}

