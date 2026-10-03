#include <blog/utils.h>
#include <blog/error.h>
#include <blog/post.h>
#include <blog/defs.h>

#include <blog/lexer.h>
#include <blog/parser.h>

#include <libgen.h>
#include <string.h>
#include <stdio.h>

VECTOR_DEFINE(BlParts, bl_parts, BlPart);
VECTOR_DEFINE(BlBlocks, bl_blocks, BlBlock);

BlPost bl_post_open(const char* path) {

    StringView content = bl_read_entire_file(path);
    if (sv_is_null(content)) {
        bl_error("failed to read the file");
    }

    BlLexer lexer;
    bl_lexer_init(&lexer, content);

    BlParser parser;
    bl_parser_init(&parser, &lexer);


    BlPost post = bl_parse_post(&parser);

    usize path_len = strlen(path);
    char* pm = malloc(path_len + 1);
    memcpy(pm, path, path_len + 1);

    char* cname = basename(pm);
    StringView name = sv_from_cstr(cname);

    if (sv_is_null(post.meta.id)) {
        post.meta.id = sv_trim_suffix(name, SV(".post"));
    }

    // content intentionally leaked
    return post;
}

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

void bl_post_print(const BlPost* post, const char* pre, FILE* out) {
    if (pre == NULL) pre = "";

    fprintf(out, "%stitle: ", pre);
    sv_print(post->meta.title, out);
    fputc('\n', out);
    fprintf(out, "%sdesc:  ", pre);
    sv_print(post->meta.desc, out);
    fputc('\n', out);
    fprintf(out, "%sid:    ", pre);
    sv_print(post->meta.id, out);
    fputc('\n', out);

    fprintf(out, "%sblocks:\n", pre);
    for (BlBlock* block = post->blocks.begin; block < post->blocks.end; ++block) {
        fprintf(out, "%s  "SV_FMT":\n", pre, SV_FARG(bl_block_kind_name(block->kind)));

        if (block->kind == BL_BLOCK_CODE) {
            fprintf(out, "%s    lang: ", pre);
            sv_print(block->as.code.lang, out);
            fputc('\n', out);
            fprintf(out, "%s    text: ", pre);
            sv_print(block->as.code.text, out);
            fputc('\n', out);
            continue;
        }

        fprintf(out, "%s    parts:\n", pre);
        for (BlPart* part = block->as.parts.begin; part < block->as.parts.end; ++part) {
            fprintf(out, "%s      [", pre);
            print_part_flags(part->flags, out);
            fputs("] ", out);
            sv_print(part->content, out);
            fputc('\n', out);
        }
    }
}
