#pragma once

#include <strlib/sv.h>
#include <blog/defs.h>

typedef enum {
    BL_TT_EOF,     //< end of file

    BL_TT_ATTR,    //< :attrname:
    BL_TT_COMMENT, //< :: Comment
    BL_TT_TEXT,    //< Any text (can contain spaces)

    BL_TT_SOFTBREAK, //< Soft break (displays as a space)
    BL_TT_HARDBREAK, //< Actual line break

    BL_TT_H1,     //< ==
    BL_TT_H2,     //< ---
    BL_TT_BOLD,   //< *
    BL_TT_ITALIC, //< /

    BL_TT_CODE_INLINE, //< `...`
    BL_TT_CODE_BLOCK,  //< ```...```
} BlTokenType;

typedef struct {
    BlTokenType type;
    StringView lexeme;
    uint line, col;
} BlToken;

StringView bl_token_type_name(BlTokenType tt);
void bl_token_print(BlToken token, FILE* out);
