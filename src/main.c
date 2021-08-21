#include <ncurses.h>
#include "buffer.h"

int main(void)
{
    initscr();
    noecho();
    raw();

    endwin();
}
