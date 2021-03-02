#include "toyc.h"

/// TODO: refactor: make it a string table
static FileCache s_fileCache[MAX_FILES_INCLUDED];
static int s_fileCacheCnt = 0;

/// TODO: implemt cache
FileCache *filecache_get(char const *path) {
    assert(s_fileCacheCnt + 1 < MAX_FILES_INCLUDED);

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        /// TODO: handle errors properly
        log("No such file or directory\n");
        exit(-1);
    }

    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    /// TODO: free
    /// TODO: only add once
    char *content = CALLOC(len + 1);

    fread(content, 1, len, fp);
    fclose(fp);

    // initialize file cache
    FileCache *tf = &s_fileCache[s_fileCacheCnt++];
    strncpy(tf->path, path, MAX_PATH_SIZE);
    tf->content = content;
    return tf;
}
