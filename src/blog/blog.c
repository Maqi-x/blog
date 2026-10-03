#include <blog/blog.h>
#include <blog/defs.h>
#include <blog/error.h>

#include <glob.h>

VECTOR_DEFINE(BlBlog, bl_blog, BlPost);

void bl_discover(BlBlog* blog, const char* path) {
    char pattern[strlen(path) + sizeof("/*.post")];
    snprintf(pattern, sizeof pattern, "%s/*.post", path);

    glob_t posts;
    int rc = glob(pattern, 0, NULL, &posts);

    if (rc == 0) {
        for (usize i = 0; i < posts.gl_pathc; i++) {
            const char* path = posts.gl_pathv[i];
            BlPost post = bl_post_open(path);
            bl_blog_push(blog, post);
        }
        globfree(&posts);
    } else if (rc == GLOB_NOMATCH) {
        bl_error("no posts found in the specified directory");
    } else {
        bl_error("glob() failed");
    }
}

void bl_blog_print(const BlBlog* blog, FILE* out) {
    fputs("posts:\n", out);
    for (const BlPost* post = blog->begin; post < blog->end; ++post) {
        bl_post_print(post, "  ", out);
    }
}
