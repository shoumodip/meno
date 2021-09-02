#ifndef SYNTAX_H
#define SYNTAX_H

#include "string.h"

typedef enum {
    SYNTAX_NORMAL,
    SYNTAX_COMMENT,
    SYNTAX_STRING,
    SYNTAX_KEYWORD,
    SYNTAX_TYPE,
    SYNTAX_MACRO,
} SyntaxType;

typedef struct {
    SyntaxType type;
    size_t length;
} SyntaxAtom;

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
bool word_list_match_atom(WordList list, const char *source, SyntaxAtom *atom, SyntaxType type);

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
int pair_list_find_element(PairList list, String line, size_t start, size_t element);
int pair_list_find_end(Pair pair, String line, size_t start, size_t offset);

typedef struct {
    int comment;
    int string;
} SyntaxAnchor;

typedef struct {
    SyntaxAnchor *anchors;
    size_t length;
    size_t capacity;

    size_t comment;
    size_t string;
} SyntaxCache;

void syntax_cache_free(SyntaxCache *cache);
void syntax_cache_grow(SyntaxCache *cache, size_t length);

typedef struct {
    char *identifier;

    WordList types;
    WordList macros;
    WordList keywords;
    WordList comment;

    PairList string;
    PairList comments;
} SyntaxContext;

void syntax_context_free(SyntaxContext *context);
bool syntax_match_identifier(SyntaxContext context, char ch);
SyntaxAtom syntax_match_word_atom(WordList list, String line, size_t start, SyntaxType type);
SyntaxAtom syntax_match_pair_atom(PairList list, String line, size_t start, SyntaxType type, SyntaxCache *cache, size_t anchor);
SyntaxAtom syntax_end_paired_atom(PairList list, int *pair, SyntaxType type, String line, size_t start);
SyntaxAtom syntax_get_atom(SyntaxContext context, String line, size_t start, SyntaxCache *cache, size_t anchor);

#endif // SYNTAX_H
