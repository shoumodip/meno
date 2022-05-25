#ifndef SYNTAX_H
#define SYNTAX_H

// C/C++
typedef struct {
    const SV name;
    const bool nostring;

    const SV ident;
    const SV comment;
    const SV *keywords;
    const SV *specials;

    const SV *filenames;
    const SV *extensions;
} Syntax;

static const SV c_keywords[] = {
    SVStatic("auto"),
    SVStatic("break"),
    SVStatic("case"),
    SVStatic("const"),
    SVStatic("continue"),
    SVStatic("default"),
    SVStatic("do"),
    SVStatic("else"),
    SVStatic("enum"),
    SVStatic("extern"),
    SVStatic("for"),
    SVStatic("goto"),
    SVStatic("if"),
    SVStatic("register"),
    SVStatic("return"),
    SVStatic("signed"),
    SVStatic("sizeof"),
    SVStatic("static"),
    SVStatic("struct"),
    SVStatic("switch"),
    SVStatic("typedef"),
    SVStatic("union"),
    SVStatic("unsigned"),
    SVStatic("volatile"),
    SVStatic("while"),
    SVStatic("alignas"),
    SVStatic("alignof"),
    SVStatic("and"),
    SVStatic("and_eq"),
    SVStatic("asm"),
    SVStatic("atomic_cancel"),
    SVStatic("atomic_commit"),
    SVStatic("atomic_noexcept"),
    SVStatic("bitand"),
    SVStatic("bitor"),
    SVStatic("catch"),
    SVStatic("class"),
    SVStatic("co_await"),
    SVStatic("co_return"),
    SVStatic("co_yield"),
    SVStatic("compl"),
    SVStatic("concept"),
    SVStatic("const_cast"),
    SVStatic("consteval"),
    SVStatic("constexpr"),
    SVStatic("constinit"),
    SVStatic("decltype"),
    SVStatic("delete"),
    SVStatic("dynamic_cast"),
    SVStatic("explicit"),
    SVStatic("export"),
    SVStatic("friend"),
    SVStatic("inline"),
    SVStatic("mutable"),
    SVStatic("namespace"),
    SVStatic("new"),
    SVStatic("noexcept"),
    SVStatic("not"),
    SVStatic("not_eq"),
    SVStatic("nullptr"),
    SVStatic("operator"),
    SVStatic("or"),
    SVStatic("or_eq"),
    SVStatic("private"),
    SVStatic("protected"),
    SVStatic("public"),
    SVStatic("reflexpr"),
    SVStatic("reinterpret_cast"),
    SVStatic("requires"),
    SVStatic("static_assert"),
    SVStatic("static_cast"),
    SVStatic("synchronized"),
    SVStatic("template"),
    SVStatic("this"),
    SVStatic("thread_local"),
    SVStatic("throw"),
    SVStatic("try"),
    SVStatic("typeid"),
    SVStatic("typename"),
    SVStatic("using"),
    SVStatic("virtual"),
    SVStatic("xor"),
    SVStatic("xor_eq"),
    {0}
};

static const SV c_specials[] = {
    SVStatic("#include"),
    SVStatic("#define"),
    SVStatic("#undef"),
    SVStatic("#if"),
    SVStatic("#ifdef"),
    SVStatic("#ifndef"),
    SVStatic("#else"),
    SVStatic("#endif"),
    {0}
};

static const SV c_extensions[] = {
    SVStatic("c"),
    SVStatic("cpp"),
    SVStatic("h"),
    SVStatic("hpp"),
    {0}
};

// Python
static const SV python_keywords[] = {
    SVStatic("and"),
    SVStatic("as"),
    SVStatic("assert"),
    SVStatic("break"),
    SVStatic("class"),
    SVStatic("continue"),
    SVStatic("def"),
    SVStatic("del"),
    SVStatic("elif"),
    SVStatic("else"),
    SVStatic("except"),
    SVStatic("finally"),
    SVStatic("for"),
    SVStatic("from"),
    SVStatic("global"),
    SVStatic("if"),
    SVStatic("import"),
    SVStatic("in"),
    SVStatic("is"),
    SVStatic("lambda"),
    SVStatic("nonlocal"),
    SVStatic("not"),
    SVStatic("or"),
    SVStatic("pass"),
    SVStatic("raise"),
    SVStatic("return"),
    SVStatic("try"),
    SVStatic("while"),
    SVStatic("with"),
    SVStatic("yield"),
    {0}
};

static const SV python_extensions[] = {
    SVStatic("py"),
    {0}
};

