#include <blog/post.h>
#include <blog/defs.h>

#include <stdio.h>

VECTOR_DEFINE(BlParts, bl_parts, BlPart);
VECTOR_DEFINE(BlBlocks, bl_blocks, BlBlock);

static void print_part_flags(BlPartFlags flags, FILE* out) {
    bool printed = false;

    if (flags & BL_PART_BOLD) {
        fputs("bold", out);
        printed = true;
    }
    if (flags & BL_PART_ITALIC) {
        if (printed) fputc('|', out);
        fputs("italic", out);
        printed = true;
    }
    if (flags & BL_PART_MONO) {
        if (printed) fputc('|', out);
        fputs("mono", out);
        printed = true;
    }

    if (!printed)
        fputs("plain", out);
}

StringView bl_block_kind_name(BlBlockKind kind) {
    switch (kind) {
    case BL_BLOCK_H1:   return SV("h1");
    case BL_BLOCK_H2:   return SV("h2");
    case BL_BLOCK_CODE: return SV("code");
    case BL_BLOCK_TEXT: return SV("text");
    }

    bl_unreachable();
}

void bl_post_print(const BlPost* post, FILE* out) {
    fputs("post:\n", out);
    fputs("  title: ", out);
    sv_print(post->meta.title, out);
    fputc('\n', out);
    fputs("  desc:  ", out);
    sv_print(post->meta.desc, out);
    fputc('\n', out);
    fputs("  id:    ", out);
    sv_print(post->meta.id, out);
    fputc('\n', out);

    fputs("  blocks:\n", out);
    for (BlBlock* block = post->blocks.begin; block < post->blocks.end; ++block) {
        fprintf(out, "    "SV_FMT":\n", SV_FARG(bl_block_kind_name(block->kind)));

        if (block->kind == BL_BLOCK_CODE) {
            fputs("      lang: ", out);
            sv_print(block->as.code.lang, out);
            fputc('\n', out);
            fputs("      text: ", out);
            sv_print(block->as.code.text, out);
            fputc('\n', out);
            continue;
        }

        fputs("      parts:\n", out);

        for (BlPart* part = block->as.parts.begin; part < block->as.parts.end; ++part) {
            fputs("        [", out);
            print_part_flags(part->flags, out);
            fputs("] ", out);
            sv_print(part->content, out);
            fputc('\n', out);
        }
    }
}
