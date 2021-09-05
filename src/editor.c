#include "editor.h"

/*
 * Check whether the virtual cursor is inside the screen
 * @param editor *Editor The editor to check the cursor in
 * @return bool Whether the virtual cursor is inside the screen
 */
bool editor_cursor_inscreen(Editor *editor)
{
    return editor->cursor.x < editor->size.x &&
        editor->cursor.y < editor->size.y;
}

/*
 * Check if the virtual cursor in the editor is correct
 * @param Editor *editor The editor to check the cursor in
 * @return bool Whether the virtual cursor is correct
 */
bool editor_cursor_correct(Editor *editor)
{
    return editor->anchor.x + editor->cursor.x == editor->buffer->cursor.x &&
        editor->anchor.y + editor->cursor.y == editor->buffer->cursor.y &&
        editor_cursor_inscreen(editor);
}

/*
 * Refresh the virtual cursor position in the editor
 * @param Editor *editor The editor to refresh the cursor in
 */
void editor_revert_cursor(Editor *editor)
{
    if (editor_update_size(editor)) {
        editor->update = true;
        return;
    }

    Vec2D init = editor->anchor;
    String line = editor->buffer->lines[editor->buffer->cursor.y];

    editor->cursor.x = 0;
    for (size_t i = 0; i < editor->buffer->cursor.x; ++i) {
        if (line.chars[i] == '\t') {
            editor->cursor.x += editor->tabsize - editor->cursor.x % editor->tabsize;
        } else if (iscntrl(line.chars[i])) {
            editor->cursor.x += 2;
        } else {
            editor->cursor.x++;
        }
    }

    editor->cursor.x = editor->cursor.x % editor->size.x;
    editor->cursor.y = editor->buffer->cursor.y % editor->size.y;
    editor->anchor.x = editor->buffer->cursor.x - editor->cursor.x;
    editor->anchor.y = editor->buffer->cursor.y - editor->cursor.y;

    if (diff(init.y, editor->anchor.y) >= editor->size.y) {
        editor->update = true;
    } else if (diff(init.x, editor->anchor.x) >= editor->size.x) {
        editor->update = true;
    }
}

/*
 * The columns required for displaying the fringe
 * @param editor *Editor The editor to check the width in
 */
size_t editor_fringe_size(Editor *editor)
{
    return snprintf(NULL, 0, " %u ", max(editor->buffer->length, 100));
}

/*
 * Update the lines count of the editor
 * @param editor *Editor The editor to update the lines count in
 */
void editor_update_lines(Editor *editor)
{
    editor->fringe = editor_fringe_size(editor);
    editor->size.x = COLS - editor->fringe - 1;
    editor->update = true;
}

/*
 * Update the size of the editor to the terminal window
 * @param editor *Editor The editor to update the size in
 * @return bool Whether the update was required
 */
bool editor_update_size(Editor *editor)
{
    Vec2D original = {
        .x = editor->size.x,
        .y = editor->size.y
    };

    editor->size.x = COLS - editor->fringe - 1;
    editor->size.y = LINES - 1;

    return original.x != editor->size.x || original.y != editor->size.y;
}

/*
 * Render the status line of the editor
 * @param editor *Editor The editor to render the status line of
 */
void editor_render_status(Editor *editor)
{
    int length = snprintf(editor->status, COLS, StringFmt ":%zu:%zu",
                          StringArg(editor->buffer->file),
                          editor->buffer->cursor.y + 1,
                          editor->buffer->cursor.x);

    move(editor->size.y, 0);
    attron(COLOR_PAIR(UI_STATUS));
    if (length > COLS) {
        printw("%.*s", COLS, editor->status);
    } else {
        printw("%-*.*s", COLS, length, editor->status);
    }
    attroff(COLOR_PAIR(UI_STATUS));
    move(editor->cursor.y, editor->fringe + editor->cursor.x + 1);
}

