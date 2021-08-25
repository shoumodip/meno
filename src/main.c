#include "ui.h"

int main(int argc, char **argv)
{
    ASSERTQ(argc == 2, "invalid number of arguments: '%d'\n" USAGE, argc - 1);

    initscr();
    noecho();
    keypad(stdscr, true);
    raw();

    Buffer buffer = {0};
    buffer_read(&buffer, argv[1], "    ");
    ui_buffer(&buffer);
    buffer_free(&buffer);
    endwin();
}
