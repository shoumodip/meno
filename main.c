#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <ctype.h>
#include <assert.h>

#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "sv.h"

#define INC_CAP 128
#define KEY_MAX 128

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
    size_t x, y;
} Vector;

bool vector_eq(Vector a, Vector b)
{
    return a.x == b.x && a.y == b.y;
}

// Syntax
typedef struct {
    const SV ident;
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
        .ident = SVStatic("#"),
        .keywords = c_keywords,
        .specials = c_specials,
    }
};

typedef enum {
    SYNTAX_NORMAL,
    SYNTAX_KEYWORD,
    SYNTAX_SPECIAL,
    COUNT_SYNTAXES
} SyntaxType;

bool svlist_find(const SV *list, SV pred)
{
    while (list->source) {
        if (sv_eq(*list++, pred)) {
            return true;
        }
    }
    return false;
}

bool syntax_isident(size_t syntax, char ch)
{
    return isalnum(ch) || ch == '_' || sv_find(syntaxes[syntax].ident, ch) != -1;
}

SV syntax_split(size_t syntax, SV *view, SyntaxType *type)
{
    SV word = {0};
    if (syntax_isident(syntax, view->source[0])) {
        for (size_t i = 1; i < view->length; ++i) {
            if (!syntax_isident(syntax, view->source[i])) {
                word = sv_split_at(view, i);
                break;
            }
        }
    } else {
        for (size_t i = 1; i < view->length; ++i) {
            if (syntax_isident(syntax, view->source[i])) {
                word = sv_split_at(view, i);
                break;
            }
        }
    }
    if (!word.source) {
        word = sv_split_at(view, view->length);
    }

    if (svlist_find(syntaxes[syntax].keywords, word)) {
        *type = SYNTAX_KEYWORD;
    } else if (svlist_find(syntaxes[syntax].specials, word)) {
        *type = SYNTAX_SPECIAL;
    } else {
        *type = SYNTAX_NORMAL;
    }

    return word;
}

// Term
typedef struct {
    int fg;
    int bg;
    bool bold;
} Color;

static_assert(COUNT_SYNTAXES == 3);
static const Color color_syntaxes[] = {
    [SYNTAX_NORMAL]  = {.fg = 15, .bg = 0,   .bold = false},
    [SYNTAX_KEYWORD] = {.fg = 3,  .bg = -1,  .bold = true},
    [SYNTAX_SPECIAL] = {.fg = 14, .bg = -1,  .bold = false},
};

#define COLOR_NORMAL  (color_syntaxes[SYNTAX_NORMAL])
#define COLOR_VISUAL  (Color) {.fg = -1, .bg = 239, .bold = false}

typedef struct {
    struct termios save;
    Vector size;
} Term;

static Term term = {0};

void term_init(void)
{
    assert(tcgetattr(STDIN_FILENO, &term.save) != -1);

    struct termios raw = term.save;
    raw.c_iflag &= ~(ICRNL | IXON);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    assert(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != -1);

    struct winsize size;
    assert(ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) != -1);
    term.size.x = size.ws_col;
    term.size.y = size.ws_row;
}

void term_clear(void)
{
    printf("\x1b[2J\x1b[H\x1b[3J");
}

void term_reset(void)
{
    term_clear();
    assert(tcsetattr(STDIN_FILENO, TCSAFLUSH, &term.save) != -1);
}

void term_move(Vector cursor)
{
    printf("\x1b[%zu;%zuH", cursor.y + 1, cursor.x + 1);
    fflush(stdout);
}

void term_color(Color color)
{
    if (color.bold) {
        printf("\x1b[1m");
    } else {
        printf("\x1b[0m");
    }

    if (color.bg != -1) {
        printf("\x1b[48;5;%dm", color.bg);
    }

    if (color.fg != -1) {
        printf("\x1b[38;5;%dm", color.fg);
    }
}

// String
typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} String;

