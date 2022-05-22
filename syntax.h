#ifndef SYNTAX_H
#define SYNTAX_H

static const Slice c_keywords[] = {
    Slice("if"),
    Slice("else"),
    Slice("return"),
    {0}
};

static const Slice c_specials[] = {
    Slice("#include"),
    Slice("#define"),
    {0}
};

static const Syntax syntaxes[] = {
    {
        .ident = Slice("#"),
        .keywords = c_keywords,
        .specials = c_specials,
    }
};

#endif // SYNTAX_H
