#include <strlib/sb.h>
#include <blog/blog.h>

void generate_index_page(BlBlog* blog) {
    StringBuf sb;
    #define soutput(s) sb_append(&sb, SV(s))
    #define output(s) sb_append(&sb, s)
    #include "index.html.h"
    #undef output
    #undef soutput
    sv_print(sb_view(&sb), stdout);
}

int main() {
    BlBlog blog = {0};
    bl_discover(&blog, "content");
    generate_index_page(&blog);
}
