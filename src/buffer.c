#include "buffer.h"

/*
 * Free all allocated memory in a buffer
 * @param buffer *Buffer The buffer to free
 */
void buffer_free(Buffer *buffer)
{
    if (buffer->lines) {
        for (size_t i = 0; i < buffer->lines_count; ++i)
            string_free(&buffer->lines[i]);

        free(buffer->lines);
    }

    buffer->lines_count = buffer->lines_capacity = 0;
    buffer->row = buffer->col = 0;
}

/*
 * Insert a line at a particular index in a buffer
 * @param buffer *Buffer The buffer to insert the line in
 * @param index size_t The index to insert the line in
 * @param line String The line to insert
 */
void buffer_insert(Buffer *buffer, size_t index, String line)
{
    // Increase the capacity of the buffer if required
    if (buffer->lines_count >= buffer->lines_capacity) {
        buffer->lines_capacity = GROW_CAPACITY(buffer->lines_capacity);
        buffer->lines = GROW_ARRAY(buffer->lines, String, buffer->lines_capacity);
    }

    // Make space for the line in the buffer if required
    if (index != buffer->lines_count)
        SHIFT_ARRAY(buffer->lines, index, 1, String, buffer->lines_count);

    buffer->lines_count++;
    buffer->lines[index] = line;
}

/*
 * Insert a line at the end of a buffer
 * @param buffer *Buffer The buffer to insert the line in
 * @param line String The line to insert
 */
void buffer_append(Buffer *buffer, String line)
{
    buffer_insert(buffer, buffer->lines_count, line);
}

/*
 * Delete lines from a index in a buffer
 * @param buffer *Buffer The buffer to to delete the lines in
 * @param index size_t The index to delete the lines from
 * @param count size_t The number of lines to delete
 */
void buffer_delete(Buffer *buffer, size_t index, size_t count)
{
    if (index + count < buffer->lines_count)
        BSHIFT_ARRAY(buffer->lines, index, count, String, buffer->lines_count);

    buffer->lines_count -= count;
}

/*
 * Read a file into a buffer
 * @param buffer *Buffer The buffer to read a file into
 * @param path *char The path of the file to read
 * @param indent *char The indentation string
 */
void buffer_read(Buffer *buffer, const char *path, const char *indent)
{
    FILE *file = fopen(path, "r");
    ASSERT(file, "could not read file '%s'", path);

    char *input = NULL;
    size_t n = 0;
    size_t tabsize = strlen(indent);
    ssize_t length;

    while ((length = getline(&input, &n, file)) != -1) {
        length -= 1;

        String line = {0};

        size_t offset = 0;
        size_t count = 0;
        char *tab = NULL;

        while (offset < (size_t) length) {
            tab = memchr(input + offset, '\t', length);

            if (tab) {
                count = (size_t) (tab - input) - offset;
                string_append(&line, input + offset, count);
                string_append(&line, indent, tabsize);
                offset += count + 1;
            } else {
                count = (size_t) length - offset;
                string_append(&line, input + offset, count);
                offset += count;
            }
        }

        if (length == 0) string_append(&line, "", 0);

        buffer_append(buffer, line);
    }

    fclose(file);
    buffer->file = path;
    if (input) free(input);
}

/*
 * Write a buffer to its associated file
 * @param buffer *Buffer The buffer to write
 */
void buffer_write(Buffer *buffer)
{
    FILE *file = fopen(buffer->file, "w");
    ASSERT(file, "could not write file '%s'", buffer->file);

    for (size_t i = 0; i < buffer->lines_count; ++i)
        fprintf(file, StringFmt "\n", StringArg(buffer->lines[i]));

    fclose(file);
}

/*
 * Insert a character at the cursor position in the buffer
 * @param buffer *Buffer The buffer to insert the character in
 * @param ch char The character to insert
 */
void buffer_insert_char(Buffer *buffer, char ch)
{
    if (ch == '\n') {
        String next = string_split(&buffer->lines[buffer->row++], buffer->col);
        buffer_insert(buffer, buffer->row, next);
        buffer->col = 0;
    } else {
        string_insert(&buffer->lines[buffer->row], buffer->col++, &ch, 1);
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
    if (buffer->row < buffer->lines_count) {
        buffer->row++;
        buffer->col = (buffer->row == buffer->lines_count)
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
        buffer->col--;
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
        buffer->col++;
    } else if (buffer->row < buffer->lines_count) {
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
    buffer->row = buffer->lines_count;
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
 * Search for a pattern in a buffer
 * @param buffer *Buffer The buffer to search the term in
 * @param pattern String The string to search in the buffer
 * @param forward bool Whether the direction is performed forward
 */
void buffer_search(Buffer *buffer, String pattern, bool forward)
{
    bool searched = false;
    size_t row = buffer->row;

    if (forward) {
        int col = buffer->col + 1;
        while (!searched || row != buffer->row || col > (int) buffer->row) {
            searched = true;

            if ((col = string_search(buffer->lines[row], pattern, col, forward)) != -1) {
                buffer->row = row;
                buffer->col = col;
                break;
            }

            row = (row < buffer->lines_count) ? row + 1 : 0;
            col = 0;
        }
    } else {
        int col = buffer->col - 1;
        while (!searched || row != buffer->row || col < (int) buffer->row) {
            searched = true;

            if ((col = string_search(buffer->lines[row], pattern, col, forward)) != -1) {
                buffer->row = row;
                buffer->col = col;
                break;
            }

            row = (row > 0) ? row - 1 : buffer->lines_count;
            col = buffer->lines[row].length;
        }
    }
}

/*
 * Delete the character left of the cursor in a buffer
 * @param buffer *Buffer The buffer to delete in
 */
void buffer_delete_left(Buffer *buffer)
{
    if (buffer->col > 0) {
        string_delete(&buffer->lines[buffer->row], --buffer->col, 1);
    } else if (buffer->row > 0) {
        String current = buffer->lines[buffer->row];
        buffer->col = buffer->lines[buffer->row - 1].length;

        string_append(&buffer->lines[buffer->row - 1], current.chars, current.length);
        string_free(&current);
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
        string_delete(&buffer->lines[buffer->row], buffer->col, 1);
    } else if (buffer->lines_count > buffer->row) {
        String next = buffer->lines[buffer->row + 1];
        string_append(&buffer->lines[buffer->row], next.chars, next.length);
        string_free(&next);
        buffer_delete(buffer, buffer->row + 1, 1);
    }
}
