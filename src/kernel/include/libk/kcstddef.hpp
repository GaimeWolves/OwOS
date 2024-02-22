#pragma once

#ifndef _GCC_MAX_ALIGN_T // nasty hack because libstdc++ cstddef header is *somehow* missing max_align_t
#define _GCC_MAX_ALIGN_T
typedef long double max_align_t;
#endif

#include <cstddef>

