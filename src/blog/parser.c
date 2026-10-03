#include <blog/parser.h>
#include <blog/error.h>
#include <blog/post.h>

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

BlPost bl_parse_post(BlParser* parser) {
    (void) parser;
    return (BlPost) {
        .meta = {
            .title = SV("My cool post!"),
            .desc = SV("Post about cool stuff"),
            .id = SV("my-cool-post"),
        },
    };
}
