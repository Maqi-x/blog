#include <blog/html.h>

void bl_escape_html(StringBuf* sb, StringView text) {
    for (usize i = 0; i < text.len; ++i) {
        char c = text.data[i];

        switch (c) {
        case '&':  sb_append(sb, SV("&amp;"));  break;
        case '<':  sb_append(sb, SV("&lt;"));   break;
        case '>':  sb_append(sb, SV("&gt;"));   break;
        case '"':  sb_append(sb, SV("&quot;")); break;
        case '\'': sb_append(sb, SV("&#39;"));  break;
        default:   sb_append_char(sb, c);       break;
        }
    }
}