// Javascript
static const SV javascript_keywords[] = {
    SVStatic("abstract"),
    SVStatic("arguments"),
    SVStatic("await"),
    SVStatic("boolean"),
    SVStatic("break"),
    SVStatic("byte"),
    SVStatic("case"),
    SVStatic("catch"),
    SVStatic("char"),
    SVStatic("class"),
    SVStatic("const"),
    SVStatic("continue"),
    SVStatic("debugger"),
    SVStatic("default"),
    SVStatic("delete"),
    SVStatic("do"),
    SVStatic("double"),
    SVStatic("else"),
    SVStatic("enum"),
    SVStatic("eval"),
    SVStatic("export"),
    SVStatic("extends"),
    SVStatic("false"),
    SVStatic("final"),
    SVStatic("finally"),
    SVStatic("float"),
    SVStatic("for"),
    SVStatic("function"),
    SVStatic("goto"),
    SVStatic("if"),
    SVStatic("implements"),
    SVStatic("import"),
    SVStatic("in"),
    SVStatic("instanceof"),
    SVStatic("int"),
    SVStatic("interface"),
    SVStatic("let"),
    SVStatic("long"),
    SVStatic("native"),
    SVStatic("new"),
    SVStatic("null"),
    SVStatic("package"),
    SVStatic("private"),
    SVStatic("protected"),
    SVStatic("public"),
    SVStatic("return"),
    SVStatic("short"),
    SVStatic("static"),
    SVStatic("super"),
    SVStatic("switch"),
    SVStatic("synchronized"),
    SVStatic("this"),
    SVStatic("throw"),
    SVStatic("throws"),
    SVStatic("transient"),
    SVStatic("true"),
    SVStatic("try"),
    SVStatic("typeof"),
    SVStatic("var"),
    SVStatic("void"),
    SVStatic("volatile"),
    SVStatic("while"),
    SVStatic("with"),
    SVStatic("yield"),
    {0}
};

static const SV javascript_extensions[] = {
    SVStatic("js"),
    SVStatic("jsx"),
    {0}
};

// Typescript
static const SV typescript_keywords[] = {
    SVStatic("break"),
    SVStatic("as"),
    SVStatic("any"),
    SVStatic("switch"),
    SVStatic("case"),
    SVStatic("if"),
    SVStatic("throw"),
    SVStatic("else"),
    SVStatic("var"),
    SVStatic("number"),
    SVStatic("string"),
    SVStatic("get"),
    SVStatic("module"),
    SVStatic("type"),
    SVStatic("instanceof"),
    SVStatic("typeof"),
    SVStatic("public"),
    SVStatic("private"),
    SVStatic("enum"),
    SVStatic("export"),
    SVStatic("finally"),
    SVStatic("for"),
    SVStatic("while"),
    SVStatic("void"),
    SVStatic("null"),
    SVStatic("super"),
    SVStatic("this"),
    SVStatic("new"),
    SVStatic("in"),
    SVStatic("return"),
    SVStatic("true"),
    SVStatic("false"),
    SVStatic("any"),
    SVStatic("extends"),
    SVStatic("static"),
    SVStatic("let"),
    SVStatic("package"),
    SVStatic("implements"),
    SVStatic("interface"),
    SVStatic("function"),
    SVStatic("new"),
    SVStatic("try"),
    SVStatic("yield"),
    SVStatic("const"),
    SVStatic("continue"),
    SVStatic("do"),
    SVStatic("catch"),
    {0}
};

static const SV typescript_extensions[] = {
    SVStatic("ts"),
    SVStatic("tsx"),
    {0}
};

// Ruby
static const SV ruby_keywords[] = {
    SVStatic("BEGIN"),
    SVStatic("END"),
    SVStatic("alias"),
    SVStatic("and"),
    SVStatic("begin"),
    SVStatic("break"),
    SVStatic("case"),
    SVStatic("class"),
    SVStatic("def"),
    SVStatic("defined?"),
    SVStatic("do"),
    SVStatic("else"),
    SVStatic("elsif"),
    SVStatic("end"),
    SVStatic("ensure"),
    SVStatic("false"),
    SVStatic("for"),
    SVStatic("if"),
    SVStatic("in"),
    SVStatic("module"),
    SVStatic("next"),
    SVStatic("nil"),
    SVStatic("not"),
    SVStatic("or"),
    SVStatic("redo"),
    SVStatic("rescue"),
    SVStatic("retry"),
    SVStatic("return"),
    SVStatic("self"),
    SVStatic("super"),
    SVStatic("then"),
    SVStatic("true"),
    SVStatic("undef"),
    SVStatic("unless"),
    SVStatic("until"),
    SVStatic("when"),
    SVStatic("while"),
    SVStatic("yield"),
    {0}
};

static const SV ruby_extensions[] = {
    SVStatic("rb"),
    {0}
};

