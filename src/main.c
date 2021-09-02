#include "editor.h"

// C-syntax for testing
void c_syntax(SyntaxContext *context)
{
    context->identifier = strdup("_#");

    word_list_push(&context->types, "int");
    word_list_push(&context->types, "void");
    word_list_push(&context->types, "float");
    word_list_push(&context->types, "double");
    word_list_push(&context->types, "char");
    word_list_push(&context->types, "bool");

    word_list_push(&context->keywords, "if");
    word_list_push(&context->keywords, "else");
    word_list_push(&context->keywords, "while");
    word_list_push(&context->keywords, "for");
    word_list_push(&context->keywords, "do");
    word_list_push(&context->keywords, "const");
    word_list_push(&context->keywords, "static");
    word_list_push(&context->keywords, "auto");
    word_list_push(&context->keywords, "register");
    word_list_push(&context->keywords, "inline");
    word_list_push(&context->keywords, "typedef");
    word_list_push(&context->keywords, "struct");
    word_list_push(&context->keywords, "union");
    word_list_push(&context->keywords, "enum");
    word_list_push(&context->keywords, "return");
    word_list_push(&context->keywords, "switch");
    word_list_push(&context->keywords, "case");
    word_list_push(&context->keywords, "default");

    word_list_push(&context->macros, "#if");
    word_list_push(&context->macros, "#pragma");
    word_list_push(&context->macros, "#include");
    word_list_push(&context->macros, "#ifndef");
    word_list_push(&context->macros, "#ifdef");
    word_list_push(&context->macros, "#define");
    word_list_push(&context->macros, "#endif");

    word_list_push(&context->comment, "//");

    pair_list_push(&context->string, "\"", "\"");
    pair_list_push(&context->string, "'", "'");
    pair_list_push(&context->comments, "/*", "*/");
}

void zenburn_colors(void)
{
    init_pair(SYNTAX_KEYWORD, 11, -1);
    init_pair(SYNTAX_TYPE, 4, -1);
    init_pair(SYNTAX_MACRO, 12, -1);
    init_pair(SYNTAX_STRING, 1, -1);
    init_pair(SYNTAX_COMMENT, 10, -1);

    init_pair(UI_SEARCH, 3, 8);
    init_pair(UI_LINE_NUMBERS, 7, -1);
    init_pair(UI_STATUS, 0, 10);
}

int main(int argc, char **argv)
{
    ASSERT(argc == 2, "invalid number of arguments: '%d'\n" USAGE, argc - 1);

    initscr();

    noecho();
    keypad(stdscr, true);
    raw();
    cbreak();

    start_color();
    use_default_colors();
    zenburn_colors();

    Editor editor = {0};
    c_syntax(&editor.syntax);

    Buffer buffer = {0};
    buffer.file = (String) { .chars = argv[1], .length = strlen(argv[1]) };
    buffer_read(&buffer);

    editor.buffer = &buffer;

    editor_interact(&editor);
    buffer_free(&buffer);
    syntax_context_free(&editor.syntax);
    syntax_cache_free(&editor.cache);

    endwin();
}
