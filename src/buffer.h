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
void buffer_delete(Buffer *buffer, size_t index, size_t count);
void buffer_read(Buffer *buffer, const char *path, const char *indent);
void buffer_insert_char(Buffer *buffer, char ch);

void buffer_cursor_up(Buffer *buffer);
void buffer_cursor_down(Buffer *buffer);
void buffer_cursor_left(Buffer *buffer);
void buffer_cursor_right(Buffer *buffer);
void buffer_cursor_top(Buffer *buffer);
void buffer_cursor_bottom(Buffer *buffer);
void buffer_cursor_start(Buffer *buffer);
void buffer_cursor_end(Buffer *buffer);

void buffer_delete_left(Buffer *buffer);
void buffer_delete_right(Buffer *buffer);

#endif // BUFFER_H
