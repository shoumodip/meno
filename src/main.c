#include <ncurses.h>
#include "buffer.h"

int main(void)
{
    initscr();
    noecho();

    Line line = {0};

    char ch;
    bool running = true;
    while (running) {
        ch = getch();

        switch (ch) {
        case 'q':
        case ERR:
            running = false;
        default: {}
        }

        line_append(&line, &ch, 1);
        clear();
        printw(LineFmt, LineArg(line));
    }

    endwin();
    line_free(&line);
}
