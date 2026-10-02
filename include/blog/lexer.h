#pragma once

#include <blog/token.h>
#include <blog/defs.h>

typedef struct {
    StringView input;
    uint pos;

    uint line, col;

    // we only want to parse :attr: attributes at the beggining of the file
    // because the syntax may be a little bit ambiguous.
    bool header_done;
} BlLexer;

void bl_lexer_init(BlLexer* lexer, StringView input);
BlToken bl_lexer_next(BlLexer* lexer);
