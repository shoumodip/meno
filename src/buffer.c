#include "buffer.h"

static bool buffer_search_forward(Buffer *buffer, String term, Vec2D *cursor, size_t limit);
static bool buffer_search_backward(Buffer *buffer, String term, Vec2D *cursor, size_t limit);

/*
 * Search for a term forward in a buffer
 * @param buffer *Buffer The buffer to search in
 * @param term String The string to search
 * @param cursor *Vec2D The cursor to move
 * @param limit size_t The limit to stop searching before
 * @return bool Whether the searching was successful
 */
bool buffer_search_forward(Buffer *buffer, String term, Vec2D *cursor, size_t limit)
{
    int position;
    while (cursor->y < limit) {
        position = string_next_term(buffer->lines[cursor->y],
                                        term, cursor->x);

        if (position == -1) {
            cursor->y++;
            cursor->x = 0;
        } else {
            cursor->x = position;
            return true;
        }
    }

    return false;
}

/*
 * Search for a term backward in a buffer
 * @param buffer *Buffer The buffer to search in
 * @param term String The string to search
 * @param cursor *Vec2D The cursor to move
 * @param limit size_t The limit to stop searching before
 * @return bool Whether the searching was successful
 */
bool buffer_search_backward(Buffer *buffer, String term, Vec2D *cursor, size_t limit)
{
    int position;
    while (cursor->y >= limit) {
        position = string_prev_term(buffer->lines[cursor->y],
                                        term, cursor->x);

        if (position == -1) {
            if (cursor->y == 0) return false;
            cursor->y--;
            cursor->x = buffer->lines[cursor->y].length;
        } else {
            cursor->x = position;
            return true;
        }
    }

    return false;
}

/*
 * Free all allocated memory in a buffer
 * @param buffer *Buffer The buffer to free
 */
void buffer_free(Buffer *buffer)
{
    if (buffer->lines) {
        for (size_t i = 0; i < buffer->length; ++i)
            string_free(&buffer->lines[i]);

        free(buffer->lines);
    }

    buffer->length = buffer->capacity = 0;
    buffer->cursor.x = buffer->cursor.y = 0;
}

/*
 * Read the buffer from its associated file
 * @param buffer *Buffer The buffer to read
 */
void buffer_read(Buffer *buffer)
{
    if (buffer->file.length == 0) goto fill_buffer;
    FILE *file = fopen(buffer->file.chars, "r");
    if (!file) goto fill_buffer;

    char *input = NULL;
    size_t n = 0;
    ssize_t length;

    while ((length = getline(&input, &n, file)) != -1) {
        if (input[length - 1] == '\n') length -= 1;

        String line = {0};
        string_insert(&line, 0, input, length);
        buffer_set_line(buffer, buffer->length, line);
    }

    fclose(file);
    if (input) free(input);

    // The buffer has to have atleast one line in it otherwise it may result in
    // illegal memory access, aka segfaults
 fill_buffer:
    if (buffer->length == 0) buffer_insert_line(buffer, 0, (String) {0});
}

/*
 * Save the buffer to its associated file
 * @param buffer *Buffer The buffer to save
 */
void buffer_save(Buffer *buffer)
{
    if (buffer->file.length == 0) return;
    FILE *file = fopen(buffer->file.chars, "w");
    if (!file) return;

    for (size_t i = 0; i < buffer->length; ++i)
        fprintf(file, StringFmt "\n", StringArg(buffer->lines[i]));

    fclose(file);
}

/*
 * Overwrite a line in a buffer, or insert it
 * @param buffer *Buffer The buffer to set the line in
 * @param index size_t The index to set the line in
 * @param line Line The line to place in the buffer
 */
void buffer_set_line(Buffer *buffer, size_t index, String line)
{
    if (buffer->length > index) {
        // A line already exists in the given index, just replace it
        String *target = &buffer->lines[index];
        string_replace(target, 0, line.chars, line.length);
        target->length = line.length;
    } else {
        // Insert the line into the buffer
        buffer_insert_line(buffer, index, line);
    }
}

