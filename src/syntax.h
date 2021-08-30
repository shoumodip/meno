#ifndef SYNTAX_H
#define SYNTAX_H

#include "string.h"

typedef enum {
    TOKEN_NORMAL,
    TOKEN_COMMENT,
    TOKEN_STRING,
    TOKEN_KEYWORD,
    TOKEN_TYPE,
    TOKEN_MACRO,
} TokenType;

typedef struct {
    TokenType type;
    size_t length;
    int pair;
} Token;

typedef struct {
    char *chars;
    size_t length;
} Word;

typedef struct {
    Word *words;
    size_t length;
    size_t capacity;
} WordList;

void word_list_free(WordList *list);
void word_list_push(WordList *list, char *word);
Word word_from_cstr(const char *word);

typedef struct {
    Word words[2];
} Pair;

typedef struct {
    Pair *pairs;
    size_t length;
    size_t capacity;
} PairList;

void pair_list_free(PairList *list);
void pair_list_push(PairList *list, char *a, char *b);
int pair_list_find(PairList list, String line, size_t start, size_t element);

typedef struct {
    WordList types;
    WordList macros;
    WordList keywords;
    WordList comment;

    PairList string;
    PairList comments;
    const char *separators;
} Syntax;

bool syntax_match_separator(Syntax syntax, char ch);
bool syntax_ends_with_separator(Syntax syntax, String line, size_t end);
Token syntax_match_word_token(Syntax syntax, WordList list, String line, size_t start, TokenType type);
Token syntax_match_pair_token(PairList list, String line, size_t start, TokenType type);
Token syntax_get_token(Syntax syntax, String line, size_t start);
bool syntax_end_token(Syntax syntax, String line, Token *token);

#endif // SYNTAX_H
