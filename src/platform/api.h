#pragma once

#include "core/defs.h"

API const char *platform_name();
API void        platform_help();
API bool        platform_build();
API bool        platform_run();
API bool        platform_backend_build();
API bool        platform_is_mobile();
API bool        platform_has_touch();
API bool        platform_is_ready();