/*
 * Insert a line at a particular index in a buffer
 * @param buffer *Buffer The buffer to insert the line in
 * @param index size_t The index to insert the line in
 * @param line String The line to insert
 */
void buffer_insert_line(Buffer *buffer, size_t index, String line)
{
    // Grow the buffer if required
    if (buffer->length >= buffer->capacity) {
        buffer->capacity = GROW_CAPACITY(buffer->capacity);
        buffer->lines = GROW_ARRAY(buffer->lines, String, buffer->capacity);
    }

    // Shift the lines in the buffer to make space for the new line
    if (buffer->cursor.y != buffer->length)
        SHIFT_ARRAY(buffer->lines, index, 1, String, buffer->length);

    // Insert the line
    buffer->length++;
    buffer->lines[index] = line;
}

/*
 * Delete a line at an index in a buffer
 * @param buffer *Buffer The buffer to to delete the lines in
 * @param index size_t The index of the line to delete
 */
void buffer_delete_line(Buffer *buffer, size_t index)
{
    // Shift back the elements in the buffer
    if (buffer->cursor.y < buffer->length)
        BSHIFT_ARRAY(buffer->lines, index, 1, String, buffer->length--);
}

/*
 * Insert a character at the cursor in a buffer
 * @param buffer *Buffer The buffer to insert the character in
 * @param ch char The character to insert
 */
void buffer_insert_char(Buffer *buffer, char ch)
{
    if (ch == '\n') {
        // Split the current line into two parts and insert the part after the
        // cursor as a new line into the buffer
        String after = string_split(&buffer->lines[buffer->cursor.y],
                                    buffer->cursor.x);

        buffer_insert_line(buffer, ++buffer->cursor.y, after);
        buffer->cursor.x = 0;
    } else {
        // Insert the character into the buffer if it is printable
        string_insert(&buffer->lines[buffer->cursor.y], buffer->cursor.x++, &ch, 1);
    }
}

/*
 * Motion for moving to the next character
 * @param buffer *Buffer The buffer to do the motion in
 */
void buffer_next_char(Buffer *buffer)
{
    if (buffer->cursor.x < buffer->lines[buffer->cursor.y].length) {
        buffer->cursor.x++;
    } else if (buffer->cursor.y + 1 < buffer->length) {
        buffer->cursor.y++;
        buffer->cursor.x = 0;
    }
}

/*
 * Motion for moving to the previous character
 * @param buffer *Buffer The buffer to do the motion in
 */
void buffer_prev_char(Buffer *buffer)
{
    if (buffer->cursor.x > 0) {
        buffer->cursor.x--;
    } else if (buffer->cursor.y > 0) {
        buffer->cursor.x = buffer->lines[--buffer->cursor.y].length;
    }
}

/*
 * Motion for moving to the start of the next word
 * @param buffer *Buffer The buffer to do the motion in
 */
void buffer_next_word(Buffer *buffer)
{
    String current = buffer->lines[buffer->cursor.y];

    if (buffer->cursor.x == current.length &&
        buffer->cursor.y + 1 < buffer->length) {
        buffer->cursor.y++;
        buffer->cursor.x = 0;
    } else {
        buffer->cursor.x = string_next_word(current, buffer->cursor.x);
    }
}

/*
 * Motion for moving to the end of the previous word
 * @param buffer *Buffer The buffer to do the motion in
 */
void buffer_prev_word(Buffer *buffer)
{
    buffer->cursor.x = (buffer->cursor.x == 0 && buffer->cursor.y > 0)
        ? buffer->lines[--buffer->cursor.y].length
        : string_prev_word(buffer->lines[buffer->cursor.y], buffer->cursor.x);
}