/*
 * Render the fringe for a particular line in the editor
 * @param editor *Editor The editor in which to draw the fringe
 * @param line size_t The line number to draw
 */
void editor_render_fringe(Editor *editor, size_t line)
{
    attron(COLOR_PAIR(UI_LINE_NUMBERS));
    printw("%*zu ", editor->fringe, line + 1);
    attroff(COLOR_PAIR(UI_LINE_NUMBERS));
}

/*
 * Print a string in a particular color
 * @param editor *Editor The editor in which to print the string
 * @param string *char The string to print
 * @param count size_t The number of characters in the maximum limit
 * @param type SyntaxType The syntax type of the string
 */
void editor_print_colored(Editor *editor, const char *string, size_t count, SyntaxType type)
{
    if (count == 0) return;

    attron(COLOR_PAIR(type));
    if (type == SYNTAX_KEYWORD) {
        attron(A_BOLD);
    } else {
        attroff(A_BOLD);
    }
    
    size_t length = 0;
    if (*string == '\t') {
        length = min(editor->size.x,
                     editor->tabsize - editor->cursor.x % editor->tabsize +
                     editor->tabsize * (count - 1));

        printw("%*s", length, "");
    } else if (iscntrl(*string)) {
        length = min(editor->size.x, count * 2);

        size_t space = editor->size.x - editor->cursor.x;
        printw("%.*s", min(space / 2, count), string);
    } else {
        length = min(editor->size.x, count);
        printw("%.*s", length, string);
    }

    editor->cursor.x += length;
    attroff(COLOR_PAIR(type));
}

/*
 * Print an atom
 * @param editor *Editor The editor in which to render the atom
 * @param index Vec2D The starting position of the atom
 * @param string String The source of the atom
 * @param atom SyntaxAtom The atom
 */
void editor_print_atom(Editor *editor, Vec2D position, String string, SyntaxAtom atom)
{
    if (editor->cursor.x < editor->size.x) {
        size_t dx = position.x < editor->anchor.x
            ? editor->anchor.x - position.x
            : 0;

        editor_print_colored(editor,
                             string.chars + position.x + dx,
                             atom.length - dx,
                             atom.type);
    }
}

/*
 * Build the syntax cache upto a point
 * @param editor *Editor The editor to build the cache in
 * @param end size_t The ending ordinate
 * @param Vec2D The coordinate to start rendering from
 */
Vec2D editor_build_cache(Editor *editor, size_t end)
{
    size_t anchor = min(editor->cache.comment, editor->cache.string);

    Vec2D head = {0};
    head.y = anchor * editor->size.y;

    String line = {0};
    SyntaxAtom atom = {0};

    if (head.y == 0) {
        editor->cache.anchors[anchor].comment = anchor
            ? editor->cache.anchors[anchor - 1].comment : -1;

        editor->cache.anchors[anchor].string = anchor
            ? editor->cache.anchors[anchor - 1].string : -1;
    }

    while (head.y < end) {
        line = editor->buffer->lines[head.y];
        atom = syntax_get_atom(editor->syntax, line, head.x, &editor->cache, anchor);

        head.x += atom.length;
        if (head.x == line.length) {
            head.x = 0;
            head.y++;

            if (head.y % editor->size.y == 1) {
                anchor++;
                editor->cache.anchors[anchor].comment = anchor
                    ? editor->cache.anchors[anchor - 1].comment : -1;
                editor->cache.anchors[anchor].string = anchor
                    ? editor->cache.anchors[anchor - 1].string : -1;
            }
        }
    }

    editor->cache.comment = anchor;
    editor->cache.string = anchor;

    return head;
}

/*
 * Render an atom
 * @param editor *Editor The editor in which to render the atom
 * @param end size_t The ending ordinate
 * @param head *Vec2D The head of the rendering process
 * @param anchor size_t The syntax cache anchor to use for rendering the atom
 * @return bool Whether the rendering can be continued
 */
