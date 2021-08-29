#include "syntax.h"

/*
 * Deallocate all allocated memory for a dictionary
 * @param dict *Dict The dictionary to free
 */
void dict_free(Dict *dict)
{
    if (dict->words) {
        for (size_t i = 0; i < dict->length; ++i)
            free(dict->words[i].chars);

        free(dict->words);
    }

    dict->length = dict->capacity = 0;
}

/*
 * Push a word onto a dictionary
 * @param dict *Dict The dictionary to push the word onto
 * @param word *char The words to push onto the dictionary
 */
void dict_push(Dict *dict, char *word)
{
    // Grow the dictionary if required
    if (dict->length >= dict->capacity) {
        dict->capacity = GROW_CAPACITY(dict->capacity);
        dict->words = GROW_ARRAY(dict->words, char*, dict->capacity);
    }

    // Push the word onto the dictionary
    dict->words[dict->length++] = (Word) {
        .chars = strdup(word),
        .length = strlen(word),
    };
}

size_t dict_match(Dict dict, const char *line, size_t length)
{
    for (size_t i = 0; i < dict.length; ++i) {
        Word word = dict.words[i];

        if (word.length <= length &&
            strncmp(line, word.chars, word.length) == 0) {
            return word.length;
        }
    }
    return 0;
}
