#ifndef STRING_H
#define STRING_H

#include "common.h"

typedef struct {
    char *chars;
    size_t length;
    size_t capacity;
} String;

#define StringFmt "%.*s"
#define StringArg(string) (int) (string).length, (string).chars

void string_free(String *string);
void string_grow(String *string, size_t size);

void string_delete(String *string, size_t start, size_t count);
void string_insert(String *string, size_t index, const char *source, size_t count);
void string_replace(String *string, size_t start, const char *source, size_t count);

String string_split(String *source, size_t index);
bool string_equal(String a, String b);

size_t string_next_word(String string, size_t start);
size_t string_prev_word(String string, size_t start);

int string_next_term(String string, String term, size_t start);
int string_prev_term(String string, String term, size_t start);

size_t string_delete_range(String *string, size_t head, size_t tail);

#endif // STRING_H