String string(const char *data, size_t size)
{
    String string = {0};
    if (size) {
        string.data = malloc(size);
        assert(string.data);
        memcpy(string.data, data, size);
        string.size = size;
    }
    return string;
}

void string_free(String *string)
{
    free(string->data);
    memset(string, 0, sizeof(String));
}

void string_insert(String *string, size_t index, const char *data, size_t size)
{
    if (string->size >= string->capacity) {
        string->capacity = MAX(INC_CAP, MAX(string->capacity + INC_CAP, string->size + size));
        string->data = realloc(string->data, string->capacity);
        assert(string->data);
    }

    memmove(string->data + index + 1, string->data + index, string->size - index);
    memcpy(string->data + index, data, size);
    string->size += size;
}

// Buffer
typedef struct {
    String *lines;
    size_t count;
    size_t capacity;

    bool region;
    Vector cursor;
    Vector marker;
} Buffer;

void buffer_free(Buffer *buffer)
{
    for (size_t i = 0; i < buffer->count; ++i) {
        string_free(buffer->lines + i);
    }
    free(buffer->lines);
    memset(buffer, 0, sizeof(Buffer));
}

void buffer_grow(Buffer *buffer)
{
    if (buffer->count >= buffer->capacity) {
        buffer->capacity = MAX(INC_CAP, buffer->capacity + INC_CAP);
        buffer->lines = realloc(buffer->lines, buffer->capacity * sizeof(String));
        assert(buffer->lines);
    }
}

void buffer_push(Buffer *buffer, String string)
{
    buffer_grow(buffer);
    buffer->lines[buffer->count++] = string;
}

void buffer_insert(Buffer *buffer, char ch)
{
    buffer_grow(buffer);

    if (buffer->count == 0) {
        memset(buffer->lines, 0, sizeof(String));
        buffer->count = 1;
    }

    if (isprint(ch)) {
        string_insert(buffer->lines + buffer->cursor.y, buffer->cursor.x++, &ch, 1);
    } else if (ch == '\r') {
        String *prev = buffer->lines + buffer->cursor.y;

        memmove(buffer->lines + buffer->cursor.y + 1,
                buffer->lines + buffer->cursor.y,
                (buffer->count - buffer->cursor.y) * sizeof(String));

        buffer->lines[++buffer->cursor.y] =
            string(prev->data + buffer->cursor.x,
                   prev->size - buffer->cursor.x);

        buffer->count++;

        prev->size = buffer->cursor.x;
        buffer->cursor.x = 0;
    }
}

void buffer_backward_char(Buffer *buffer)
{
    if (buffer->count) {
        if (buffer->cursor.x) {
            buffer->cursor.x--;
        }
    }
}

void buffer_forward_char(Buffer *buffer)
{
    if (buffer->count) {
        if (buffer->cursor.x < buffer->lines[buffer->cursor.y].size) {
            buffer->cursor.x++;
        }
    }
}

void buffer_cursor_fix(Buffer *buffer)
{
    if (buffer->count) {
        buffer->cursor.x = MIN(buffer->cursor.x, buffer->lines[buffer->cursor.y].size);
    }
}

void buffer_previous_line(Buffer *buffer)
{
    if (buffer->cursor.y) {
        buffer->cursor.y--;
        buffer_cursor_fix(buffer);
    }
}

void buffer_next_line(Buffer *buffer)
{
    if (buffer->cursor.y + 1 < buffer->count) {
        buffer->cursor.y++;
        buffer_cursor_fix(buffer);
    }
}

void buffer_get_region(Buffer buffer, Vector *start, Vector *end)
{
    *start = buffer.marker;
    *end = buffer.cursor;

    if (start->y > end->y || (start->y == end->y && start->x > end->x)) {
        const Vector temp = *start;
        *start = *end;
        *end = temp;
    }
}

