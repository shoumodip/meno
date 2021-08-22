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
void string_insert(String *string, size_t index, const char *source, size_t length);
void string_append(String *string, const char *source, size_t count);
void string_delete(String *string, size_t index, size_t count);
String string_split(String *string, size_t index);
int string_search(String string, String pattern, size_t start, bool forward, bool ignorecase);

#endif // STRING_H
