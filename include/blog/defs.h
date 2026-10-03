#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef unsigned char uchar;
typedef unsigned int uint;
typedef size_t usize;

#define bl_unreachable() __builtin_unreachable()
