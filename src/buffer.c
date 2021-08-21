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
 * Delete lines from a index in a buffer
 * @param buffer *Buffer The buffer to to delete the lines in
 * @param index size_t The index to delete the lines from
 * @param count size_t The number of lines to delete
 */
void buffer_delete(Buffer *buffer, size_t index, size_t count)
{
    if (index + count < buffer->length)
        BSHIFT_ARRAY(buffer->lines, index, count, Line, buffer->length);

    buffer->length -= count;
}

/*
 * Read a file into a buffer
 * @param buffer *Buffer The buffer to read a file into
 * @param path *char The path of the file to read
 * @param indent *char The indentation string
 */
void buffer_read(Buffer *buffer, const char *path, const char *indent)
{
    FILE *fp = fopen(path, "r");
    ASSERT(fp, "could not read file '%s'", path);

    char *input = NULL;
    size_t n = 0;
    size_t tabsize = strlen(indent);
    ssize_t length;

    while ((length = getline(&input, &n, fp)) != -1) {
        length -= 1;

        Line line = {0};

        size_t offset = 0;
        size_t count = 0;
        char *tab = NULL;

        while (offset < (size_t) length) {
            tab = memchr(input + offset, '\t', length);

            if (tab) {
                count = (size_t) (tab - input) - offset;
                line_append(&line, input + offset, count);
                line_append(&line, indent, tabsize);
                offset += count + 1;
            } else {
                count = (size_t) length - offset;
                line_append(&line, input + offset, count);
                offset += count;
            }
        }

        if (length == 0) line_append(&line, "", 0);

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
    if (ch == '\n') {
        Line next = line_split(&buffer->lines[buffer->row++], buffer->col);
        buffer_insert(buffer, buffer->row, next);
        buffer->col = 0;
    } else {
        line_insert(&buffer->lines[buffer->row], buffer->col++, &ch, 1);
    }
}

/*
 * Move the cursor up one line in the buffer
 * @param buffer *Buffer The buffer to move the cursor in
 */
void buffer_cursor_up(Buffer *buffer)
{
    if (buffer->row > 0)
        buffer->col = min(buffer->lines[--buffer->row].length, buffer->col);
}

/*
 * Move the cursor down one line in the buffer
 * @param buffer *Buffer The buffer to move the cursor in
 */
void buffer_cursor_down(Buffer *buffer)
{
    if (buffer->row < buffer->length) {
        buffer->row += 1;
        buffer->col = (buffer->row == buffer->length)
            ? 0
            : min(buffer->lines[buffer->row].length, buffer->col);
    }
}

/*
 * Move the cursor one character backward in the buffer
 * @param buffer *Buffer The buffer to move the cursor in
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
 * @param buffer *Buffer The buffer to move the cursor in
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

/*
 * Move the cursor to the top of the buffer
 * @param buffer *Buffer The buffer to move the cursor in
 */
void buffer_cursor_top(Buffer *buffer)
{
    buffer->row = buffer->col = 0;
}

/*
 * Move the cursor to the bottom of the buffer
 * @param buffer *Buffer The buffer to move the cursor in
 */
void buffer_cursor_bottom(Buffer *buffer)
{
    buffer->row = buffer->length;
    buffer->col = buffer->lines[buffer->row].length;
}

/*
 * Move the cursor to the start of the line in the buffer
 * @param buffer *Buffer The buffer to move the cursor in
 */
void buffer_cursor_start(Buffer *buffer)
{
    buffer->col = 0;
}

/*
 * Move the cursor to the end of the line in the buffer
 * @param buffer *Buffer The buffer to move the cursor in
 */
void buffer_cursor_end(Buffer *buffer)
{
    buffer->col = buffer->lines[buffer->row].length;
}

/*
 * Delete the character left of the cursor in a buffer
 * @param buffer *Buffer The buffer to delete in
 */
void buffer_delete_left(Buffer *buffer)
{
    if (buffer->col > 0) {
        line_delete(&buffer->lines[buffer->row], --buffer->col, 1);
    } else if (buffer->row > 0) {
        Line current = buffer->lines[buffer->row];
        buffer->col = buffer->lines[buffer->row - 1].length;

        line_append(&buffer->lines[buffer->row - 1], current.text, current.length);
        line_free(&current);
        buffer_delete(buffer, buffer->row--, 1);
    }
}

/*
 * Delete the character right of the cursor in a buffer
 * @param buffer *Buffer The buffer to delete in
 */
void buffer_delete_right(Buffer *buffer)
{
    if (buffer->col < buffer->lines[buffer->row].length) {
        line_delete(&buffer->lines[buffer->row], buffer->col, 1);
    } else if (buffer->length > buffer->row) {
        Line next = buffer->lines[buffer->row + 1];
        line_append(&buffer->lines[buffer->row], next.text, next.length);
        line_free(&next);
        buffer_delete(buffer, buffer->row + 1, 1);
    }
}
