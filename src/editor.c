#include "editor.h"

/*
 * Check if the virtual cursor position in the editor is correct
 * @param Editor *editor The editor to check the cursor in
 * @return bool Whether the virtual cursor is correct
 */
bool editor_cursor_correct(Editor *editor)
{
    return editor->anchor.x + editor->cursor.x == editor->buffer->cursor.x &&
        editor->anchor.y + editor->cursor.y == editor->buffer->cursor.y &&
        editor->cursor.x < editor->size.x &&
        editor->cursor.y < editor->size.y;
}

/*
 * Refresh the virtual cursor position in the editor
 * @param Editor *editor The editor to refresh the cursor in
 * @return bool Whether the refreshing was needed
 */
bool editor_revert_cursor(Editor *editor)
{
    if (!editor_cursor_correct(editor)) {
        editor->cursor.y = editor->buffer->cursor.y % editor->size.y;
        editor->cursor.x = editor->buffer->cursor.x % editor->size.x;
        editor->anchor.y = editor->buffer->cursor.y - editor->cursor.y;
        editor->anchor.x = editor->buffer->cursor.x - editor->cursor.x;
        return true;
    }
    return false;
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
 * Print a token
 * @param editor *Editor The editor in which to render the token
 * @param col size_t The column of the token
 * @param string String The source of the token
 * @param token Token The token
 */
void editor_print_token(Editor *editor, size_t col, String string, Token token)
{
    attron(COLOR_PAIR(token.type));
    if (token.type == TOKEN_KEYWORD) {
        attron(A_BOLD);
    } else {
        attroff(A_BOLD);
    }

    printw("%.*s",
           min(token.length, editor->size.x - col % editor->size.x),
           string.chars + col);

    attroff(COLOR_PAIR(token.type));
}

/*
 * Render a token
 * @param editor *Editor The editor in which to render the token
 * @param end size_t The ending ordinate
 * @param head *Vec2D The head of the rendering process
 * @param bool Whether the rendering can be continued
 */
bool editor_render_token(Editor *editor, size_t end, Vec2D *head)
{
    String line = editor->buffer->lines[head->y];
    Token token = syntax_get_token(editor->syntax, line, head->x);

    if (head->x == editor->anchor.x)
        editor_render_fringe(editor, head->y);

    editor_print_token(editor, head->x, line, token);
    head->x += token.length;

    if (head->x == line.length && ++head->y < end) {
        if (token.length < editor->size.x) addch('\n');
        head->x = editor->anchor.x;
    }

    while (token.pair != -1 && head->y < end) {
        editor_render_fringe(editor, head->y);

        line = editor->buffer->lines[head->y];
        if (syntax_end_token(editor->syntax, line, &token)) token.pair = -1;
        editor_print_token(editor, head->x, line, token);

        if (token.pair == -1) {
            head->x = token.length;
            return true;
        } else {
            head->y++;
        }

        if (token.length < editor->size.x) addch('\n');
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
    if (editor_revert_cursor(editor) || editor->update) {
        clear();

        Vec2D head = editor->anchor;
        size_t end = min(editor->buffer->length, editor->anchor.y + editor->size.y);
        while (editor_render_token(editor, end, &head)) // addch('~')
                                                            ;

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

    int ch;
    while (true) {
        editor->buffer->cursor = buffer_search_term(editor->buffer,
                                                    search->line,
                                                    origin,
                                                    forward);
        editor->update = true;
        editor_render_buffer(editor, false);

        editor_highlight_matched_term(editor, search->line);
        editor_render_minibuffer(editor);

        ch = getch();
        while (search->line.length != 0 && (ch == CTRL('r') || ch == CTRL('s'))) {
            editor->buffer->cursor = buffer_search_term(editor->buffer,
                                                        search->line,
                                                        editor->buffer->cursor,
                                                        ch == CTRL('s'));
            editor->update = true;
            editor_render_buffer(editor, false);

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
    editor->size.y = LINES - 1;

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
