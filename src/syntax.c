#include "syntax.h"

/*
 * Deallocate all allocated memory for a word list
 * @param list *WordList The word list to free
 */
void word_list_free(WordList *list)
{
    if (list->words) {
        for (size_t i = 0; i < list->length; ++i)
            free(list->words[i].chars);

        free(list->words);
    }

    list->length = list->capacity = 0;
}

/*
 * Push a word onto a word list
 * @param list *WordList The word list to push the word onto
 * @param word *char The words to push onto the word list
 */
void word_list_push(WordList *list, char *word)
{
    // Grow the listionary if required
    if (list->length >= list->capacity) {
        list->capacity = GROW_CAPACITY(list->capacity);
        list->words = GROW_ARRAY(list->words, char*, list->capacity);
    }

    // Push the word onto the listionary
    list->words[list->length++] = word_from_cstr(word);
}

/*
 * Create a word from a C-string
 * @param word *char The C-string
 * @return Word The allocated word
 */
Word word_from_cstr(const char *word)
{
    return (Word) {
        .chars = strdup(word),
        .length = strlen(word)
    };
}

/*
 * Deallocate all allocated memory for a pair list
 * @param list *PairList The pair list to free
 */
void pair_list_free(PairList *list)
{
    if (list->pairs) {
        for (size_t i = 0; i < list->length; ++i) {
            free(list->pairs[i].words[0].chars);
            free(list->pairs[i].words[1].chars);
        }

        free(list->pairs);
    }

    list->length = list->capacity = 0;
}

/*
 * Push a pair onto a pair list
 * @param list *PairList The pair list to push the pair onto
 * @param first *char The first word
 * @param second *char The second word
 */
void pair_list_push(PairList *list, char *first, char *second)
{
    // Grow the listionary if required
    if (list->length >= list->capacity) {
        list->capacity = GROW_CAPACITY(list->capacity);
        list->pairs = GROW_ARRAY(list->pairs, Pair, list->capacity);
    }

    // Push the pair onto the listionary
    list->pairs[list->length++] = (Pair) {
        .words = {
            word_from_cstr(first),
            word_from_cstr(second),
        }
    };
}

/*
 * Find the word at a point in a string inside a pair list
 * @param list PairList The pair list to search
 * @param line String The source line
 * @param start size_t The starting position
 * @param element size_t Which index of the pairs to match against
 * @return int The index of the matching pair and -1 if not found
 */
int pair_list_find(PairList list, String line, size_t start, size_t element)
{
    for (size_t i = 0; i < list.length; ++i) {
        Word word = list.pairs[i].words[element];

        if (word.length <= line.length - start &&
            memcmp(line.chars + start, word.chars, word.length) == 0) {

            return i;
        }
    }

    return -1;
}

/*
 * Check if a character is a separator
 * @param syntax Syntax The syntax definition to use
 * @param ch char The character to check
 * @return bool Whether the character is a separator
 */
bool syntax_match_separator(Syntax syntax, char ch)
{
    return strchr(syntax.separators, ch) != NULL;
}

/*
 * Check if a token ends with a separator
 * @param syntax Syntax The syntax definition to use
 * @param line String The line in which the token is present
 * @param end size_t The end of the token
 * @return bool Whether the token ends with a separator
 */
bool syntax_ends_with_separator(Syntax syntax, String line, size_t end)
{
    return end == line.length ||
        (end < line.length &&
         syntax_match_separator(syntax, line.chars[end]));
}

/*
 * Try to match an index of a line with some token from a word list
 * @param syntax Syntax The syntax definition to use
 * @param list WordList The word list
 * @param line String The line
 * @param start size_t The index to match against
 * @param type TokenType The type to assign if a match is obtained
 * @return Token The token generated
 */
