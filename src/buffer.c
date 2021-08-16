#include "buffer.h"

// Reset a buffer
void buffer_free(Buffer *buffer)
{
    if (buffer->lines) {
        for (size_t i = 0; i < buffer->length; ++i)
            line_free(&buffer->lines[i]);

        free(buffer->lines);
    }

    buffer->length = buffer->capacity = 0;
}

// Insert a line at a particular index in a buffer
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

// Insert a line at the end of a buffer
void buffer_append(Buffer *buffer, Line line)
{
    buffer_insert(buffer, buffer->length, line);
}

// Read a file into a buffer
void buffer_read(Buffer *buffer, const char *file_path)
{
    FILE *fp = fopen(file_path, "r");
    ASSERT(fp, "could not read file '%s'", file_path);

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
