#include "line.h"

// Reset a line
void line_free(Line *line)
{
    if (line->text)
        free(line->text);

    line->length = line->capacity = 0;
}

// Insert a string at a particular index in a line
void line_insert(Line *line, size_t index, char *string, size_t length)
{
    // Increase the capacity of the line if required
    if (line->length + length >= line->capacity) {
        line->capacity = GROW_CAPACITY(line->capacity);

        if (line->capacity < length)
            line->capacity = length;

        line->text = GROW_ARRAY(line->text, char, line->capacity);
    }

    // Make space for the string in the line if required
    if (index != line->length)
        SHIFT_ARRAY(line->text, index, length, char, line->length);

    line->length += length;
    memcpy(line->text + index, string, length);
}

// Insert a string at the end of a line
void line_append(Line *line, char *string, size_t length)
{
    line_insert(line, line->length, string, length);
}