// Crystal
static const SV crystal_keywords[] = {
    SVStatic("abstract"),
    SVStatic("do"),
    SVStatic("if"),
    SVStatic("nil"),
    SVStatic("select"),
    SVStatic("union"),
    SVStatic("alias"),
    SVStatic("else"),
    SVStatic("in"),
    SVStatic("of"),
    SVStatic("self"),
    SVStatic("unless"),
    SVStatic("as"),
    SVStatic("elsif"),
    SVStatic("include"),
    SVStatic("out"),
    SVStatic("sizeof"),
    SVStatic("until"),
    SVStatic("as"),
    SVStatic("end"),
    SVStatic("instance_sizeof"),
    SVStatic("pointerof"),
    SVStatic("struct"),
    SVStatic("verbatim"),
    SVStatic("asm"),
    SVStatic("ensure"),
    SVStatic("is_a"),
    SVStatic("private"),
    SVStatic("super"),
    SVStatic("when"),
    SVStatic("begin"),
    SVStatic("enum"),
    SVStatic("lib"),
    SVStatic("protected"),
    SVStatic("then"),
    SVStatic("while"),
    SVStatic("break"),
    SVStatic("extend"),
    SVStatic("macro"),
    SVStatic("require"),
    SVStatic("true"),
    SVStatic("with"),
    SVStatic("case"),
    SVStatic("false"),
    SVStatic("module"),
    SVStatic("rescue"),
    SVStatic("type"),
    SVStatic("yield"),
    SVStatic("class"),
    SVStatic("for"),
    SVStatic("next"),
    SVStatic("responds_to"),
    SVStatic("typeof"),
    SVStatic("def"),
    SVStatic("fun"),
    SVStatic("nil"),
    SVStatic("return"),
    SVStatic("uninitialized"),
};

static const SV crystal_extensions[] = {
    SVStatic("cr"),
    {0}
};

// Go
static const SV go_keywords[] = {
    SVStatic("break"),
    SVStatic("case"),
    SVStatic("chan"),
    SVStatic("const"),
    SVStatic("continue"),
    SVStatic("default"),
    SVStatic("defer"),
    SVStatic("else"),
    SVStatic("fallthrough"),
    SVStatic("for"),
    SVStatic("func"),
    SVStatic("go"),
    SVStatic("goto"),
    SVStatic("if"),
    SVStatic("import"),
    SVStatic("interface"),
    SVStatic("map"),
    SVStatic("package"),
    SVStatic("range"),
    SVStatic("return"),
    SVStatic("select"),
    SVStatic("struct"),
    SVStatic("switch"),
    SVStatic("type"),
    SVStatic("var"),
};

static const SV go_extensions[] = {
    SVStatic("go"),
    {0}
};

// Rust
static const SV rust_keywords[] = {
    SVStatic("as"),
    SVStatic("break"),
    SVStatic("const"),
    SVStatic("continue"),
    SVStatic("crate"),
    SVStatic("else"),
    SVStatic("enum"),
    SVStatic("extern"),
    SVStatic("false"),
    SVStatic("fn"),
    SVStatic("for"),
    SVStatic("if"),
    SVStatic("impl"),
    SVStatic("in"),
    SVStatic("let"),
    SVStatic("loop"),
    SVStatic("match"),
    SVStatic("mod"),
    SVStatic("move"),
    SVStatic("mut"),
    SVStatic("pub"),
    SVStatic("ref"),
    SVStatic("return"),
    SVStatic("self"),
    SVStatic("Self"),
    SVStatic("static"),
    SVStatic("struct"),
    SVStatic("super"),
    SVStatic("trait"),
    SVStatic("true"),
    SVStatic("type"),
    SVStatic("unsafe"),
    SVStatic("use"),
    SVStatic("where"),
    SVStatic("while"),
    SVStatic("async"),
    SVStatic("await"),
    SVStatic("dyn"),
};

static const SV rust_extensions[] = {
    SVStatic("rs"),
    {0}
};

// Main list
static const Syntax syntaxes[] = {
    {
        .name = SVStatic("txt"),
        .nostring = true
    },

    {
        .name = SVStatic("c"),
        .ident = SVStatic("#"),
        .comment = SVStatic("//"),
        .keywords = c_keywords,
        .specials = c_specials,
        .extensions = c_extensions,
    },

    {
        .name = SVStatic("python"),
        .comment = SVStatic("#"),
        .keywords = python_keywords,
        .extensions = python_extensions,
    },

    {
        .name = SVStatic("javascript"),
        .comment = SVStatic("//"),
        .keywords = javascript_keywords,
        .extensions = javascript_extensions,
    },

    {
        .name = SVStatic("typescript"),
        .comment = SVStatic("//"),
        .keywords = typescript_keywords,
        .extensions = typescript_extensions,
    },

    {
        .name = SVStatic("ruby"),
        .comment = SVStatic("#"),
        .keywords = ruby_keywords,
        .extensions = ruby_extensions,
    },

    {
        .name = SVStatic("crystal"),
        .comment = SVStatic("#"),
        .keywords = crystal_keywords,
        .extensions = crystal_extensions,
    },

    {
        .name = SVStatic("go"),
        .comment = SVStatic("//"),
        .keywords = go_keywords,
        .extensions = go_extensions,
    },

    {
        .name = SVStatic("rust"),
        .comment = SVStatic("//"),
        .keywords = rust_keywords,
        .extensions = rust_extensions,
    },
};

static const size_t syntaxes_count = sizeof(syntaxes) / sizeof(Syntax);

#endif // SYNTAX_H
