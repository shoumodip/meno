#ifndef MINIBUFFER_H
#define MINIBUFFER_H

#include "string.h"

typedef struct {
    const char *prompt;
    String line;
    size_t x;
} Minibuffer;

void minibuffer_next_char(Minibuffer *minibuffer);
void minibuffer_prev_char(Minibuffer *minibuffer);

void minibuffer_next_word(Minibuffer *minibuffer);
void minibuffer_prev_word(Minibuffer *minibuffer);

void minibuffer_goto_head(Minibuffer *minibuffer);
void minibuffer_goto_tail(Minibuffer *minibuffer);

void minibuffer_insert_char(Minibuffer *minibuffer, char ch);
void minibuffer_delete_motion(Minibuffer *minibuffer, void (*motion)(Minibuffer *));

bool minibuffer_update(Minibuffer *minibuffer, int ch);

#endif // MINIBUFFER_H
