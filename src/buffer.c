#include "buffer.h"

/*
 * Free all allocated memory in a buffer
 * @param buffer *Buffer The buffer to free
 */
void buffer_free(Buffer *buffer)
{
    if (buffer->lines) {
        for (size_t i = 0; i < buffer->length; ++i)
            line_free(&buffer->lines[i]);

        free(buffer->lines);
    }

    buffer->length = buffer->capacity = buffer->row = buffer->col = 0;
}

/*
 * Insert a line at a particular index in a buffer
 * @param buffer *Buffer The buffer to insert the line in
 * @param index size_t The index to insert the line in
 * @param line Line The line to insert
 */
void buffer_insert(Buffer *buffer, size_t index, Line line)
{
    // Increase the capacity of the buffer if required
    if (buffer->length >= buffer->capacity) {
        buffer->capacity = GROW_CAPACITY(buffer->capacity);
        buffer->lines = GROW_ARRAY(buffer->lines, Line, buffer->capacity);
    }

    // Make space for the line in the buffer if required
    if (index != buffer->length)
        SHIFT_ARRAY(buffer->lines, index, 1, Line, buffer->length);

    buffer->length++;
    buffer->lines[index] = line;
}

/*
 * Insert a line at the end of a buffer
 * @param buffer *Buffer The buffer to insert the line in
 * @param line Line The line to insert
 */
void buffer_append(Buffer *buffer, Line line)
{
    buffer_insert(buffer, buffer->length, line);
}

/*
 * Read a file into a buffer
 * @param buffer *Buffer The buffer to read a file into
 * @param path *char The path of the file to read
 */
void buffer_read(Buffer *buffer, const char *path)
{
    FILE *fp = fopen(path, "r");
    ASSERT(fp, "could not read file '%s'", path);

    char *input = NULL;
    size_t n = 0;
    ssize_t length;

    while ((length = getline(&input, &n, fp)) != -1) {
        Line line = {0};
        line_append(&line, input, length - 1);
        buffer_append(buffer, line);
    }

    fclose(fp);
    if (input) free(input);
}

/*
 * Insert a character at the cursor position in the buffer
 * @param buffer *Buffer The buffer to insert the character in
 * @param ch char The character to insert
 */
void buffer_insert_char(Buffer *buffer, char ch)
{
    ASSERT(buffer->row < buffer->length, "cursor row is out of the buffer");

    if (ch == '\n') {
        Line next = line_split(&buffer->lines[buffer->row], buffer->col);
        buffer_insert(buffer, buffer->row++, next);
        buffer->col = 0;
    } else {
        line_insert(&buffer->lines[buffer->row], buffer->col++, &ch, 1);
    }
}

/*
 * Move the cursor down one line in the buffer
 * @param buffer *Buffer The buffer to move the cursor down in
 */
void buffer_cursor_down(Buffer *buffer)
{
    if (buffer->row < buffer->length) {
        buffer->col = MIN(buffer->lines[++buffer->row].length,
                          buffer->col);
    }
}

/*
 * Move the cursor up one line in the buffer
 * @param buffer *Buffer The buffer to move the cursor up in
 */
void buffer_cursor_up(Buffer *buffer)
{
    if (buffer->row > 0) {
        buffer->col = MIN(buffer->lines[--buffer->row].length,
                          buffer->col);
    }
}

/*
 * Move the cursor one character backward in the buffer
 * @param buffer *Buffer The buffer to move the cursor backward in
 */
void buffer_cursor_left(Buffer *buffer)
{
    if (buffer->col > 0) {
        buffer->col -= 1;
    } else if (buffer->row > 0) {
        buffer->col = buffer->lines[--buffer->row].length;
    }
}

/*
 * Move the cursor one character forward in the buffer
 * @param buffer *Buffer The buffer to move the cursor forward in
 */
void buffer_cursor_right(Buffer *buffer)
{
    if (buffer->col < buffer->lines[buffer->row].length) {
        buffer->col += 1;
    } else if (buffer->row < buffer->length) {
        buffer->row += 1;
        buffer->col = 0;
    }
}
