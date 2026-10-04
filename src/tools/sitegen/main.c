#include <blog/blog.h>
#include <blog/error.h>
#include <blog/post.h>
#include <blog/html.h>

#include <argparse.h>
#include <strlib/sb.h>
#include <stdio.h>

#ifndef TEMPLATE_HEADER
    #ifdef _LSP
        // complex meta programming in C is kinda pain in the ass
        // i want my lsp to work here so i think it's the best solution
        // to just define TEMPLATE_HEADER to some example file
        #define TEMPLATE_HEADER "index.html.h"
    #else
        #error TEMPLATE_HEADER must be defined
    #endif
#endif

#define RENDER_INDEX 0
#define RENDER_POST 1

#ifndef RENDER_MODE
    #ifdef _LSP
        #define RENDER_MODE RENDER_INDEX
    #else
        #error "render mode not specified"
    #endif
#endif

#if RENDER_MODE != RENDER_INDEX && RENDER_MODE != RENDER_POST
    #error "unknown render more. use 0 for index and 1 for post"
#endif

static const char* const usages[] = {
#if RENDER_MODE == RENDER_POST
    "sitegen -i <input.html.h> -o <output.html>",
#elif RENDER_MODE == RENDER_INDEX
    "sitegen -i <content-directory> -o <output.html>",
#endif
    NULL,
};

void _print_escaped(StringView s, FILE* out) {
    StringBuf sb;
    sb_init(&sb);
    bl_escape_html(&sb, s);
    sv_print(sb_view(&sb), out);
    sb_free(&sb);
}

void _print_htmlified(const BlBlocks* blocks, FILE* out) {
    StringBuf sb;
    sb_init(&sb);
    bl_blocks_to_html(&sb, blocks);
    sv_print(sb_view(&sb), out);
    sb_free(&sb);
}

void render(const char* input_path, FILE* out) {
    #define soutput(s) fputs(s, out)
    #define output(s) sv_print(s, out)
    #define escape(s) _print_escaped(s, out)

#if RENDER_MODE == RENDER_INDEX
    BlBlog blog = {0};
    bl_discover(&blog, input_path);
    #include TEMPLATE_HEADER
#elif RENDER_MODE == RENDER_POST
    #define htmlify(b) _print_htmlified(&b, out)
    BlPost post = bl_post_open(input_path);
    #include TEMPLATE_HEADER
    #undef htmlify
#endif

    #undef escape
    #undef output
    #undef soutput

}

int main(int argc, const char** argv) {
    const char* input_path = NULL;
    const char* output_path = NULL;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_STRING('i', "input",  &input_path,  "input content path", NULL, 0, 0),
        OPT_STRING('o', "output", &output_path, "output html path",   NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argc = argparse_parse(&argparse, argc, argv);

    if (argc != 0)
        bl_error("too much arguments provided");
    if (input_path == NULL)
        bl_error("input path must be provided");
    if (output_path == NULL)
        bl_error("output path must be provided");

    FILE* output = fopen(output_path, "w");
    if (output == NULL) bl_error("failed to open output file");

    render(input_path, output);
    fclose(output);
}
