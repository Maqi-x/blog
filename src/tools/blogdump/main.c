#include <blog/blog.h>

int main() {
    BlBlog blog = {0};
    bl_discover(&blog, "content");
    bl_blog_print(&blog, stdout);
}