/*
 * Motion for moving to the next line
 * @param buffer *Buffer The buffer to do the motion in
 */
void buffer_next_line(Buffer *buffer)
{
    if (buffer->cursor.y + 1 < buffer->length)
        buffer->cursor.y++;

    buffer->cursor.x = min(buffer->cursor.x,
                           buffer->lines[buffer->cursor.y].length);
}

/*
 * Motion for moving to the previous line
 * @param buffer *Buffer The buffer to do the motion in
 */
void buffer_prev_line(Buffer *buffer)
{
    if (buffer->cursor.y > 0)
        buffer->cursor.y--;

    buffer->cursor.x = min(buffer->cursor.x,
                           buffer->lines[buffer->cursor.y].length);
}

/*
 * Motion for moving to the start of the current line
 * @param buffer *Buffer The buffer to do the motion in
 */
void buffer_head_line(Buffer *buffer)
{
    buffer->cursor.x = 0;
}

/*
 * Motion for moving to the end of the current line
 * @param buffer *Buffer The buffer to do the motion in
 */
void buffer_tail_line(Buffer *buffer)
{
    buffer->cursor.x = buffer->lines[buffer->cursor.y].length;
}

/*
 * Motion for moving to the start of the next paragraph
 * @param buffer *Buffer The buffer to do the motion in
 */
void buffer_next_para(Buffer *buffer)
{
    size_t y = buffer->cursor.y;

    // Go to the end of the current paragraph
    while (buffer->lines[y].length != 0 && y + 1 < buffer->length) y++;

    // Go to the start of the next paragraph
    while (buffer->lines[y].length == 0 && y + 1 < buffer->length) y++;

    buffer->cursor.y = y;
    buffer->cursor.x = 0;
}

/*
 * Motion for moving to the start of the previous paragraph
 * @param buffer *Buffer The buffer to do the motion in
 */
void buffer_prev_para(Buffer *buffer)
{
    size_t y = buffer->cursor.y;

    // Go to the start of the current paragraph
    while (buffer->lines[y].length != 0 && y > 0) y--;

    // Go to the end of the previous paragraph
    while (buffer->lines[y].length == 0 && y > 0) y--;

    buffer->cursor.y = y;
    buffer->cursor.x = 0;
}

/*
 * Motion for moving to the start of the file
 * @param buffer *Buffer The buffer to do the motion in
 */
void buffer_head_file(Buffer *buffer)
{
    buffer->cursor.y = 0;
    buffer->cursor.x = 0;
}

/*
 * Motion for moving to the end of the file
 * @param buffer *Buffer The buffer to do the motion in
 */
void buffer_tail_file(Buffer *buffer)
{
    buffer->cursor.y = buffer->length - 1;
    buffer->cursor.x = buffer->lines[buffer->cursor.y].length;
}

/*
 * Motion for moving to the next occurence of a term
 * @param buffer *Buffer The buffer to do the motion in
 * @param term String The term to search
 * @return Vec2D The location of the previous term
 */
Vec2D buffer_next_term(Buffer *buffer, String term, Vec2D origin)
{
    Vec2D cursor = origin;
    cursor.x++;

    if (buffer_search_forward(buffer, term, &cursor, buffer->length))
        return cursor;

    cursor.y = 0;
    cursor.x = 0;

    if (buffer_search_forward(buffer, term, &cursor, origin.y))
        return cursor;

    return origin;
}

/*
 * Motion for moving to the previous occurence of a term
 * @param buffer *Buffer The buffer to do the motion in
 * @param term String The term to search
 * @return Vec2D The location of the previous term
 */
Vec2D buffer_prev_term(Buffer *buffer, String term, Vec2D origin)
{
    Vec2D cursor = origin;
    if (cursor.x > 0) cursor.x--;

    if (buffer_search_backward(buffer, term, &cursor, 0))
        return cursor;

    cursor.y = buffer->length - 1;
    cursor.x = buffer->lines[cursor.y].length;

    if (buffer_search_backward(buffer, term, &cursor, origin.y))
        return cursor;

    return origin;
}

