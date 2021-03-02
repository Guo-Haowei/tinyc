#include "toyc.h"

Buffer *make_buffer(int capacity) {
    Buffer *buf = CALLOC(sizeof(Buffer));
    buf->start = CALLOC(capacity);
    buf->p = buf->start;
    buf->capacity = capacity;
    *buf->start = '\0';
    return buf;
}

void clear_buffer(Buffer *buf) {
    buf->p = buf->start;
    *buf->start = '\0';
}

void destroy_buffer(Buffer **pBuffer) {
    FREE((*pBuffer)->start);
    (*pBuffer)->capacity = 0;
    (*pBuffer)->p = 0;
    (*pBuffer)->start = 0;
    FREE(*pBuffer);
    *pBuffer = NULL;
}

void buffer_append_char(Buffer *buf, int c) {
    assert(buf->p + 1 < buf->start + buf->capacity);
    *(buf->p) = c;
    *(++buf->p) = '\0';
}

void buffer_append_str(Buffer *buf, const char *str) {
    int len = (int)strlen(str);
    assert(buf->p + len < buf->start + buf->capacity);
    strcpy(buf->p, str);
    buf->p += len;
}

void buffer_append_fmt(Buffer *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int offset = vsnprintf(buf->p, buf->start + buf->capacity - buf->p, fmt, args);
    buf->p += offset;
    va_end(args);
}
