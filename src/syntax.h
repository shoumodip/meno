#ifndef SYNTAX_H
#define SYNTAX_H

typedef struct {
    const char *name;
    const char *format;

    const SV ident;
    const SV comment;
    const SV *keywords;
    const SV *specials;
} Syntax;

static const SV c_keywords[] = {
    SVStatic("if"),
    SVStatic("else"),
    SVStatic("return"),
    {0}
};

static const SV c_specials[] = {
    SVStatic("#include"),
    SVStatic("#define"),
    {0}
};

static const Syntax syntaxes[] = {
    {
        .name = "C/C++",
        .format = "astyle",

        .ident = SVStatic("#"),
        .comment = SVStatic("//"),
        .keywords = c_keywords,
        .specials = c_specials,
    }
};

#endif // SYNTAX_H
