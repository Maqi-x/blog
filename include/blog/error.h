#pragma once

__attribute__((format(printf, 1, 2)))
_Noreturn void bl_error(const char* fmt, ...);
