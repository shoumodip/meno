#include "editor.h"

int main(int argc, char **argv)
{
    ASSERT(argc == 2, "invalid number of arguments: '%d'\n" USAGE, argc - 1);

    initscr();

    noecho();
    keypad(stdscr, true);
    raw();
    start_color();
    use_default_colors();
    init_pair(1, 3, 8);

    Editor editor = {0};

    Buffer buffer = {0};
    buffer.file = (String) { .chars = argv[1], .length = strlen(argv[1]) };
    buffer_read(&buffer);

    editor.buffer = &buffer;

    editor_interact(&editor);
    buffer_free(&buffer);

    endwin();
}