bool editor_render_atom(Editor *editor, size_t end, Vec2D *head, size_t anchor)
{
    String line = editor->buffer->lines[head->y];
    SyntaxAtom atom = syntax_get_atom(editor->syntax, line, head->x, &editor->cache, anchor);

    if (head->x == 0) editor_render_fringe(editor, head->y);

    editor_print_atom(editor, *head, line, atom);
    head->x += atom.length;

    if (head->x == line.length && ++head->y < end) {
        if (line.length < editor->size.x || editor->cursor.x < editor->size.x)
            addch('\n');

        head->x = editor->cursor.x = 0;
    }

    return head->y < end;
}

/*
 * Render the buffer in the editor
 * @param editor *Editor The editor to render the buffer of
 * @param status bool Whether the statusline should be rendered
 */
void editor_render_buffer(Editor *editor, bool status)
{
    editor_revert_cursor(editor);

    if (editor->update) {
        erase();

        Vec2D head = {0};
        head.y = editor->anchor.y;
        
        size_t end = min(editor->buffer->length, editor->anchor.y + editor->size.y);
        size_t anchor = max(end / editor->size.y, 1);

        if (anchor > (size_t) min(editor->cache.comment, editor->cache.string)) {
            syntax_cache_grow(&editor->cache, editor->anchor.y / editor->size.y + 1);
            editor->cache.length = anchor;
            head = editor_build_cache(editor, editor->anchor.y);
        }

        int init_comment = editor->cache.anchors[anchor - 1].comment;
        int init_string = editor->cache.anchors[anchor - 1].comment;

        editor->cache.anchors[anchor - 1].comment = (anchor == 1)
            ? -1 : editor->cache.anchors[anchor - 2].comment;

        editor->cache.anchors[anchor - 1].string = (anchor == 1)
            ? -1 : editor->cache.anchors[anchor - 2].string;

        Vec2D cursor = editor->cursor;
        editor->cursor = (Vec2D) {0};
        while (editor_render_atom(editor, end, &head, anchor - 1));
        editor->cursor = cursor;

        if (editor->cache.anchors[anchor - 1].comment != init_comment)
            editor->cache.comment = anchor - 1;

        if (editor->cache.anchors[anchor - 1].string != init_string)
            editor->cache.string = anchor - 1;

        editor->update = false;
    }

    if (status) editor_render_status(editor);
}

/*
 * Render the minibuffer in the status area
 * @param editor *Editor The editor to render the minibuffer in
 */
void editor_render_minibuffer(Editor *editor)
{
    move(editor->size.y, 0);
    attron(COLOR_PAIR(UI_STATUS));
    printw("%-*.*s", editor->size.x + editor->fringe, StringArg(editor->minibuffer.line));
    attroff(COLOR_PAIR(UI_STATUS));
    move(editor->size.y, editor->minibuffer.x);
}

/*
 * Highlight a term if present under the cursor
 * @param editor *Editor The editor to highlight the term in
 * @param term String The term to highlight
 */
void editor_highlight_matched_term(Editor *editor, String term)
{
    Vec2D cursor = editor->cursor;
    String line = editor->buffer->lines[editor->buffer->cursor.y];

    if (strncasecmp(line.chars + cursor.x, term.chars, term.length) == 0) {
        attron(COLOR_PAIR(UI_SEARCH));
        attron(A_BOLD);

        move(cursor.y, cursor.x + editor->fringe + 1);
        printw("%.*s", (int) term.length, line.chars + cursor.x);

        attroff(COLOR_PAIR(UI_SEARCH));
        attroff(A_BOLD);
    }
}

/*
 * Incremental search
 * @param editor *Editor The editor in which to search
 * @param forward bool Whether the search is done forward
 */
