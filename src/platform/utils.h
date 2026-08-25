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

API char *get_platform_flags(u8 platform)
{
  switch (platform) {
    case PLATFORM_LINUX: return "-include platform/linux.h -DPLATFORM=PLATFORM_LINUX";
    case PLATFORM_WEB:   return "-include platform/web.h   -DPLATFORM=PLATFORM_WEB";
    case PLATFORM_WINDOWS: return "-include platform/windows.h -DPLATFORM=PLATFORM_WINDOWS";
    default:             return "";
  }
}

API char *get_backend_flags(u8 backend, u8 platform, arena_t *arena)
{
  switch (backend) {
    case BACKEND_RAYLIB: {
      char *base = raylib_lib_path(arena);

      char *suffix = "build_linux";
      if (platform == PLATFORM_WEB)      suffix = "build_web";
      else if (platform == PLATFORM_WINDOWS) suffix = "build_windows";

      char *inc_path = str_format("%s/src", base);
      char *lib_path = str_format("%s/%s/raylib", base, suffix);
      char *deps = (platform == PLATFORM_WEB) ? "-lraylib" : "-lraylib -lm -lX11";

      return str_format("-DBACKEND=BACKEND_RAYLIB -include backend/raylib.h -I%s -L%s %s", inc_path, lib_path, deps);
    }
    default:
      return "";
  }
}

API char *get_compile_flags(u8 backend, u8 platform, arena_t *arena)
{
  // plataforma primeiro: o backend pode depender dos headers da plataforma (ex.: <emscripten.h>)
  return str_format("%s %s",
    get_platform_flags(platform),
    get_backend_flags(backend, platform, arena));
}

