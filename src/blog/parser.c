#include <blog/parser.h>
#include <blog/error.h>
#include <blog/post.h>

#define PUSH_BLOCK(VEC, ...) \
    bl_blocks_push(VEC, (BlBlock) __VA_ARGS__)

#define PUSH_PART(VEC, ...) \
    bl_parts_push(VEC, (BlPart) __VA_ARGS__)

void bl_parser_init(BlParser* parser, BlLexer* lexer) {
    *parser = (BlParser) {
        .lexer = lexer,
        .lookahead = bl_lexer_next(lexer),
    };
}

static BlToken peek(const BlParser* parser) {
    return parser->lookahead;
}

static BlToken advance(BlParser* parser) {
    BlToken token = parser->lookahead;
    if (token.type != BL_TT_EOF)
        parser->lookahead = bl_lexer_next(parser->lexer);
    return token;
}

static bool check(const BlParser* parser, BlTokenType type) {
    return parser->lookahead.type == type;
}

//static bool match(BlParser* parser, BlTokenType type) {
//    if (!check(parser, type))
//        return false;
//
//    advance(parser);
//    return true;
//}

BlToken bl_parser_expect(BlParser* parser, BlTokenType type) {
    if (check(parser, type))
        return advance(parser);

    BlToken actual = peek(parser);
    bl_error(
        "%u:%u: expected "SV_FMT", got "SV_FMT,
        actual.line, actual.col,
        SV_FARG(bl_token_type_name(type)),
        SV_FARG(bl_token_type_name(actual.type))
    );
}

static void flush(BlParser* parser) {
    if (VECTOR_SIZE(&parser->parts) != 0) {
        PUSH_BLOCK(&parser->blocks, {
            .kind = BL_BLOCK_TEXT,
            .as.parts = parser->parts,
        });

        //bl_parts_clear(&parser->parts);
        parser->parts = (BlParts) {0};
    }
}

BlPost bl_parse_post(BlParser* parser) {
    while (!check(parser, BL_TT_EOF)) {
        BlToken tok = advance(parser);
        switch (tok.type) {
        case BL_TT_TEXT:
            PUSH_PART(&parser->parts, {
                .content = tok.lexeme,
                .flags = parser->pf,
            });
            continue;
        case BL_TT_CODE_INLINE:
            PUSH_PART(&parser->parts, {
                .content = tok.lexeme,
                .flags = parser->pf | BL_PART_MONO,
            });
            continue;
        case BL_TT_BOLD:
            parser->pf ^= BL_PART_BOLD;
            break;
        case BL_TT_ITALIC:
            parser->pf ^= BL_PART_ITALIC;
            break;
        default:
            flush(parser);
        }
    }

    if (parser->pf & BL_PART_BOLD) {
        bl_error("unterminated *bold*");
    }
    if (parser->pf & BL_PART_ITALIC) {
        bl_error("unterminated /italic/");
    }

    return (BlPost) {
        .meta = {
            .title = SV("My cool post!"),
            .desc = SV("Post about cool stuff"),
            .id = SV("my-cool-post"),
        },
        .blocks = parser->blocks,
    };
}
