#pragma once

#include <strlib/sv.h>
#include <vector.h>

// TODO: links

typedef enum {
    BL_PART_BOLD   = 1 << 0, // *bold*
    BL_PART_ITALIC = 1 << 1, // /italic/
    BL_PART_MONO   = 1 << 2, // `code`
} BlPartFlags;

typedef struct {
    BlPartFlags flags;
    StringView content;
} BlPart;

VECTOR_DECLARE(BlParts, bl_parts, BlPart);

typedef enum {
    BL_BLOCK_H1,   // == H1
    BL_BLOCK_H2,   // --- H2
    BL_BLOCK_CODE, // ```code```
    BL_BLOCK_TEXT, // just text
} BlBlockKind;

typedef struct {
    StringView lang;
    StringView text;
} BlCode;

typedef struct {
    BlBlockKind kind;
    union {
        // for h1, h2 and text
        BlParts parts;

        // for code blocks
        BlCode code;
    } as;
} BlBlock;

StringView bl_block_kind_name(BlBlockKind kind);

VECTOR_DECLARE(BlBlocks, bl_blocks, BlBlock);
// bl_blocks_free won't actually free the inner parts array which
// is probably fine. idfc. the kernel will free things for us.

typedef struct {
    StringView title; // e.g. My cool post!
    StringView desc;  // e.g. Post about cool stuff
    StringView id;    // e.g. my-cool-post
    // TODO: maybe some sort of #tags
} BlPostMeta;

typedef struct {
    BlPostMeta meta;
    BlBlocks blocks;
} BlPost;

BlPost bl_post_open(const char* path);
void bl_post_print(const BlPost* post, const char* pre, FILE* out);
