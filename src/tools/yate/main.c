#include <blog/error.h>

#include <argparse.h>
#include <stdio.h>

#include <tools/yate.h>

static const char* const usages[] = {
    "yate [options] <input.yate> -o <output.h>",
    NULL,
};

int main(int argc, const char** argv) {
    const char* input_path;
    const char* output_path;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_STRING('i', "input",  &input_path,  "the input file",  NULL, 0, 0),
        OPT_STRING('o', "output", &output_path, "the output file", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);

    argparse_describe(&argparse,
        "Convert a .yate file into a C header file",
        NULL);

    argc = argparse_parse(&argparse, argc, argv);

    if (argc != 0)
        bl_error("too much arguments provided");


    if (input_path == NULL) {
        bl_error("input file must be provided");
    }

    FILE* output;
    if (output_path == NULL) {
        output = stdout;
    } else {
        output = fopen(output_path, "w");
        if (output == NULL) bl_error("failed to open the output file");
    }

    yate_translate(input_path, output);
}
