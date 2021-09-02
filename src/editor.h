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

    SyntaxContext syntax;
    SyntaxCache cache;
} Editor;

bool editor_cursor_inscreen(Editor *editor);
bool editor_cursor_correct(Editor *editor);
void editor_revert_cursor(Editor *editor);
size_t editor_fringe_size(Editor *editor);
void editor_update_lines(Editor *editor);
bool editor_update_size(Editor *editor);

void editor_render_status(Editor *editor);
void editor_render_fringe(Editor *editor, size_t line);
void editor_print_colored(const char *string, size_t count, SyntaxType type);
void editor_print_atom(Editor *editor, Vec2D index, String string, SyntaxAtom atom);
Vec2D editor_build_cache(Editor *editor, size_t end);

bool editor_render_atom(Editor *editor, size_t end, Vec2D *head, size_t anchor);
void editor_render_buffer(Editor *editor, bool status);
void editor_render_minibuffer(Editor *editor);

void editor_highlight_matched_term(Editor *editor, String term);
void editor_search(Editor *editor, bool forward);
void editor_interact(Editor *editor);

#endif // EDITOR_H
