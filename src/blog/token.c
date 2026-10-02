#include <blog/token.h>

void bl_token_print(BlToken token, FILE* out) {
    fprintf(out, "%u:%u:", token.line, token.col);

    switch (token.type) {
    case BL_TT_EOF:     fputs("eof", out);     break;

    case BL_TT_ATTR:    fputs("attr", out);    break;
    case BL_TT_TEXT:    fputs("text", out);    break;
    case BL_TT_COMMENT: fputs("comment", out); break;

    case BL_TT_BOLD:    fputs("bold", out);    break;
    case BL_TT_ITALIC:  fputs("italic", out);  break;
    case BL_TT_H1:      fputs("h1", out);      break;
    case BL_TT_H2:      fputs("h2", out);      break;

    case BL_TT_HARDBREAK:
        fputs("hard-break", out);
        break;
    case BL_TT_SOFTBREAK:
        fputs("soft-break", out);
        break;

    case BL_TT_CODE_INLINE:
        fputs("code-inline", out);
        break;
    case BL_TT_CODE_BLOCK:
        fputs("code-block", out);
        break;
    }

    if (!sv_is_null(token.lexeme)) {
        fputc('(', out);
        sv_print(token.lexeme, out);
        fputc(')', out);
    }
}