void buffer_print(Buffer buffer)
{
    term_clear();

    Vector start, end;
    if (buffer.region) {
        buffer_get_region(buffer, &start, &end);
    }

    Vector pen;
    for (pen.y = 0; pen.y < buffer.count; ++pen.y) {
        pen.x = 0;

        SV view = sv(buffer.lines[pen.y].data, buffer.lines[pen.y].size);
        while (view.length) {
            SyntaxType type;
            const SV word = syntax_split(0, &view, &type);

            term_color(color_syntaxes[type]);
            for (size_t i = 0; i < word.length; ++i) {
                if (buffer.region && vector_eq(pen, start)) {
                    term_color(COLOR_VISUAL);
                }

                putchar(word.source[i]);

                if (buffer.region && vector_eq(pen, end)) {
                    term_color(COLOR_NORMAL);
                }

                pen.x++;
            }
        }

        if (buffer.region && vector_eq(pen, start)) {
            term_color(COLOR_VISUAL);
        }

        if (buffer.region && vector_eq(pen, end)) {
            term_color(COLOR_NORMAL);
        }

        putchar('\n');
    }

    term_move(buffer.cursor);
}

void buffer_toggle_region(Buffer *buffer)
{
    if (buffer->region) {
        buffer->region = false;
    } else {
        buffer->region = true;
        buffer->marker = buffer->cursor;
    }
}

typedef void (*BufferAction)(Buffer *);

void buffer_delete(Buffer *buffer, BufferAction motion)
{
    if (buffer->count == 0) {
        return;
    }

    if (!buffer->region) {
        buffer->marker = buffer->cursor;
        motion(buffer);
    }

    Vector start, end;
    buffer_get_region(*buffer, &start, &end);

    if (buffer->region && end.x < buffer->lines[end.y].size) {
        end.x++;
    }

    if (start.y == end.y) {
        if (end.x > start.x) {
            String *string = buffer->lines + start.y;
            assert(string->size >= end.x);

            memmove(string->data + start.x, string->data + end.x, string->size - end.x);
            string->size -= end.x - start.x;
        }
    } else {
        String *string_start = buffer->lines + start.y;
        String string_end = buffer->lines[end.y];

        string_start->size = start.x;
        string_insert(string_start, start.x, string_end.data + end.x, string_end.size - end.x);

        for (size_t i = start.y + 1; i <= end.y; ++i) {
            string_free(buffer->lines + i);
        }

        memmove(buffer->lines + start.y + 1,
                buffer->lines + end.y + 1,
                (buffer->count - end.y - 1) * sizeof(String));

        buffer->count -= end.y - start.y;
    }

    buffer->region = false;
    buffer->cursor = start;
}

// Editor
typedef struct {
    bool quit;
    Buffer buffer;
} Editor;

static Editor editor;

void editor_quit(void)
{
    editor.quit = true;
}

// Mappings
typedef struct {
    BufferAction buffer;
    BufferAction delete;

    void (*editor)(void);
} Mapping;

static const Mapping normal_mappings[KEY_MAX] = {
    [CTRL('q')] = {.editor = editor_quit},
    [CTRL('b')] = {.buffer = buffer_backward_char},
    [CTRL('f')] = {.buffer = buffer_forward_char},
    [CTRL('p')] = {.buffer = buffer_previous_line},
    [CTRL('n')] = {.buffer = buffer_next_line},
    [CTRL(' ')] = {.buffer = buffer_toggle_region},
    [CTRL('d')] = {.delete = buffer_forward_char},
    [127]       = {.delete = buffer_backward_char},
};

int main(void)
{
    term_init();

    while (!editor.quit) {
        buffer_print(editor.buffer);

        const char ch = getchar();
        const Mapping mapping = normal_mappings[(size_t) ch];
        if (mapping.editor) {
            mapping.editor();
        } else if (mapping.buffer) {
            mapping.buffer(&editor.buffer);
        } else if (mapping.delete) {
            buffer_delete(&editor.buffer, mapping.delete);
        } else if (isprint(ch) || ch == '\r') {
            buffer_insert(&editor.buffer, ch);
        }
    }

    buffer_free(&editor.buffer);
    term_reset();
    return 0;
}
