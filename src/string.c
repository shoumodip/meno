#include "string.h"

/*
 * Free all allocated memory in a string
 * @param string *String The string to free
 */
void string_free(String *string)
{
    if (string->chars) {
        free(string->chars);
        string->chars = NULL;
    }

    string->length = string->capacity = 0;
}

/*
 * Grow a string to a particular size
 * @param string *String The string to grow
 * @param size size_t The size to grow the string to
 */
void string_grow(String *string, size_t size)
{
    if (size > string->capacity) {
        string->capacity = max(GROW_CAPACITY(string->capacity), size);
        string->chars = GROW_ARRAY(string->chars, char, string->capacity);
    }
}

/*
 * Delete characters from a point in a string
 * @param string *String The string to delete the characters in
 * @param start size_t The index to delete the characters from
 * @param count size_t The number of characters to delete
 */
void string_delete(String *string, size_t start, size_t count)
{
    if (start + count < string->length)
        BSHIFT_ARRAY(string->chars, start, count, char, string->length);

    string->length -= count;
}

/*
 * Insert a C-string at a particular index in a string
 * @param string *String The string to insert the C-string in
 * @param index size_t The index to insert the C-string in
 * @param string *char The C-string to insert
 * @param count size_t The number of bytes in the C-string to insert
 */
void string_insert(String *string, size_t index, const char *source, size_t count)
{
    string_grow(string, string->length + count);

    if (index != string->length)
        SHIFT_ARRAY(string->chars, index, count, char, string->length);

    string->length += count;
    memcpy(string->chars + index, source, count);
}

/*
 * Replace a part of a string with a C-string
 * @param string *String The string to replace in
 * @param start size_t The index to start replacing from
 * @param source *char The C-string to replace the subpart with
 * @param count size_t The number of characters to replace
 */
void string_replace(String *string, size_t start, const char *source, size_t count)
{
    string_grow(string, start + count);
    memcpy(string->chars + start, source, count);
    string->length = max(string->length, start + count);
}

/*
 * Split a string at a particular index
 * @param string *String The string to split
 * @param index size_t The index to split the string at
 * @return String The left hand side of the split string
 */
String string_split(String *string, size_t index)
{
    String new = {0};

    if (index < string->length) {
        string_insert(&new, 0, string->chars + index, string->length - index);
        string->length = index;
    }

    return new;
}

/*
 * Check if two strings are equal
 * @param a String The first string
 * @param b String The second string
 * @return bool Whether the strings are equal
 */
bool string_equal(String a, String b)
{
    return a.length == b.length && memcmp(a.chars, b.chars, a.length) == 0;
}

/*
 * Find the start of the next word in a string
 * @param string String The string to search the next word in
 * @param start size_t The index to start searching from
 * @return size_t The index of the start of the next word
 */
size_t string_next_word(String string, size_t start)
{
    size_t i = start;

    if (i < string.length && string.chars[i] == ' ') i++;
    while (i < string.length && string.chars[i] != ' ') i++;
    while (i < string.length && string.chars[i] == ' ') i++;

    return i;
}

/*
 * Find the end of the previous word in a string
 * @param string String The string to search the previous word in
 * @param start size_t The index to start searching from
 * @return size_t The index of the start of the previous word
 */
size_t string_prev_word(String string, size_t start)
{
    size_t i = start;

    if (i > 0 && string.chars[i] == ' ') i--;
    while (i > 0 && string.chars[i] != ' ') i--;
    while (i > 0 && string.chars[i - 1] == ' ') i--;

    return i;
}

/*
 * Find the next occurence of a term in a string
 * @param string String The string to search the term in
 * @param term String The term to search in the string
 * @param start size_t The index to start searching from
 * @return int The index of the term or -1 if not found
 */
int string_next_term(String string, String term, size_t start)
{
    if (string.length >= term.length) {
        for (size_t i = start; i < string.length; ++i) {
            if (strncasecmp(string.chars + i, term.chars, term.length) == 0) {
                return i;
            }
        }
    }

    return -1;
}

/*
 * Find the previous occurence of a term in a string
 * @param string String The string to search the term in
 * @param term String The term to search in the string
 * @param start size_t The index to start searching from
 * @return int The index of the term or -1 if not found
 */
int string_prev_term(String string, String term, size_t start)
{
    if (string.length >= term.length) {
        for (int i = start; i >= 0; --i) {
            if (strncasecmp(string.chars + i, term.chars, term.length) == 0) {
                return i;
            }
        }
    }

    return -1;
}

/*
 * Delete the characters inside a range in a string
 * @param string *String The string to delete the range in
 * @param head size_t The head of the range
 * @param tail size_t The tail of the range
 * @return size_t The true head
 */
size_t string_delete_range(String *string, size_t head, size_t tail)
{
    // Ensure the head is before the tail
    if (head > tail) {
        size_t temp = head;
        head = tail;
        tail = temp;
    }

    head = max(min(head, string->length), 0);
    tail = max(min(tail, string->length), 0);

    string_replace(string, head, string->chars + tail, string->length - tail);
    string->length -= tail - head;
    return head;
}
