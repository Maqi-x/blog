#include <blog/utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

StringView bl_read_entire_file(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0)
        return SV_NULL;

    FILE* fp = fopen(path, "rb");
    if (!fp) return SV_NULL;

    char* buf = malloc(st.st_size + 1);
    if (buf == NULL) {
        fclose(fp);
        return SV_NULL;
    }

    usize len = fread(buf, 1, st.st_size, fp);

    fclose(fp);
    return (StringView) {
        .data = buf,
        .len = len,
    };
}

