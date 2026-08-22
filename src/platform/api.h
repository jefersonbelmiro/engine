#pragma once

#include "core/defs.h"

API bool platform_is_mobile();
API bool platform_has_touch();
API bool platform_is_ready();
API bool platform_save_data();
API char *platform_binary_path();
API bool platform_save_file(const char *file_name, const void *data, const int data_size);
API unsigned char* platform_load_file(const char *file_name, int *data_size);
