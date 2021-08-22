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
 * Insert a C-string at a particular index in a string
 * @param string *String The string to insert the C-string in
 * @param index size_t The index to insert the C-string in
 * @param string *char The C-string to insert
 * @param count size_t The number of bytes in the C-string to insert
 */
void string_insert(String *string, size_t index, const char *source, size_t count)
{
    // Increase the capacity of the string if required
    if (string->length + count >= string->capacity) {
        string->capacity = GROW_CAPACITY(string->capacity);

        if (string->capacity < count)
            string->capacity = count;

        string->chars = GROW_ARRAY(string->chars, char, string->capacity);
    }

    // Make space for the string in the string if required
    if (index != string->length)
        SHIFT_ARRAY(string->chars, index, count, char, string->length);

    string->length += count;
    memcpy(string->chars + index, source, count);
}

/*
 * Insert a C-string at the end of a string
 * @param string *String The string to insert the C-string in
 * @param source *char The C-string to insert
 * @param count size_t The number of bytes in the C-string to insert
 */
void string_append(String *string, const char *source, size_t count)
{
    string_insert(string, string->length, source, count);
}

/*
 * Delete characters from a point in a string
 * @param string *String The string to delete the characters in
 * @param index size_t The index to delete the characters from
 * @param count size_t The number of characters to delete
 */
void string_delete(String *string, size_t index, size_t count)
{
    if (index + count < string->length)
        BSHIFT_ARRAY(string->chars, index, count, char, string->length);

    string->length -= count;
}

/*
 * Split a string at a particular index
 * @param string *String The string to split
 * @param index size_t The index to split the string at
 * @return *String The left hand side of the split string
 */
String string_split(String *string, size_t index)
{
    String new = {0};

    if (index < string->length) {
        string_append(&new, string->chars + index, string->length - index);
        string->length = index;
    }

    return new;
}

/*
 * Search for a pattern inside a string
 * @param string String The string to search the pattern in
 * @param pattern String The pattern to search
 * @param start size_t The index to start searching from
 * @param forward bool Whether searching is being performed forward
 * @param ignorecase bool Whether the case is ignored while searching
 */
int string_search(String string, String pattern, size_t start, bool forward, bool ignorecase)
{
    if (string.length > pattern.length) {
        int i = start;

        while (true) {
            if (forward) {
                if ((size_t) ++i >= string.length - pattern.length) break;
            } else {
                if (--i <= 0) break;
            }

            if (ignorecase) {
                if (strncasecmp(string.chars + i, pattern.chars, pattern.length) == 0)
                    return i;
            } else {
                if (strncmp(string.chars + i, pattern.chars, pattern.length) == 0)
                    return i;
            }
        }
    }

    return -1;
}
