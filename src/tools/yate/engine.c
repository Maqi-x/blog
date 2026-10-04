#include <tools/yate.h>
#include <blog/utils.h>
#include <blog/error.h>

void cescape(char c, FILE* out) {
    switch (c) {
    case '\\': fputs("\\\\", out); break;
    case '"':  fputs("\\\"", out); break;
    case '\n': fputs("\\n",  out); break;
    case '\t': fputs("\\t",  out); break;
    case '\r': fputs("\\r",  out); break;
    case '\0': fputs("\\0",  out); break;
    default:
        if (c >= 32 && c <= 126) {
            fputc(c, out);
        } else {
            fprintf(out, "\\%03o", (unsigned char)c);
        }
        break;
    }
}

void enter_output(FILE* out, bool* in) {
    if (!*in) {
        fputs("\nsoutput(\"", out);
        *in = true;
    }
}
void leave_output(FILE* out, bool* in) {
    if (*in) {
        fputs("\");\n", out);
        *in = false;
    }
}

void yate_translate(const char* input_path, FILE* out) {
    StringView template = bl_read_entire_file(input_path);
    if (sv_is_null(template)) {
        bl_error("failed to read the input file");
    }

    fprintf(out, "// GENERATED FROM %s. do not modify directly.\n", input_path);

    bool code = false;
    bool code_inline;

    bool brand_new_line = true;
    bool in_output_stmt = false;

    for (usize i = 0; i < template.len; ++i) {
        char c1 = template.data[i],
             c2 = (i + 1 < template.len)
                 ? template.data[i + 1]
                 : '\0';

        if (brand_new_line && c1 == '$') {
            leave_output(out, &in_output_stmt);
            brand_new_line = false;
            code = true, code_inline = false;
            continue;
        } else if (c1 == '$' && c2 == '{') {
            leave_output(out, &in_output_stmt);
            brand_new_line = false;
            code = true, code_inline = true;
            i += 1; // to skip the $ sign
            continue;
        } else if (!isspace((uchar)c1)) {
            brand_new_line = false;
        }

        if (c1 == '\n')
            brand_new_line = true;

        if (code) {
            if (!code_inline && c1 == '\n') {
                // i know this is very inefficient,
                // but it works.
                enter_output(out, &in_output_stmt);
                cescape('\n', out);
                leave_output(out, &in_output_stmt);
                code = false;
                continue;
            } else if (code_inline && c1 == '}' && c2 == '$') {
                code = false;
                i += 1; // to skip the } brace
                continue;
            }
        }

        if (code) {
            fputc(c1, out);
        } else {
            enter_output(out, &in_output_stmt);
            cescape(c1, out);
        }
    }

    leave_output(out, &in_output_stmt);
}
