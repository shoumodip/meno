#ifndef BUFFER_H
#define BUFFER_H

#include "string.h"

typedef struct {
    String file;

    String *lines;
    size_t length;
    size_t capacity;

    Vec2D cursor;
} Buffer;

void buffer_free(Buffer *buffer);
void buffer_read(Buffer *buffer);
void buffer_save(Buffer *buffer);

void buffer_set_line(Buffer *buffer, size_t index, String line);
void buffer_insert_line(Buffer *buffer, size_t index, String line);
void buffer_delete_line(Buffer *buffer, size_t index);

void buffer_insert_char(Buffer *buffer, char ch);

void buffer_next_char(Buffer *buffer);
void buffer_prev_char(Buffer *buffer);

void buffer_next_word(Buffer *buffer);
void buffer_prev_word(Buffer *buffer);

void buffer_next_line(Buffer *buffer);
void buffer_prev_line(Buffer *buffer);
void buffer_head_line(Buffer *buffer);
void buffer_tail_line(Buffer *buffer);

void buffer_next_para(Buffer *buffer);
void buffer_prev_para(Buffer *buffer);

void buffer_head_file(Buffer *buffer);
void buffer_tail_file(Buffer *buffer);

Vec2D buffer_next_term(Buffer *buffer, String term, Vec2D origin);
Vec2D buffer_prev_term(Buffer *buffer, String term, Vec2D origin);
Vec2D buffer_search_term(Buffer *buffer, String term, Vec2D origin, bool forward);

void buffer_delete_motion(Buffer *buffer, void (*motion)(Buffer *));

#endif // BUFFER_H
