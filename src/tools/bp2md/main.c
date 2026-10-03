#include <blog/lexer.h>
#include <blog/parser.h>

int main() {
    BlLexer lexer;
    bl_lexer_init(&lexer, SV(
        ":title: First post\n"
        ":desc: Nothing special. Just a dummy post for testing.\n"
        ":: :id: overriden\n"
        "\n"
        ":: Comment\n"
        "\n"
        "== H1\n"
        "--- H2\n"
        "\n"
        "Hello, world!\n"
        "This is another line separated with a soft-break\n"
        "/So it actually displays as one line/\n"
        "\n"
        "*bold*\n"
        "/italic/\n"
        "`code`\n"
        "*mixed /mixed/ `mixed` /`mixed`/*"
        "\n"
        "```c\n"
        "int main() {\n"
        "    foo();\n"
        "}\n"
        "```\n"
    ));

    BlToken token;
    while ((token = bl_lexer_next(&lexer)).type != BL_TT_EOF) {
        bl_token_print(token, stdout);
        putchar('\n');
    }

    putchar('\n');

    // TODO: bl_lexer_reset
    bl_lexer_init(&lexer, lexer.input);

    BlParser parser;
    bl_parser_init(&parser, &lexer);
    BlPost post = bl_parse_post(&parser);
    bl_post_print(&post, stdout);
}
