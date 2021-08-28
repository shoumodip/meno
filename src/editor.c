#include "editor.h"
#include "minibuffer.h"

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
    editor->size.x = COLS - editor->fringe;
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
    if (length > COLS) {
        printw("%.*s", COLS - 3, editor->status);
    } else {
        printw("%-*.*s", COLS - length, length, editor->status);
    }
    move(editor->cursor.y, editor->fringe + editor->cursor.x + 1);
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

        int size;
        String line;
        size_t last = min(editor->buffer->length, editor->anchor.y + editor->size.y);
        for (size_t i = editor->anchor.y; i < last; ++i) {
            printw("%*zu ", editor->fringe, i + 1);

            line = editor->buffer->lines[i];
            size = min(editor->size.x, line.length - editor->anchor.x);

            if (size > 0) printw(StringFmt, size, line.chars + editor->anchor.x);
            if (size < (int) editor->size.x) addch('\n');
        }

        editor->update = false;
    }

    if (status) editor_render_status(editor);
}

/*
 * Render a minibuffer in the status area
 * @param editor *Editor The editor to render the minibuffer in
 * @param minibuffer Minibuffer The minibuffer to render
 */
void editor_render_minibuffer(Editor *editor, Minibuffer minibuffer)
{
    move(editor->size.y, 0);
    printw("%s: %-*.*s", minibuffer.prompt,
           editor->size.x + editor->fringe, StringArg(minibuffer.line));
    move(editor->size.y, minibuffer.x + strlen(minibuffer.prompt) + 2);
}

/*
 * Highlight a term if present under the cursor
 * @param editor *Editor The editor to highlight the term in
 * @param term String The term to highlight
 */
void editor_highlight_matched_term(Editor *editor, String term)
{
    Vec2D cursor = editor->buffer->cursor;
    String line = editor->buffer->lines[cursor.y];

    if (strncasecmp(line.chars + cursor.x, term.chars, term.length) == 0) {
        attron(COLOR_PAIR(1));
        attron(A_BOLD);

        move(cursor.y, cursor.x + editor->fringe + 1);
        printw("%.*s", (int) term.length, line.chars + cursor.x);

        attroff(COLOR_PAIR(1));
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
    Minibuffer search = {0};
    Vec2D origin = editor->buffer->cursor;

    search.prompt = "Search";

    int ch;
    while (true) {
        editor->buffer->cursor = buffer_search_term(editor->buffer,
                                                    search.line,
                                                    origin,
                                                    forward);
        editor->update = true;
        editor_render_buffer(editor, false);

        editor_highlight_matched_term(editor, search.line);
        editor_render_minibuffer(editor, search);

        ch = getch();
        while (search.line.length != 0 && (ch == CTRL('r') || ch == CTRL('s'))) {
            editor->buffer->cursor = buffer_search_term(editor->buffer,
                                                        search.line,
                                                        editor->buffer->cursor,
                                                        ch == CTRL('s'));
            editor->update = true;
            editor_render_buffer(editor, false);

            editor_highlight_matched_term(editor, search.line);
            editor_render_minibuffer(editor, search);

            ch = getch();
        }
                
        if (!minibuffer_update(&search, ch))
            break;
    }

    if (search.line.length == 0) {
        editor->buffer->cursor = origin;
    } else {
        string_free(&search.line);
    }

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

    editor->status = malloc(COLS);

    int ch = 0;
    while (true) {
        editor_render_buffer(editor, true);
        ch = getch();

        switch (ch) {
        case CTRL('q'):
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
            if (isprint(ch)) {
                buffer_insert_char(editor->buffer, ch);
                editor_update_lines(editor);
            }
            break;
        }
    }
}
