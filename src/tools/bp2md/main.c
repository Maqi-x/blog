#include <blog/lexer.h>
#include <blog/parser.h>

int main() {
    BlLexer lexer;
    bl_lexer_init(&lexer, SV(
        ":shit: value!\n*He/ll/o,* world!\n\n== Title\n--- Subtitle\n:: Comment\n"
        "Blah blah blah\n`code1`, ```code2```, */x/*"));

    BlToken token;
    while ((token = bl_lexer_next(&lexer)).type != BL_TT_EOF) {
        bl_token_print(token, stdout);
        putchar('\n');
    }

    putchar('\n');

    BlParser parser;
    bl_parser_init(&parser, &lexer);
    BlPost post = bl_parse_post(&parser);
    bl_post_print(&post, stdout);
}
