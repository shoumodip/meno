#include "editor.h"

// C-syntax for testing
void c_syntax(Syntax *syntax)
{
    syntax->separators = "()[]{}!@#$%^&*-=+\\|;:,.?<> ";

    word_list_push(&syntax->types, "int");
    word_list_push(&syntax->types, "void");
    word_list_push(&syntax->types, "float");
    word_list_push(&syntax->types, "double");
    word_list_push(&syntax->types, "char");
    word_list_push(&syntax->types, "bool");

    word_list_push(&syntax->keywords, "if");
    word_list_push(&syntax->keywords, "else");
    word_list_push(&syntax->keywords, "while");
    word_list_push(&syntax->keywords, "for");
    word_list_push(&syntax->keywords, "do");
    word_list_push(&syntax->keywords, "const");
    word_list_push(&syntax->keywords, "static");
    word_list_push(&syntax->keywords, "auto");
    word_list_push(&syntax->keywords, "register");
    word_list_push(&syntax->keywords, "inline");
    word_list_push(&syntax->keywords, "typedef");
    word_list_push(&syntax->keywords, "struct");
    word_list_push(&syntax->keywords, "union");
    word_list_push(&syntax->keywords, "enum");
    word_list_push(&syntax->keywords, "return");
    word_list_push(&syntax->keywords, "switch");
    word_list_push(&syntax->keywords, "case");
    word_list_push(&syntax->keywords, "default");

    word_list_push(&syntax->macros, "#include");
    word_list_push(&syntax->macros, "#ifndef");
    word_list_push(&syntax->macros, "#ifdef");
    word_list_push(&syntax->macros, "#define");
    word_list_push(&syntax->macros, "#endif");

    word_list_push(&syntax->comment, "//");

    pair_list_push(&syntax->string, "\"", "\"");
    pair_list_push(&syntax->string, "'", "'");
    pair_list_push(&syntax->comments, "/*", "*/");
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
    init_pair(TOKEN_KEYWORD, 11, -1);
    init_pair(TOKEN_TYPE, 4, -1);
    init_pair(TOKEN_MACRO, 12, -1);
    init_pair(TOKEN_STRING, 1, -1);
    init_pair(TOKEN_COMMENT, 10, -1);

    init_pair(UI_SEARCH, 3, 8);
    init_pair(UI_LINE_NUMBERS, 7, -1);
    init_pair(UI_STATUS, 0, 10);

    Editor editor = {0};
    c_syntax(&editor.syntax);

    Buffer buffer = {0};
    buffer.file = (String) { .chars = argv[1], .length = strlen(argv[1]) };
    buffer_read(&buffer);

    editor.buffer = &buffer;

    editor_interact(&editor);
    buffer_free(&buffer);
    word_list_free(&editor.syntax.types);
    word_list_free(&editor.syntax.keywords);
    word_list_free(&editor.syntax.macros);
    word_list_free(&editor.syntax.comment);
    pair_list_free(&editor.syntax.string);
    pair_list_free(&editor.syntax.comments);

    endwin();
}
