#include "line.h"

/*
 * Free all allocated memory in a line
 * @param line *Line The line to free
 */
void line_free(Line *line)
{
    if (line->text)
        free(line->text);

    line->length = line->capacity = 0;
}

/*
 * Insert a string at a particular index in a line
 * @param line *Line The line to insert the string in
 * @param index size_t The index to insert the string in
 * @param string *char The string to insert
 * @param count size_t The number of bytes in the string to insert
 */
void line_insert(Line *line, size_t index, char *string, size_t count)
{
    // Increase the capacity of the line if required
    if (line->length + count >= line->capacity) {
        line->capacity = GROW_CAPACITY(line->capacity);

        if (line->capacity < count)
            line->capacity = count;

        line->text = GROW_ARRAY(line->text, char, line->capacity);
    }

    // Make space for the string in the line if required
    if (index != line->length)
        SHIFT_ARRAY(line->text, index, count, char, line->length);

    line->length += count;
    memcpy(line->text + index, string, count);
}

/*
 * Insert a string at the end of a line
 * @param line *Line The line to insert the string in
 * @param string *char The string to insert
 * @param count size_t The number of bytes in the string to insert
 */
void line_append(Line *line, char *string, size_t count)
{
    line_insert(line, line->length, string, count);
}

/*
 * Delete characters from a point in a line
 * @param line *Line The line to delete the characters in
 * @param index size_t The index to delete the characters from
 * @param count size_t The number of characters to delete
 */
void line_delete(Line *line, size_t index, size_t count)
{
    if (index + count < line->length) {
        memmove(line->text + index,
                line->text + index + count,
                line->length - count);
    }

    line->length -= count;
}

/*
 * Split a line at a particular index
 * @param line *Line The line to split
 * @param index size_t The index to split the line at
 * @return *Line The left hand side of the split line
 */
Line line_split(Line *line, size_t index)
{
    Line new = {0};

    if (index < line->length) {
        line_append(&new, line->text + index, line->length - index);
        line->length = index;
    }

    return new;
}
