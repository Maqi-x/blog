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

    BL_TT_DIR_BEGIN,   //< [dir
    BL_TT_ARG_SEP,     //< ;
    BL_TT_DIR_END,     //< ]
} BlTokenType;

typedef struct {
    BlTokenType type;
    StringView lexeme;
    usize unescaped_len;
    uint line, col;
} BlToken;

StringView bl_token_type_name(BlTokenType tt);
StringView bl_token_unescape(BlToken token);
void bl_token_print(BlToken token, FILE* out);
