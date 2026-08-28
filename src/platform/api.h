#pragma once

#include "core/defs.h"

API void fs_init();
API bool fs_ready();
API void fs_mark_ready();
API char *fs_binary_path();
API bool fs_save_file(const char *file_name, const void *data, int data_size);
API unsigned char *fs_load_file(const char *file_name, int *data_size);
API bool device_is_mobile();
API bool device_has_touch();
