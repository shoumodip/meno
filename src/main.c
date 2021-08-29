#include "editor.h"

// C-syntax for testing
void c_syntax(Syntax *syntax)
{
    dict_push(&syntax->types, "int");
    dict_push(&syntax->types, "void");
    dict_push(&syntax->types, "float");
    dict_push(&syntax->types, "double");
    dict_push(&syntax->types, "char");
    dict_push(&syntax->types, "bool");

    dict_push(&syntax->keywords, "if");
    dict_push(&syntax->keywords, "else");
    dict_push(&syntax->keywords, "while");
    dict_push(&syntax->keywords, "for");
    dict_push(&syntax->keywords, "do");
    dict_push(&syntax->keywords, "const");
    dict_push(&syntax->keywords, "typedef");
    dict_push(&syntax->keywords, "struct");
    dict_push(&syntax->keywords, "union");
    dict_push(&syntax->keywords, "enum");

    dict_push(&syntax->macros, "#include");
    dict_push(&syntax->macros, "#ifndef");
    dict_push(&syntax->macros, "#ifdef");
    dict_push(&syntax->macros, "#define");
    dict_push(&syntax->macros, "#endif");

    syntax->comment_start = "/*";
    syntax->comment_end = "*/";
    syntax->comment_line = "//";
}

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
    init_pair(SYNTAX_KEYWORD, 11, -1);
    init_pair(SYNTAX_TYPE, 4, -1);
    init_pair(SYNTAX_MACRO, 12, -1);
    init_pair(SYNTAX_STRING, 1, -1);
    init_pair(SYNTAX_COMMENT_LINE, 10, -1);

    Editor editor = {0};
    c_syntax(&editor.syntax);

    Buffer buffer = {0};
    buffer.file = (String) { .chars = argv[1], .length = strlen(argv[1]) };
    buffer_read(&buffer);

    editor.buffer = &buffer;

    editor_interact(&editor);
    buffer_free(&buffer);

    endwin();
}
