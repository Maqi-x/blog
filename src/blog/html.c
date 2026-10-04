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

void bl_part_to_html(StringBuf* sb, const BlPart* part) {
    // maybe it should be <strong> and <em> but i kinda don't
    // understand the difference.
    if (part->flags & BL_PART_BOLD)   sb_append(sb, SV("<b>"));
    if (part->flags & BL_PART_ITALIC) sb_append(sb, SV("<i>"));
    if (part->flags & BL_PART_MONO)   sb_append(sb, SV("<code>"));

    bl_escape_html(sb, part->content);

    if (part->flags & BL_PART_MONO)   sb_append(sb, SV("</code>"));
    if (part->flags & BL_PART_ITALIC) sb_append(sb, SV("</i>"));
    if (part->flags & BL_PART_BOLD)   sb_append(sb, SV("</b>"));
}

void bl_parts_to_html(StringBuf* sb, const BlParts* parts) {
    for (const BlPart* p = parts->begin; p < parts->end; ++p) {
        bl_part_to_html(sb, p);
    }
}

void bl_block_to_html(StringBuf* sb, const BlBlock* block) {
    switch (block->kind) {
    case BL_BLOCK_TEXT:
        bl_parts_to_html(sb, &block->as.parts);
        break;
    case BL_BLOCK_H1:
        sb_append(sb, SV("<h1>"));
        bl_parts_to_html(sb, &block->as.parts);
        sb_append(sb, SV("</h1>"));
        break;
    case BL_BLOCK_H2:
        sb_append(sb, SV("<h2>"));
        bl_parts_to_html(sb, &block->as.parts);
        sb_append(sb, SV("</h2>"));
        break;
    case BL_BLOCK_CODE:
        // TODO: syntax highlighting
        (void) block->as.code.lang;
        sb_append(sb, SV("<pre><code>"));
        bl_escape_html(sb, block->as.code.text);
        sb_append(sb, SV("</code></pre>"));
    }

    sb_append(sb, SV("<br>"));
    return;
}

void bl_blocks_to_html(StringBuf* sb, const BlBlocks* blocks) {
    for (const BlBlock* b = blocks->begin; b < blocks->end; ++b) {
        bl_block_to_html(sb, b);
    }
}
