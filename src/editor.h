#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"
#include "minibuffer.h"
#include "syntax.h"

typedef struct {
    Vec2D size;

    Vec2D anchor;
    Vec2D cursor;
    size_t fringe;

    bool update;
    Buffer *buffer;

    Minibuffer minibuffer;
    char *status;

    Syntax syntax;
} Editor;

bool editor_cursor_inscreen(Editor *editor);
bool editor_cursor_correct(Editor *editor);
bool editor_revert_cursor(Editor *editor);
size_t editor_fringe_size(Editor *editor);
void editor_update_lines(Editor *editor);
bool editor_update_size(Editor *editor);

void editor_render_status(Editor *editor);
void editor_render_fringe(Editor *editor, size_t line);
void editor_print_token(Editor *editor, Vec2D index, String string, Token token);
bool editor_render_token(Editor *editor, size_t end, Vec2D *head);
void editor_render_buffer(Editor *editor, bool status);
void editor_render_minibuffer(Editor *editor);
void editor_highlight_matched_term(Editor *editor, String term);
void editor_search(Editor *editor, bool forward);
void editor_interact(Editor *editor);

#endif // EDITOR_H
