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

    buffer->length = buffer->capacity = 0;
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
 * Read a file into a buffer
 * @param buffer *Buffer The buffer to read a file into
 * @param file_path *char The path of the file to read
 */
void buffer_read(Buffer *buffer, const char *file_path)
{
    FILE *fp = fopen(file_path, "r");
    ASSERT(fp, "could not read file '%s'", file_path);

    char *input = NULL;
    size_t n = 0;
    ssize_t length;

    while ((length = getline(&input, &n, fp)) != -1) {
        Line line = {0};
        line_insert(&line, line.length, input, length - 1);
        buffer_insert(buffer, buffer->length, line);
    }

    fclose(fp);
    if (input) free(input);
}
