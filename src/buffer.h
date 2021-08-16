#ifndef BUFFER_H
#define BUFFER_H

#include "line.h"

typedef struct {
    Line *lines;
    size_t length;
    size_t capacity;
} Buffer;

void buffer_free(Buffer *buffer);
void buffer_insert(Buffer *buffer, size_t index, Line line);
void buffer_append(Buffer *buffer, Line line);
void buffer_read(Buffer *buffer, const char *file_path);

#endif // BUFFER_H
