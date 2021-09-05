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
    if (list->length >= list->capacity) {
        list->capacity = GROW_CAPACITY(list->capacity);
        list->words = GROW_ARRAY(list->words, char*, list->capacity);
    }

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
 * Match an atom from a word list
 * @param list WordList The word list to match against
 * @param source *char The source to match against
 * @param atom *SyntaxAtom The atom to write the information into
 * @param type SyntaxType The atom type
 * @param bool Whether it was successful
 */
bool word_list_match_atom(WordList list, const char *source, SyntaxAtom *atom, SyntaxType type)
{
    for (size_t i = 0; i < list.length; ++i) {
        Word word = list.words[i];

        if (word.length == atom->length &&
            memcmp(source, word.chars, word.length) == 0) {
            atom->type = type;
            return true;
        }
    }
    return false;
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
    if (list->length >= list->capacity) {
        list->capacity = GROW_CAPACITY(list->capacity);
        list->pairs = GROW_ARRAY(list->pairs, Pair, list->capacity);
    }

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
int pair_list_find_element(PairList list, String line, size_t start, size_t element)
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
 * Find the end of pair in a line
 * @param pair Pair The pair to end
 * @param line String The line to find the end in
 * @param start size_t The index to start searching from
 * @param offset size_t The offset at the beginning
 * @return int The index of end of the pair or -1 if not found
 */
int pair_list_find_end(Pair pair, String line, size_t start, size_t offset)
{
    Word end = pair.words[1];

    if (line.length >= end.length) {
        for (size_t i = start + offset; i <= line.length - end.length; ++i) {
            if (line.chars[i] == '\\') {
                i++;
                continue;
            }

            if (memcmp(end.chars, line.chars + i, end.length) == 0) {
                return i - start + end.length;
            }
        }
    }

    return -1;
}

/*
 * Free a syntax cache
 * @param cache *SyntaxCache The syntax cache to free
 */
void syntax_cache_free(SyntaxCache *cache)
{
    if (cache->anchors) free(cache->anchors);
    cache->length = cache->capacity = 0;
}

/*
 * Grow a syntax cache to a particular capacity
 * @param cache *SyntaxCache The syntax cache
 * @param capacity size_t The capacity to grow the cache to
 */
void syntax_cache_grow(SyntaxCache *cache, size_t capacity)
{
    if (capacity > cache->capacity) {
        cache->capacity = max(capacity, GROW_CAPACITY(cache->capacity));
        cache->anchors = GROW_ARRAY(cache->anchors, SyntaxAnchor, cache->capacity);
    }
}

/*
 * Free the allocated memory in a syntax context
 * @param context *SyntaxContext The syntax context to free
 */
void syntax_context_free(SyntaxContext *context)
{
    if (context->identifier) free(context->identifier);

    word_list_free(&context->types);
    word_list_free(&context->keywords);
    word_list_free(&context->macros);
    word_list_free(&context->comment);

    pair_list_free(&context->string);
    pair_list_free(&context->comments);

    word_list_free(&context->indent);
    word_list_free(&context->dedent);
}

/*
 * Check if a character is a separator
 * @param syntax SyntaxContext The syntax context to use
 * @param ch char The character to check
 * @return bool Whether the character is a separator
 */
bool syntax_match_identifier(SyntaxContext context, char ch)
{
    return isalnum(ch) || strchr(context.identifier, ch) != NULL;
}

/*
 * Try to match an index of a line with an atom from a word list
 * @param list WordList The word list
 * @param line String The line
 * @param start size_t The index to match against
 * @param type SyntaxType The type to assign if a match is obtained
 * @return SyntaxAtom The atom generated
 */
SyntaxAtom syntax_match_word_atom(WordList list, String line, size_t start, SyntaxType type)
{
    SyntaxAtom atom = {0};

    for (size_t i = 0; i < list.length; ++i) {
        Word word = list.words[i];

        if (word.length <= line.length - start &&
            memcmp(line.chars + start, word.chars, word.length) == 0) {

            atom.type = type;
            atom.length = word.length;
            break;
        }
    }

    if (atom.length == 0) return atom;

    atom.length = (atom.type == SYNTAX_COMMENT)
        ? line.length - start : 0;

    return atom;
}

/*
 * Try to match an index of a line in a pair list
 * @param list PairList The pair list
 * @param line String The line
 * @param start size_t The index to match against
 * @param type SyntaxType The type to assign if a match is obtained
 * @param cache *SyntaxCache The syntax cache
 * @param anchor size_t The anchor in the syntax cache to use
 * @return SyntaxAtom The atom generated
 */
SyntaxAtom syntax_match_pair_atom(PairList list, String line, size_t start, SyntaxType type, SyntaxCache *cache, size_t anchor)
{
    SyntaxAtom atom = {0};
    int pair = pair_list_find_element(list, line, start, 0);

    if (pair == -1) return atom;
    atom.type = type;

    int end = pair_list_find_end(list.pairs[pair], line, start, list.pairs[pair].words[0].length);
    if (end == -1) {
        if (type == SYNTAX_COMMENT) {
            cache->anchors[anchor].comment = pair;
        } else if (type == SYNTAX_STRING) {
            cache->anchors[anchor].string = pair;
        }
    } else {
        atom.length = end;
        return atom;
    }

    atom.length = line.length - start;
    return atom;
}

/*
 * End a paired atom in the syntax cache if possible
 * @param list PairList The pair list
 * @param pair *int The pair in the syntax cache
 * @param type SyntaxType The atom type
 * @param line String The source line
 * @param start size_t The starting abcissa
 * @return SyntaxAtom The generated atom
 */
SyntaxAtom syntax_end_paired_atom(PairList list, int *pair, SyntaxType type, String line, size_t start)
{
    SyntaxAtom atom = {0};
    int end = pair_list_find_end(list.pairs[*pair], line, start, 0);

    if (end == -1) {
        atom.length = line.length - start;
    } else {
        atom.length = end;
        *pair = -1;
    }

    atom.type = type;
    return atom;
}

/*
 * Get the atom at an index of a line
 * @param syntax Syntax The syntax context to use
 * @param line String The line to parse
 * @param start size_t The index to parse at
 * @return SyntaxAtom The parsed atom
 */
SyntaxAtom syntax_get_atom(SyntaxContext context, String line, size_t start, SyntaxCache *cache, size_t anchor)
{
    SyntaxAtom atom = {0};

    int *comment = &cache->anchors[anchor].comment;
    int *string = &cache->anchors[anchor].string;

    if (start == line.length) return atom;

    if (*comment != -1) {
        return syntax_end_paired_atom(context.comments, comment, SYNTAX_COMMENT, line, start);
    } else if (*string != -1) {
        return syntax_end_paired_atom(context.string, string, SYNTAX_STRING, line, start);
    }

    atom = syntax_match_pair_atom(context.comments, line, start, SYNTAX_COMMENT, cache, anchor);
    if (atom.length) return atom;

    atom = syntax_match_pair_atom(context.string, line, start, SYNTAX_STRING, cache, anchor);
    if (atom.length) return atom;

    atom = syntax_match_word_atom(context.comment, line, start, SYNTAX_COMMENT);
    if (atom.length) return atom;

    size_t i = start;
    if (syntax_match_identifier(context, line.chars[i++])) {
        while (i < line.length && syntax_match_identifier(context, line.chars[i])) i++;
        atom.length = i - start;

        if (word_list_match_atom(context.types, line.chars + start, &atom, SYNTAX_TYPE)) return atom;
        if (word_list_match_atom(context.keywords, line.chars + start, &atom, SYNTAX_KEYWORD)) return atom;
        if (word_list_match_atom(context.macros, line.chars + start, &atom, SYNTAX_MACRO)) return atom;
    } else {
        atom.length = 1;
    }

    atom.type = SYNTAX_NORMAL;
    return atom;
}

static size_t get_indent_level(String line)
{
    size_t level = 0;
    for (size_t i = 0; i < line.length; ++i) {
        if (line.chars[i] == '\t') {
            level++;
        } else if (line.chars[i] == ' ') {
            level++;
            i += 3;
        } else {
            break;
        }
    }
    return level;
}

size_t syntax_indent_level(SyntaxContext context, String previous, SyntaxCache *cache)
{
    size_t level = get_indent_level(previous);
    SyntaxAtom atom = {0};

    for (size_t i = level; i < previous.length; i += atom.length) {
        atom = syntax_get_atom(context, previous, i, cache, 0);

        if (word_list_match_atom(context.indent, previous.chars + i, &atom, SYNTAX_NORMAL)) {
            level++;
        } else if (word_list_match_atom(context.dedent, previous.chars + i, &atom, SYNTAX_NORMAL)) {
            if (level > 0) level--;
        }
    }

    return level;
}
