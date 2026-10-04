#include <blog/html.h>
#include <blog/error.h>
#include <c2html.h>

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

static void code_block_to_html(StringBuf* sb, const BlCode* code) {
    // c2html seems to work with other languages just fine.
    // of course not everything is highlighted but it's still
    // way better than no highlighting at all.
    //if (sv_eql(code->lang, SV("c"))) {
    if (!sv_is_null(code->lang)) {
        long output_len;
        const char* error;

        char* output = c2html(
            code->text.data, code->text.len,
            "c2h-", &output_len, &error
        );

        if (error != NULL)
            bl_error("c2html error: %s", error);

        sb_append(sb, sv_from_data_and_len(output, output_len));
        free(output);
    } else {
        sb_append(sb, SV("<pre><code>"));
        bl_escape_html(sb, code->text);
        sb_append(sb, SV("</code></pre>"));
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
        sb_append(sb, SV("<div class=\"code-block\">"));
        code_block_to_html(sb, &block->as.code);
        sb_append(sb, SV("</div>"));
        break;
    }

    return;
}

static bool is_block_element(BlBlockKind kind) {
    return kind == BL_BLOCK_H1
        || kind == BL_BLOCK_H2;
}

void bl_blocks_to_html(StringBuf* sb, const BlBlocks* blocks) {
    for (const BlBlock* b = blocks->begin; b < blocks->end; ++b) {
        bl_block_to_html(sb, b);

        bool has_next = (b < blocks->end - 1);
        if (has_next && !is_block_element(b->kind) && !is_block_element((b + 1)->kind)) {
            sb_append(sb, SV("<br>"));
        }
    }
}
