#pragma once

#include "platform/api.h"

#include <emscripten/emscripten.h>
#include <emscripten/em_js.h>
#include <stdlib.h>

static bool g_need_filesync = false;

static volatile bool g_web_is_mobile = false;

// set to 1 by the js syncfs callback when the indexeddb populate is done
static volatile int g_idbfs_ready = 0;

// web_set_idbfs_ptr() passes the address of g_idbfs_ready so the js
// syncfs callback can set it to 1 when the IndexedDB populate finishes
EM_JS(void, web_set_idbfs_ptr, (volatile int *ptr), {
  Module._g_idbfs_ready_ptr = ptr;
});

EM_JS(void, web_idbfs_mount, (), {
  FS.mkdir('/data');
  FS.mount(IDBFS, { autoPersist: true }, '/data');
  FS.syncfs(true, function(err) {
    if (err) console.error('IDBFS syncfs error:', err);
    if (Module._g_idbfs_ready_ptr) {
      setValue(Module._g_idbfs_ready_ptr, 1, 'i32');
    }
  });
});

EM_JS(int, web_is_mobile_js, (), {
  return (navigator.maxTouchPoints > 0 || /Mobi|Android|iPhone|iPad|iPod/i.test(navigator.userAgent)) ? 1 : 0;
});

API void web_syncfs()
{
  EM_ASM({
    FS.syncfs(/*populate=*/false, function(err) {
      if (err) console.log("syncfs error: ", err);
    });
  });
  g_need_filesync = false;
}

API void web_remove_loading_overlay()
{
  EM_ASM({
    const loadElement = document.getElementById("loading-overlay");
    loadElement.style.display = 'none';
    loadElement.parentElement.removeChild(loadElement);
  });
}

API void fs_init()
{
  printn("[web] fs_init()");
  web_set_idbfs_ptr(&g_idbfs_ready);
  web_idbfs_mount();
  g_web_is_mobile = web_is_mobile_js();
}

API bool fs_ready()
{
  return g_idbfs_ready;
}

API void fs_mark_ready()
{
  web_remove_loading_overlay();
}

API bool device_is_mobile(void)
{
  return g_web_is_mobile;
}

API bool device_has_touch(void)
{
  return g_web_is_mobile;
}

API char *fs_binary_path()
{
  return "/";
}

API bool fs_save_file(const char *file_name, const void *data, int data_size)
{
  char path[2048];
  snprintf(path, sizeof(path), "%s%s", fs_binary_path(), file_name);

  FILE *file = fopen(path, "wb");
  if (!file) {
    return false;
  }

  size_t written = fwrite(data, 1, (size_t)data_size, file);
  fclose(file);

  g_need_filesync = true;
  return (int)written == data_size;
}

API unsigned char *fs_load_file(const char *file_name, int *data_size)
{
  if (g_need_filesync) {
    web_syncfs();
  }

  char path[2048];
  snprintf(path, sizeof(path), "%s%s", fs_binary_path(), file_name);

  *data_size = 0;

  FILE *file = fopen(path, "rb");
  if (!file) {
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (size <= 0) {
    fclose(file);
    return NULL;
  }

  unsigned char *buffer = (unsigned char *)malloc((size_t)size);
  if (!buffer) {
    fclose(file);
    return NULL;
  }

  size_t read_count = fread(buffer, 1, (size_t)size, file);
  fclose(file);

  *data_size = (int)read_count;
  return buffer;
}
