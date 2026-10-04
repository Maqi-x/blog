#pragma once

#include <strlib/sb.h>
#include <strlib/sv.h>

#include <blog/post.h>
#include <blog/defs.h>

void bl_escape_html(StringBuf* sb, StringView text);

void bl_part_to_html(StringBuf* sb, const BlPart* part);
void bl_parts_to_html(StringBuf* sb, const BlParts* parts);
void bl_block_to_html(StringBuf* sb, const BlBlock* block);
void bl_blocks_to_html(StringBuf* sb, const BlBlocks* blocks);