Token syntax_match_word_token(Syntax syntax, WordList list, String line, size_t start, TokenType type)
{
    Token token = {0};
    token.pair = -1;

    for (size_t i = 0; i < list.length; ++i) {
        Word word = list.words[i];

        if (word.length <= line.length - start &&
            memcmp(line.chars + start, word.chars, word.length) == 0) {

            token.type = type;
            token.length = word.length;
            break;
        }
    }

    if (token.length == 0) return token;

    if (token.type == TOKEN_COMMENT) {
        token.length = line.length - start;
        return token;
    }

    if (syntax_ends_with_separator(syntax, line, start + token.length)) return token;

    token.type = TOKEN_NORMAL;
    token.length = 0;
    return token;
}

/*
 * Try to match an index of a line with some surrounders from a pair list
 * @param syntax Syntax The syntax definition to use
 * @param list PairList The pair list
 * @param line String The line
 * @param start size_t The index to match against
 * @param type TokenType The type to assign if a match is obtained
 * @return Token The token generated
 */
Token syntax_match_pair_token(PairList list, String line, size_t start, TokenType type)
{
    Token token = {0};
    token.pair = pair_list_find(list, line, start, 0);

    if (token.pair == -1) return token;
    token.type = type;

    Word end = list.pairs[token.pair].words[1];

    for (size_t i = start + list.pairs[token.pair].words[0].length; i < line.length; ++i) {
        if (end.length <= line.length - i &&
            memcmp(end.chars, line.chars + i, end.length) == 0) {

            token.length = i - start + end.length;
            token.pair = -1;
            return token;
        }
    }

    token.length = line.length - start;
    return token;
}

/*
 * Get the token at an index of a line
 * @param syntax Syntax The syntax definition to use
 * @param line String The line to parse
 * @param start size_t The index to parse at
 * @return Token The parsed token
 */
Token syntax_get_token(Syntax syntax, String line, size_t start)
{
    Token token = {0};
    token.pair = -1;

    if (start == line.length) return token;

    token = syntax_match_word_token(syntax, syntax.comment, line, start, TOKEN_COMMENT);
    if (token.length) return token;

    token = syntax_match_word_token(syntax, syntax.types, line, start, TOKEN_TYPE);
    if (token.length) return token;

    token = syntax_match_word_token(syntax, syntax.keywords, line, start, TOKEN_KEYWORD);
    if (token.length) return token;
    
    token = syntax_match_word_token(syntax, syntax.macros, line, start, TOKEN_MACRO);
    if (token.length) return token;

    token = syntax_match_pair_token(syntax.comments, line, start, TOKEN_COMMENT);
    if (token.length) return token;

    token = syntax_match_pair_token(syntax.string, line, start, TOKEN_STRING);
    if (token.length) return token;

    size_t i = start;
    bool type = syntax_match_separator(syntax, line.chars[i]);
    while (++i < line.length && syntax_match_separator(syntax, line.chars[i]) == type);

    token.type = TOKEN_NORMAL;
    token.length = i - start;

    return token;
}

/*
 * End a multiline token
 * @param syntax Syntax The syntax definition to use
 * @param line String The line to check the token end in
 * @param token *Token The token to end
 * @return bool Whether the ending was successful
 */
bool syntax_end_token(Syntax syntax, String line, Token *token)
{
    if (token->pair != -1) {
        Word end;

        switch (token->type) {
        case TOKEN_STRING:
            end = syntax.string.pairs[token->pair].words[1];
            break;
        case TOKEN_COMMENT:
            end = syntax.comments.pairs[token->pair].words[1];
            break;
        default:
            endwin();
            ASSERT(0, "unreachable");
            break;
        }

        for (size_t i = 0; i < line.length; ++i) {
            if (end.length <= line.length - i &&
                memcmp(line.chars + i, end.chars, end.length) == 0) {
                token->length = i + end.length;
                token->pair = -1;
                return true;
            }
        }

        token->length = line.length;
        return false;
    }

    endwin();
    ASSERT(0, "unreachable");
}
