#ifndef BUFFER_H
#define BUFFER_H

#include "line.h"

typedef struct {
    // Text
    Line *lines;
    size_t length;
    size_t capacity;

    // Cursor position
    size_t row;
    size_t col;
} Buffer;

void buffer_free(Buffer *buffer);
void buffer_insert(Buffer *buffer, size_t index, Line line);
void buffer_append(Buffer *buffer, Line line);
void buffer_read(Buffer *buffer, const char *path);
void buffer_insert_char(Buffer *buffer, char ch);

void buffer_cursor_down(Buffer *buffer);
void buffer_cursor_up(Buffer *buffer);
void buffer_cursor_left(Buffer *buffer);
void buffer_cursor_right(Buffer *buffer);

#endif // BUFFER_H