/*
 * Search for a term forward or backwards
 * @param buffer *Buffer The buffer to search the term in
 * @param term String The term to search
 * @param origin Vec2D The origin to start searching from
 * @param bool forward Whether the searching should be done forward
 * @return Vec2D The location of the previous term
 */
Vec2D buffer_search_term(Buffer *buffer, String term, Vec2D origin, bool forward)
{
    if (term.length) {
        return forward
            ? buffer_next_term(buffer, term, origin)
            : buffer_prev_term(buffer, term, origin);
    } else {
        return origin;
    }
}

/*
 * Execute a motion and delete the characters encompassed by the motion
 * @param buffer *Buffer The buffer to do the motion in
 * @param motion void (*)(Buffer *) The motion to execute
 */
void buffer_delete_motion(Buffer *buffer, void (*motion)(Buffer *))
{
    Vec2D head = buffer->cursor;
    motion(buffer);
    Vec2D tail = buffer->cursor;

    // The head has to in a buffer position before the tail
    // position. There are two situations that can arise.
    //
    // ----------------------------------------------------
    // P1: The tail is above the head (irrespective of dx)
    //
    //   T
    //
    //          H
    //
    // ----------------------------------------------------
    // P2: The tail is before the head in the same line
    //
    //   T      H
    //
    if (tail.y < head.y || (tail.y == head.y && tail.x < head.x)) {
        Vec2D temp = head;
        head = tail;
        tail = temp;
    }

    String *head_line = &buffer->lines[head.y];
    String *tail_line = &buffer->lines[tail.y];

    // Replace the characters after the head with the
    // characters after the tail. This will create an effect
    // of deleting the characters in between.
    //
    // ----------------------------------------------------
    // P1: Deleting the same line
    // 
    //  +++++ H ===== T *****
    //  ............
    // 
    // 
    //  +++++ HT *****
    //  ............
    // 
    //
    // ----------------------------------------------------
    // P2: Deleting across multiple lines
    //
    //  +++++ H =====
    //  ===========
    //  ================
    //  === T *******
    //  ............
    // 
    //
    //  +++++ HT *******
    //  ===========
    //  ================
    //  ............
    // 
    string_replace(head_line, head.x,
                   tail_line->chars + tail.x,
                   tail_line->length - tail.x);
    head_line->length = head.x + tail_line->length - tail.x;

    // Delete the lines which have to be removed completely.
    //
    // ----------------------------------------------------
    // P1: The tail line is just below the head line.
    //
    //  +++++ H =====
    //  === T *******
    //  ..........
    // 
    //
    // In the previous conjoining step, the buffer assumes a
    // form like this.
    //
    //  +++++ HT *******
    //  ===
    //  ..........
    //
    //
    // After this step the extra tail line is removed
    //
    //  +++++ HT *******
    //  ..........
    // 
    // 
    // ----------------------------------------------------
    // P2: The tail line is way below the head line.
    //
    //  +++++ H =====
    //  ===========
    //  ================
    //  === T *******
    //  ..........
    // 
    //
    // In the previous conjoining step, the buffer assumes a
    // form like this.
    //
    //  +++++ HT *******
    //  ===========
    //  ================
    //  ===
    //  ..........
    //
    //
    // After this step the extra tail line is removed
    //
    //  +++++ HT *******
    //  ..........
    // 
    if (tail.y > head.y) {
        for (size_t y = head.y + 1; y <= tail.y; ++y)
            string_free(&buffer->lines[y]);

        memmove(buffer->lines + head.y + 1,
                buffer->lines + tail.y + 1,
                sizeof(String) * (buffer->length - tail.y));

        buffer->length -= tail.y - head.y;
    }

    buffer->cursor = head;
}
