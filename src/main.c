#include <ncurses.h>
#include "buffer.h"

int main(int argc, char **argv)
{
    ASSERT(argc == 2, "invalid number of arguments: '%d'\n" USAGE, argc - 1);

    initscr();
    noecho();
    keypad(stdscr, true);
    raw();

    Buffer buffer = {0};
    buffer_read(&buffer, argv[1], "    ");

    int ch;
    bool running = true;

    while (running) {
        clear();
        for (size_t i = 0; i < buffer.length; ++i)
            printw(LineFmt "\n", LineArg(buffer.lines[i]));
        move(buffer.row, buffer.col);

        ch = getch();
        switch (ch) {
        case CTRL('q'):
            running = false;
            break;

        case CTRL('<'):
        case KEY_PPAGE:
            buffer_cursor_top(&buffer);
            break;

        case CTRL('>'):
        case KEY_NPAGE:
            buffer_cursor_bottom(&buffer);
            break;

        case CTRL('a'):
        case KEY_HOME:
            buffer_cursor_start(&buffer);
            break;

        case CTRL('e'):
        case KEY_END:
            buffer_cursor_end(&buffer);
            break;

        case CTRL('p'):
        case KEY_UP:
            buffer_cursor_up(&buffer);
            break;

        case CTRL('n'):
        case KEY_DOWN:
            buffer_cursor_down(&buffer);
            break;

        case CTRL('b'):
        case KEY_LEFT:
            buffer_cursor_left(&buffer);
            break;

        case CTRL('f'):
        case KEY_RIGHT:
            buffer_cursor_right(&buffer);
            break;

        case CTRL('k'):
        case KEY_BACKSPACE:
            buffer_delete_left(&buffer);
            break;

        case CTRL('d'):
        case KEY_DC:
            buffer_delete_right(&buffer);
            break;

        case ERR: break;
        default:
            buffer_insert_char(&buffer, ch);
        }
    }

    buffer_free(&buffer);

    endwin();
}
