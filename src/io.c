#include "io.h"

/*
 * Render the buffer
 * @param buffer *Buffer The buffer to render
 * @param status bool Whether the statusline has to be rendered
 */
void io_render(Buffer *buffer, bool status)
{
    clear();

    size_t anchor_start = (buffer->row / (LINES - 1)) * (LINES - 1);
    size_t anchor_end = min(anchor_start + LINES - 1, buffer->lines_count);

    for (size_t i = anchor_start; i < anchor_end; ++i)
        printw(StringFmt "\n", StringArg(buffer->lines[i]));

    if (status) {
        move(LINES - 1, 0);
        printw("%s (%zu:%zu)", buffer->file, buffer->row, buffer->col);
    }

    move(buffer->row % (LINES - 1), buffer->col);
}

/*
 * Get input from the user from the statusline area
 * @param prompt *char The prompt to display
 * @return String The input string from the user
 */
String io_query(const char *prompt)
{
    int x, y;
    getyx(stdscr, y, x);

    move(LINES - 1, 0);
    printw("%s: ", prompt);

    String input = {0};
    int ch;
    size_t origin = strlen(prompt) + 2;
    size_t col = 0;
    bool running = true;

    char whitespace[COLS];
    memset(whitespace, ' ', COLS);

    while (running) {
        move(LINES - 1, origin);

        if (input.chars) printw(StringFmt, StringArg(input));
        printw("%.*s", COLS - (int) input.length, whitespace);

        move(LINES - 1, origin + col);

        ch = getch();
        switch (ch) {
        case CTRL('q'):
            string_free(&input);
            running = false;
            break;

        case 10:
            running = false;
            break;

        case CTRL('f'):
        case KEY_RIGHT:
            col = min(input.length, col + 1);
            break;

        case CTRL('b'):
        case KEY_LEFT:
            col = max(0, col - 1);
            break;

        case CTRL('a'):
        case KEY_HOME:
            col = 0;
            break;

        case CTRL('e'):
        case KEY_END:
            col = input.length;
            break;

        case CTRL('k'):
        case KEY_BACKSPACE:
            if (col > 0) string_delete(&input, --col, 1);
            break;

        case CTRL('d'):
        case KEY_DC:
            if (col < input.length) string_delete(&input, col, 1);
            break;

        case ERR: break;

        default:
            if (ch != '\n') string_insert(&input, col++, (char *) &ch, 1);
        }
    }

    move(LINES - 1, 0);
    printw("%.*s", COLS, whitespace);
    move(x, y);
    return input;
}

/*
 * IO for a buffer
 * @param buffer *Buffer The buffer to perform IO for
 */
void io_buffer(Buffer *buffer)
{
    int ch;
    bool running = true;
    bool searched = false;

    while (running) {
        io_render(buffer, true);

        if (searched) {
            searched = false;
        } else {
            ch = getch();
        }

        switch (ch) {
        case CTRL('q'):
            running = false;
            break;

        case CTRL('w'):
            buffer_write(buffer);
            break;

        case CTRL('<'):
        case KEY_PPAGE:
            buffer_cursor_top(buffer);
            break;

        case CTRL('>'):
        case KEY_NPAGE:
            buffer_cursor_bottom(buffer);
            break;

        case CTRL('a'):
        case KEY_HOME:
            buffer_cursor_start(buffer);
            break;

        case CTRL('e'):
        case KEY_END:
            buffer_cursor_end(buffer);
            break;

        case CTRL('p'):
        case KEY_UP:
            buffer_cursor_up(buffer);
            break;

        case CTRL('n'):
        case KEY_DOWN:
            buffer_cursor_down(buffer);
            break;

        case CTRL('b'):
        case KEY_LEFT:
            buffer_cursor_left(buffer);
            break;

        case CTRL('f'):
        case KEY_RIGHT:
            buffer_cursor_right(buffer);
            break;

        case CTRL('k'):
        case KEY_BACKSPACE:
            buffer_delete_left(buffer);
            break;

        case CTRL('d'):
        case KEY_DC:
            buffer_delete_right(buffer);
            break;

        case CTRL('r'):
        case CTRL('s'): {
            String pattern = io_query("Search");
            buffer_search(buffer, pattern, ch == CTRL('s'));

            bool searching = true;
            while (searching) {
                io_render(buffer, false);
                ch = getch();

                switch (ch) {
                case CTRL('s'):
                    buffer_search(buffer, pattern, true);
                    break;
                case CTRL('r'):
                    buffer_search(buffer, pattern, false);
                    break;
                case ERR: break;
                default:
                    searching = false;
                    break;
                }
            }

            searched = true;
            break;
        }

        case ERR: break;
        default: buffer_insert_char(buffer, ch);
        }
    }
}
