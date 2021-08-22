#include "io.h"

/*
 * Render the buffer
 * @param buffer *Buffer The buffer to render
 * @param status bool Whether the statusline has to be rendered
 */
void io_render(Buffer *buffer, bool status)
{
    clear();

    const size_t height = LINES - 1;
    const size_t anchor_start = (buffer->row / height) * height;
    const size_t anchor_end = min(anchor_start + height, buffer->lines_count);

    size_t line_space = 2;
    for (size_t i = buffer->lines_count + 1; i != 0; i /= 10) line_space++;

    const size_t width = COLS - line_space;
    const size_t dx = (buffer->col / width) * width;
    for (size_t i = anchor_start; i < anchor_end; ++i) {
        printw("%*zu ", line_space - 1, i + 1);

        if (buffer->lines[i].length > dx) {
            printw(StringFmt,
                   min(buffer->lines[i].length - dx, width),
                   buffer->lines[i].chars + dx);
        }

        if (buffer->lines[i].length < dx + width) addch('\n');
    }

    if (status) {
        move(height, 0);
        printw("%s (%zu:%zu)", buffer->file, buffer->row + 1, buffer->col);
    }

    move(buffer->row % height, line_space + buffer->col - dx);
}

/*
 * Get input from the user from the statusline area
 * @param input *String The string to input into
 * @param prompt *char The prompt to display
 * @return bool Whether the input was successful
 */
bool io_query(String *input, const char *prompt)
{
    int x, y;
    getyx(stdscr, y, x);

    move(LINES - 1, 0);
    printw("%s: ", prompt);

    int ch;
    size_t origin = strlen(prompt) + 2;
    size_t col = 0;

    char whitespace[COLS];
    memset(whitespace, ' ', COLS);

    while (true) {
        move(LINES - 1, origin);

        if (input->chars) printw(StringFmt, StringArg(*input));
        printw("%.*s", COLS - (int) input->length, whitespace);

        move(LINES - 1, origin + col);

        ch = getch();
        switch (ch) {
        case CTRL('q'):
            string_free(input);
            return false;

        case 10:
            return true;

        case CTRL('f'):
        case KEY_RIGHT:
            col = min(input->length, col + 1);
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
            col = input->length;
            break;

        case CTRL('k'):
        case KEY_BACKSPACE:
            if (col > 0) string_delete(input, --col, 1);
            break;

        case CTRL('d'):
        case KEY_DC:
            if (col < input->length) string_delete(input, col, 1);
            break;

        case ERR: break;

        default:
            if (ch != '\n') string_insert(input, col++, (char *) &ch, 1);
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

        case CTRL('g'):
        case KEY_PPAGE:
            buffer_cursor_top(buffer);
            break;

        case CTRL('h'):
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
            String pattern = {0};
            if (!io_query(&pattern, "Search")) break;

            bool searching = buffer_search(buffer, pattern, ch == CTRL('s'), true, true);
            while (searching) {
                io_render(buffer, false);
                ch = getch();

                switch (ch) {
                case CTRL('s'):
                    searching = buffer_search(buffer, pattern, true, true, true);
                    break;
                case CTRL('r'):
                    searching = buffer_search(buffer, pattern, false, true, true);
                    break;
                case ERR: break;
                case CTRL('q'):
                    searching = false;
                    break;
                default:
                    searched = true;
                    searching = false;
                    break;
                }
            }
            break;
        }

        case CTRL('z'): {
            String pattern = {0};
            if (!io_query(&pattern, "Search")) break;

            if (buffer_search(buffer, pattern, true, true, true)) {
                io_render(buffer, false);

                String replacement = {0};
                if (!io_query(&replacement, "Replace")) break;

                buffer_replace(buffer, pattern, replacement, true);
            }
            break;
        }

        case ERR: break;
        default: buffer_insert_char(buffer, ch);
        }
    }
}
