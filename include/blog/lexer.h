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

    // a semicolon only becomes an arg separator if we're inside a directive
    // again it's because of syntax ambiguity
    uint directive_depth;
} BlLexer;

void bl_lexer_init(BlLexer* lexer, StringView input);
BlToken bl_lexer_next(BlLexer* lexer);
