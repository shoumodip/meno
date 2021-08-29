#ifndef SYNTAX_H
#define SYNTAX_H

#include "common.h"

typedef struct {
    char *chars;
    size_t length;
} Word;

typedef struct {
    Word *words;
    size_t length;
    size_t capacity;
} Dict;

void dict_free(Dict *dict);
void dict_push(Dict *dict, char *word);
size_t dict_match(Dict dict, const char *line, size_t length);

typedef struct {
    Dict types;
    Dict macros;
    Dict keywords;

    char *comment_start;
    char *comment_end;
    char *comment_line;
} Syntax;

typedef enum {
    SYNTAX_NORMAL,
    SYNTAX_COMMENT,
    SYNTAX_COMMENT_LINE,
    SYNTAX_STRING,
    SYNTAX_KEYWORD,
    SYNTAX_TYPE,
    SYNTAX_MACRO,
} SyntaxType;

#endif // SYNTAX_H
