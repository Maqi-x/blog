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

static void push_text_part(BlParser* parser, StringView content, BlPartFlags flags) {
    PUSH_PART(&parser->parts, {
        .content = content,
        .flags = parser->pf | flags,
    });
}

static void parse_attr(BlParser* parser, BlToken attr, BlPostMeta* meta) {
    BlToken value = bl_parser_expect(parser, BL_TT_TEXT);
    StringView text = sv_trim(value.lexeme, isspace);

    if (sv_eql(attr.lexeme, SV("title"))) {
        meta->title = text;
    } else if (sv_eql(attr.lexeme, SV("desc"))) {
        meta->desc = text;
    } else if (sv_eql(attr.lexeme, SV("id"))) {
        meta->id = text;
    } else {
        bl_error(
            "%u:%u: unknown attribute "SV_FMT,
            attr.line, attr.col, SV_FARG(attr.lexeme)
        );
    }
}

static void push_line_block(BlParser* parser, BlBlockKind kind, StringView content) {
    BlParts parts = {0};
    PUSH_PART(&parts, {
        .content = content,
        .flags = 0,
    });

    PUSH_BLOCK(&parser->blocks, {
        .kind = kind,
        .as.parts = parts,
    });
}

static void push_code_block(BlParser* parser, StringView content) {
    StringView lang, text;

    uint i = 0;
    while (i < content.len) {
        if (content.data[i++] == '\n') break;
    }

    if (i == content.len) {
        lang = SV_NULL;
        text = content;
    } else {
        lang = sv_slice(content, 0, i - 1);
        text = sv_slice(content, i, content.len);
    }

    PUSH_BLOCK(&parser->blocks, {
        .kind = BL_BLOCK_CODE,
        .as.code = {
            .lang = lang,
            .text = text,
        },
    });
}

BlPost bl_parse_post(BlParser* parser) {
    BlPostMeta meta = { 0 };

    while (!check(parser, BL_TT_EOF)) {
        BlToken tok = advance(parser);
        switch (tok.type) {
        case BL_TT_ATTR:
            parse_attr(parser, tok, &meta);
            continue;
        case BL_TT_TEXT:
            push_text_part(parser, tok.lexeme, 0);
            continue;
        case BL_TT_CODE_INLINE:
            push_text_part(parser, tok.lexeme, BL_PART_MONO);
            continue;
        case BL_TT_SOFTBREAK:
            if (VECTOR_SIZE(&parser->parts) != 0)
                push_text_part(parser, SV(" "), 0);
            continue;
        case BL_TT_HARDBREAK:
            flush(parser);
            continue;
        case BL_TT_H1:
            flush(parser);
            push_line_block(parser, BL_BLOCK_H1, tok.lexeme);
            continue;
        case BL_TT_H2:
            flush(parser);
            push_line_block(parser, BL_BLOCK_H2, tok.lexeme);
            continue;
        case BL_TT_CODE_BLOCK:
            flush(parser);
            push_code_block(parser, tok.lexeme);
            continue;
        case BL_TT_BOLD:
            parser->pf ^= BL_PART_BOLD;
            break;
        case BL_TT_ITALIC:
            parser->pf ^= BL_PART_ITALIC;
            break;
        default:
            (void)tok.type;
        }
    }

    flush(parser);

    if (parser->pf & BL_PART_BOLD)
        bl_error("unterminated *bold*");
    if (parser->pf & BL_PART_ITALIC)
        bl_error("unterminated /italic/");

    return (BlPost) {
        .meta = meta,
        .blocks = parser->blocks,
    };
}
