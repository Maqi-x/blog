#include <blog/error.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

_Noreturn void bl_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    // TODO: fancy ansi formatting
    fputs("error: ", stderr);
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);

    va_end(args);

    exit(EXIT_FAILURE);
}
