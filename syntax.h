#ifndef SYNTAX_H
#define SYNTAX_H

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
        .ident = SVStatic("#"),
        .comment = SVStatic("//"),
        .keywords = c_keywords,
        .specials = c_specials,
    }
};

#endif // SYNTAX_H
