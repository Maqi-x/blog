#include <blog/token.h>

StringView bl_token_type_name(BlTokenType tt) {
    switch (tt) {
    case BL_TT_EOF:     return SV("eof");

    case BL_TT_ATTR:    return SV("attr");
    case BL_TT_TEXT:    return SV("text");
    case BL_TT_COMMENT: return SV("comment");

    case BL_TT_BOLD:    return SV("bold");
    case BL_TT_ITALIC:  return SV("italic");
    case BL_TT_H1:      return SV("h1");
    case BL_TT_H2:      return SV("h2");

    case BL_TT_HARDBREAK: return SV("hard-break");
    case BL_TT_SOFTBREAK: return SV("soft-break");

    case BL_TT_CODE_INLINE: return SV("code-inline");
    case BL_TT_CODE_BLOCK:  return SV("code-block");
    }

    bl_unreachable();
}

void bl_token_print(BlToken token, FILE* out) {
    fprintf(out, "%u:%u:", token.line, token.col);
    sv_print(bl_token_type_name(token.type), out);

    if (!sv_is_null(token.lexeme)) {
        fputc('(', out);
        sv_print(token.lexeme, out);
        fputc(')', out);
    }
}