void editor_search(Editor *editor, bool forward)
{
    Minibuffer *search = &editor->minibuffer;
    Vec2D origin = editor->buffer->cursor;

    search->line.length = 0;
    search->x = 0;

    size_t previous = false;
    int ch;
    while (true) {
        if (buffer_search_term(editor->buffer, search->line,
                               origin, forward)) {
            editor->update = true;
            previous = true;
        } else if (previous) {
            editor->update = true;
            previous = false;
        }

        editor_render_buffer(editor, false);
        editor_highlight_matched_term(editor, search->line);
        editor_render_minibuffer(editor);

        ch = getch();
        while (search->line.length != 0 && (ch == CTRL('r') || ch == CTRL('s'))) {
            if (buffer_search_term(editor->buffer, search->line,
                                   editor->buffer->cursor, ch == CTRL('s'))) {
                editor->update = true;
                previous = false;
            } else if (previous) {
                editor->update = true;
                previous = false;
            }

            editor_render_buffer(editor, false);
            editor_highlight_matched_term(editor, search->line);

            editor_highlight_matched_term(editor, search->line);
            editor_render_minibuffer(editor);

            ch = getch();
        }

        if (!minibuffer_update(search, ch))
            break;
    }

    if (search->line.length == 0) {
        editor->buffer->cursor = origin;
    }

    string_free(&search->line);
    editor->update = true;
}

/*
 * Interact with the user
 * @param editor *Editor The editor to interact
 */
void editor_interact(Editor *editor)
{
    editor_update_lines(editor);

    editor->cursor.x = 0;
    editor->cursor.y = 0;

    string_grow(&editor->minibuffer.line, COLS);
    editor->status = malloc(COLS);

    int ch = 0;
    while (true) {
        editor_render_buffer(editor, true);
        ch = getch();

        switch (ch) {
        case CTRL('q'):
            string_free(&editor->minibuffer.line);
            free(editor->status);
            return;

        case KEY_RIGHT:
        case CTRL('f'):
            buffer_next_char(editor->buffer);
            editor->cursor.x++;
            break;

        case KEY_LEFT:
        case CTRL('b'):
            buffer_prev_char(editor->buffer);
            editor->cursor.x--;
            break;

        case KEY_DOWN:
        case CTRL('n'):
            buffer_next_line(editor->buffer);
            editor->cursor.y++;
            break;

        case KEY_UP:
        case CTRL('p'):
            buffer_prev_line(editor->buffer);
            editor->cursor.y--;
            break;

        case KEY_HOME:
        case CTRL('a'):
            buffer_head_line(editor->buffer);
            break;

        case KEY_END:
        case CTRL('e'):
            buffer_tail_line(editor->buffer);
            break;

        case KEY_DC:
        case CTRL('d'):
            buffer_delete_motion(editor->buffer, &buffer_next_char);
            editor_update_lines(editor);
            break;

        case KEY_BACKSPACE:
        case CTRL('k'):
            buffer_delete_motion(editor->buffer, &buffer_prev_char);
            editor_update_lines(editor);
            break;

        case CTRL('s'):
            editor_search(editor, true);
            break;

        case CTRL('r'):
            editor_search(editor, false);
            break;

        case KEY_ESCAPE:
            // M- combinations
            switch (ch = getch()) {
            case 'n':
                buffer_next_para(editor->buffer);
                break;

            case 'p':
                buffer_prev_para(editor->buffer);
                break;

            case 'f':
                buffer_next_word(editor->buffer);
                break;

            case 'b':
                buffer_prev_word(editor->buffer);
                break;

            case KEY_DC:
            case 'd':
                buffer_delete_motion(editor->buffer, &buffer_next_word);
                editor_update_lines(editor);
                break;

            case KEY_BACKSPACE:
            case 'k':
                buffer_delete_motion(editor->buffer, &buffer_prev_word);
                editor_update_lines(editor);
                break;

            case '<':
                buffer_head_file(editor->buffer);
                break;

            case '>':
                buffer_tail_file(editor->buffer);
                break;
            }
            break;

        default:
            if (isprint(ch) || ch == '\n') {
                buffer_insert_char(editor->buffer, ch);
                editor_update_lines(editor);
            }
            break;
        }
    }
}
