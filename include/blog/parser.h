#pragma once

#include <blog/lexer.h>
#include <blog/post.h>

typedef struct {
    BlLexer* lexer;
    BlToken lookahead;
} BlParser;

void bl_parser_init(BlParser* parser, BlLexer* lexer);
BlPost bl_parse_post(BlParser* parser);
