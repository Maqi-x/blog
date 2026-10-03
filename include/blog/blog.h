#pragma once

#include <blog/post.h>
#include <vector.h>

VECTOR_DECLARE(BlBlog, bl_blog, BlPost);

void bl_blog_print(const BlBlog* blog, FILE* out);
void bl_discover(BlBlog* blog, const char* path);
