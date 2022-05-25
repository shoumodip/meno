#ifndef SYNTAX_H
#define SYNTAX_H

typedef struct {
    const SV name;
    const char *format;
    const bool nostring;

    const SV ident;
    const SV comment;
    const SV *keywords;
    const SV *specials;

    const SV *filenames;
    const SV *extensions;
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

static const SV c_extensions[] = {
    SVStatic("c"),
    SVStatic("cpp"),
    SVStatic("h"),
    SVStatic("hpp"),
    {0}
};

static const Syntax syntaxes[] = {
    {
        .name = SVStatic("txt"),
        .nostring = true
    },

    {
        .name = SVStatic("c"),
        .format = "astyle",

        .ident = SVStatic("#"),
        .comment = SVStatic("//"),
        .keywords = c_keywords,
        .specials = c_specials,

        .extensions = c_extensions,
    }
};

static const size_t syntaxes_count = sizeof(syntaxes) / sizeof(Syntax);

#endif // SYNTAX_H
