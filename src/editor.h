#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"

typedef struct {
    Vec2D size;

    Vec2D anchor;
    Vec2D cursor;
    size_t fringe;

    bool update;
    Buffer *buffer;

    char *status;
} Editor;

bool editor_cursor_correct(Editor *editor);
bool editor_revert_cursor(Editor *editor);
size_t editor_fringe_size(Editor *editor);
void editor_update_lines(Editor *editor);

void editor_render_status(Editor *editor);
void editor_render_buffer(Editor *editor, bool status);
void editor_render_minibuffer(Editor *editor, Minibuffer minibuffer);
void editor_highlight_matched_term(Editor *editor, String term);
void editor_search(Editor *editor, bool forward);
void editor_interact(Editor *editor);

#endif // EDITOR_H
