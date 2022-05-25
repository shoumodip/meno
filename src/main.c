#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#include <ctype.h>
#include <errno.h>
#include <assert.h>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "sv.h"
#include "syntax.h"

#define INC_CAP 128
#define KEY_MAX 128

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Vector
typedef struct {
    size_t x, y;
} Vector;

#define Vector(x, y) (Vector) {x, y}

static inline bool vector_eq(Vector a, Vector b)
{
    return a.x == b.x && a.y == b.y;
}

static inline Vector vector_add(Vector a, Vector b)
{
    return (Vector) {.x = a.x + b.x, .y = a.y + b.y};
}

static inline Vector vector_sub(Vector a, Vector b)
{
    return (Vector) {.x = a.x - b.x, .y = a.y - b.y};
}

// Syntax
typedef enum {
    SYNTAX_NORMAL,
    SYNTAX_KEYWORD,
    SYNTAX_SPECIAL,
    SYNTAX_STRING,
    SYNTAX_COMMENT,
    COUNT_SYNTAXES
} SyntaxType;

bool sv_list_find(const SV *list, SV pred)
{
    while (list && list->data) {
        if (sv_eq(*list++, pred)) {
            return true;
        }
    }
    return false;
}

static inline bool syntax_isident(size_t syntax, char ch)
{
    return isalnum(ch) || ch == '_' || sv_find(syntaxes[syntax].ident, ch) != -1;
}

static inline bool isstring(char ch)
{
    return ch == '"' || ch == '\'';
}

SV syntax_split(size_t syntax, SV *view, SyntaxType *type)
{
    SV word = {0};
    bool found = false;
    if (!syntaxes[syntax].nostring && isstring(*view->data)) {
        found = true;
        *type = SYNTAX_STRING;

        for (size_t i = 1; i < view->size; ++i) {
            if (view->data[i] == '\\') {
                i += 1;
            } else if (view->data[i] == *view->data) {
                word = sv_split_at(view, i + 1);
                break;
            }
        }
    } else if (syntaxes[syntax].comment.size && sv_starts_with(*view, syntaxes[syntax].comment)) {
        found = true;
        *type = SYNTAX_COMMENT;
    } else if (syntax_isident(syntax, *view->data)) {
        for (size_t i = 1; i < view->size; ++i) {
            if (!syntax_isident(syntax, view->data[i])) {
                word = sv_split_at(view, i);
                break;
            }
        }
    } else {
        found = true;
        *type = SYNTAX_NORMAL;

        for (size_t i = 1; i < view->size; ++i) {
            if (isstring(view->data[i]) || syntax_isident(syntax, view->data[i])) {
                word = sv_split_at(view, i);
                break;
            }
        }
    }

    if (!word.data) {
        word = sv_split_at(view, view->size);
    }

    if (!found) {
        if (sv_list_find(syntaxes[syntax].keywords, word)) {
            *type = SYNTAX_KEYWORD;
        } else if (sv_list_find(syntaxes[syntax].specials, word)) {
            *type = SYNTAX_SPECIAL;
        } else {
            *type = SYNTAX_NORMAL;
        }
    }

    return word;
}

// Term
typedef struct {
    int fg;
    int bg;
    int bold;
} Color;

static_assert(COUNT_SYNTAXES == 5);
static const Color color_syntaxes[] = {
    [SYNTAX_NORMAL]  = {.fg = 15, .bg = -1,  .bold = 0},
    [SYNTAX_KEYWORD] = {.fg = 3,  .bg = -1,  .bold = 1},
    [SYNTAX_SPECIAL] = {.fg = 14, .bg = -1,  .bold = 0},
    [SYNTAX_STRING]  = {.fg = 2,  .bg = -1,  .bold = 0},
    [SYNTAX_COMMENT] = {.fg = 8,  .bg = -1,  .bold = 0},
};

#define COLOR_NORMAL (Color) {.fg = -1, .bg = 0,   .bold = -1}
#define COLOR_VISUAL (Color) {.fg = -1, .bg = 239, .bold = -1}
#define COLOR_PROMPT (Color) {.fg = 12, .bg = -1,  .bold = 1}
#define COLOR_SEARCH (Color) {.fg = 0,  .bg = 15,  .bold = 0}
#define COLOR_FAILED (Color) {.fg = 0,  .bg = 9,   .bold = 0}

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
    term.size.y = size.ws_row - 1;
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

void term_color_reset(void)
{
    printf("\x1b[0m");
    fflush(stdout);
}

void term_color(Color color)
{
    if (color.bold == 1) {
        printf("\x1b[1m");
    } else if (color.bold == 0) {
        printf("\x1b[22m");
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

void string_grow(String *string, size_t size)
{
    string->capacity = MAX(string->capacity + INC_CAP, size);
    string->data = realloc(string->data, string->capacity);
    assert(string->data);
}

void string_insert(String *string, size_t index, const char *data, size_t size)
{
    if (string->size + size > string->capacity) {
        string_grow(string, string->size + size);
    }

    memmove(string->data + index + size, string->data + index, string->size - index);
    memcpy(string->data + index, data, size);
    string->size += size;
}

void string_replace(String *string, size_t index, size_t size, String with)
{
    if (with.size > size) {
        string_grow(string, with.size - size);
    }

    memmove(string->data + index + with.size, string->data + index + size, string->size - index - size);
    memcpy(string->data + index, with.data, with.size);

    if (with.size > size) {
        string->size += with.size - size;
    }
}

bool memieq(const char *a, const char *b, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        if (tolower(a[i]) != tolower(b[i])) {
            return false;
        }
    }

    return true;
}

bool string_search_forward(String string, String query, size_t *position)
{
    if (string.size >= query.size) {
        for (size_t i = *position; i <= string.size - query.size; ++i) {
            if (memieq(string.data + i, query.data, query.size)) {
                *position = i;
                return true;
            }
        }
    }

    return false;
}

bool string_search_backward(String string, String query, size_t *position)
{
    if (string.size >= query.size) {
        for (size_t i = *position + 1; i > 0; --i) {
            if (memieq(string.data + i - 1, query.data, query.size)) {
                *position = i - 1;
                return true;
            }
        }
    }

    return false;
}

// Buffer
typedef struct {
    String *lines;
    size_t count;
    size_t capacity;

    bool region;
    Vector cursor;
    Vector marker;

    String path;
    Vector anchor;
    size_t syntax;

    bool modified;
} Buffer;

void buffer_free(Buffer *buffer)
{
    for (size_t i = 0; i < buffer->count; ++i) {
        string_free(buffer->lines + i);
    }
    free(buffer->lines);
    memset(buffer, 0, sizeof(Buffer));
}

void buffer_grow(Buffer *buffer, size_t size)
{
    if (size >= buffer->capacity) {
        buffer->capacity = MAX(buffer->capacity + INC_CAP, size);
        buffer->lines = realloc(buffer->lines, buffer->capacity * sizeof(String));
        assert(buffer->lines);
    }
}

void buffer_push(Buffer *buffer, String line)
{
    buffer_grow(buffer, buffer->count + 1);
    buffer->lines[buffer->count++] = line;
}

void buffer_open(Buffer *buffer)
{
    String path = buffer->path;
    buffer_free(buffer);
    buffer->path = path;

    const int fd = open(buffer->path.data, O_RDONLY);
    if (fd == -1) {
        return;
    }

    struct stat statbuf;
    if (fstat(fd, &statbuf) == -1) {
        close(fd);
        return;
    }

    SV contents = {0};
    contents.size = statbuf.st_size;

    contents.data = mmap(NULL, contents.size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (contents.data == MAP_FAILED) {
        return;
    }

    char *head = (char *) contents.data;
    while (contents.size) {
        const SV line = sv_split(&contents, '\n');
        buffer_push(buffer, string(line.data, line.size));
    }

    munmap(head, statbuf.st_size);
}

void buffer_detect_syntax(Buffer *buffer)
{
    SV path = sv_rtrim(sv(buffer->path.data, buffer->path.size), '\0');
    for (size_t i = 1; i < syntaxes_count; ++i) {
        if (sv_list_find(syntaxes[i].filenames, path)) {
            buffer->syntax = i;
            return;
        }
    }

    sv_split(&path, '.');
    for (size_t i = 1; i < syntaxes_count; ++i) {
        if (sv_list_find(syntaxes[i].extensions, path)) {
            buffer->syntax = i;
            return;
        }
    }

    buffer->syntax = 0;
}

bool buffer_save(Buffer *buffer)
{
    if (buffer->modified) {
        FILE *output = fopen(buffer->path.data, "w");
        if (!output) {
            return false;
        }

        for (size_t i = 0; i < buffer->count; ++i) {
            const String line = buffer->lines[i];
            fprintf(output, "%.*s\n", (int) line.size, line.data);
        }

        fclose(output);
        buffer->modified = false;
    }

    return true;
}

void buffer_anchor_fix(Buffer *buffer)
{
    const Vector limit = vector_add(buffer->anchor, term.size);

    if (buffer->cursor.y >= limit.y) {
        buffer->anchor.y += buffer->cursor.y - limit.y + 1;
    } else if (buffer->cursor.y < buffer->anchor.y) {
        buffer->anchor.y -= buffer->anchor.y - buffer->cursor.y;
    }

    if (buffer->cursor.x >= limit.x) {
        buffer->anchor.x += buffer->cursor.x - limit.x + 1;
    } else if (buffer->cursor.x < buffer->anchor.x) {
        buffer->anchor.x -= buffer->anchor.x - buffer->cursor.x;
    }
}

void buffer_insert(Buffer *buffer, char ch)
{
    buffer->modified = true;
    buffer_grow(buffer, buffer->count + 1);

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

        buffer_anchor_fix(buffer);
    }
}

String buffer_snap_previous_line(Buffer *buffer)
{
    const String line = buffer->lines[--buffer->cursor.y];
    buffer->cursor.x = line.size;
    return line;
}

String buffer_snap_next_line(Buffer *buffer)
{
    const String line = buffer->lines[++buffer->cursor.y];
    buffer->cursor.x = 0;
    return line;
}

void buffer_anchor_snap(Buffer *buffer)
{
    buffer->anchor.x = buffer->cursor.x - buffer->cursor.x % term.size.x;
}

void buffer_backward_char(Buffer *buffer)
{
    if (buffer->count) {
        if (buffer->cursor.x) {
            buffer->cursor.x--;
            buffer_anchor_fix(buffer);
        } else if (buffer->cursor.y) {
            buffer_snap_previous_line(buffer);
            buffer_anchor_snap(buffer);
        }
    }
}

void buffer_forward_char(Buffer *buffer)
{
    if (buffer->count) {
        if (buffer->cursor.x < buffer->lines[buffer->cursor.y].size) {
            buffer->cursor.x++;
            buffer_anchor_fix(buffer);
            buffer_anchor_snap(buffer);
        }
    }
}

void buffer_backward_word(Buffer *buffer)
{
    if (buffer->count) {
        String line = buffer->lines[buffer->cursor.y];

        if (line.size) {
            if (buffer->cursor.x && syntax_isident(buffer->syntax, line.data[buffer->cursor.x])) {
                buffer->cursor.x--;
            }

            while (buffer->cursor.x && !syntax_isident(buffer->syntax, line.data[buffer->cursor.x])) {
                buffer->cursor.x--;
            }
        }

        if (buffer->cursor.x == 0 && buffer->cursor.y) {
            line = buffer_snap_previous_line(buffer);
        }

        while (buffer->cursor.x > 1 && syntax_isident(buffer->syntax, line.data[buffer->cursor.x - 1])) {
            buffer->cursor.x--;
        }

        if (buffer->cursor.x == 1 && syntax_isident(buffer->syntax, *line.data)) {
            buffer->cursor.x--;
        }

        buffer_anchor_snap(buffer);
        buffer_anchor_fix(buffer);
    }
}

void buffer_forward_word(Buffer *buffer)
{
    if (buffer->count) {
        String line = buffer->lines[buffer->cursor.y];

        while (buffer->cursor.x < line.size && !syntax_isident(buffer->syntax, line.data[buffer->cursor.x])) {
            buffer->cursor.x++;
        }

        if (buffer->cursor.x == line.size && buffer->cursor.y < buffer->count) {
            line = buffer_snap_next_line(buffer);
        }

        while (buffer->cursor.x < line.size && syntax_isident(buffer->syntax, line.data[buffer->cursor.x])) {
            buffer->cursor.x++;
        }

        buffer_anchor_snap(buffer);
        buffer_anchor_fix(buffer);
    }
}

void buffer_backward_line(Buffer *buffer)
{
    if (buffer->count) {
        buffer->cursor.x = 0;
    }
}

void buffer_forward_line(Buffer *buffer)
{
    if (buffer->count) {
        buffer->cursor.x = buffer->lines[buffer->cursor.y].size;
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
        buffer_anchor_snap(buffer);
        buffer_anchor_fix(buffer);
    }
}

void buffer_next_line(Buffer *buffer)
{
    if (buffer->cursor.y + 1 < buffer->count) {
        buffer->cursor.y++;
        buffer_cursor_fix(buffer);
        buffer_anchor_snap(buffer);
        buffer_anchor_fix(buffer);
    }
}

void buffer_previous_para(Buffer *buffer)
{
    while (buffer->cursor.y && buffer->lines[buffer->cursor.y].size) {
        buffer->cursor.y--;
    }

    while (buffer->cursor.y && !buffer->lines[buffer->cursor.y].size) {
        buffer->cursor.y--;
    }

    buffer_cursor_fix(buffer);
    buffer_anchor_snap(buffer);
    buffer_anchor_fix(buffer);
}

void buffer_next_para(Buffer *buffer)
{
    while (buffer->cursor.y + 1 < buffer->count && buffer->lines[buffer->cursor.y].size) {
        buffer->cursor.y++;
    }

    while (buffer->cursor.y + 1 < buffer->count && !buffer->lines[buffer->cursor.y].size) {
        buffer->cursor.y++;
    }

    buffer_cursor_fix(buffer);
    buffer_anchor_snap(buffer);
    buffer_anchor_fix(buffer);
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
    bool first = true;
    const size_t space = MIN(buffer.count, buffer.anchor.y + term.size.y);
    for (pen.y = buffer.anchor.y; pen.y < space; ++pen.y) {
        if (first) {
            first = false;
        } else {
            putchar('\n');
        }
        pen.x = 0;

        SV view = {
            .data = buffer.lines[pen.y].data,
            .size = buffer.lines[pen.y].size
        };

        view.size = MIN(view.size, buffer.anchor.x + term.size.x);

        while (view.size) {
            SyntaxType type;
            const SV word = syntax_split(buffer.syntax, &view, &type);

            if (type) term_color(color_syntaxes[type]);
            for (size_t i = 0; i < word.size; ++i) {
                if (buffer.region && vector_eq(pen, start)) {
                    term_color(COLOR_VISUAL);
                }

                if (pen.x >= buffer.anchor.x) {
                    putchar(word.data[i]);
                }

                if (buffer.region && vector_eq(pen, end)) {
                    term_color(COLOR_NORMAL);
                }

                pen.x++;
            }
            if (type) term_color(color_syntaxes[SYNTAX_NORMAL]);
        }

        if (buffer.region && vector_eq(pen, start)) {
            term_color(COLOR_VISUAL);
        }

        if (buffer.region && vector_eq(pen, end)) {
            term_color(COLOR_NORMAL);
        }
    }

    term_move(vector_sub(buffer.cursor, buffer.anchor));
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

    buffer->modified = true;
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

bool buffer_search(Buffer *buffer, String query, bool forward)
{
    if (forward) {
        size_t x = buffer->cursor.x + 1;

        for (size_t y = buffer->cursor.y; y < buffer->count; ++y) {
            if (string_search_forward(buffer->lines[y], query, &x)) {
                buffer->cursor = Vector(x, y);
                goto found;
            }

            if (y == buffer->cursor.y) {
                x = 0;
            }
        }

        for (size_t y = 0; y <= buffer->cursor.y; ++y) {
            if (string_search_forward(buffer->lines[y], query, &x)) {
                buffer->cursor = Vector(x, y);
                goto found;
            }
        }
    } else {
        size_t x = buffer->cursor.x;
        if (x) x--;

        for (size_t y = buffer->cursor.y + 1; y > 0; --y) {
            const String line = buffer->lines[y - 1];

            if (y <= buffer->cursor.y) {
                x = line.size;
            }

            if (string_search_backward(line, query, &x)) {
                buffer->cursor = Vector(x, y - 1);
                goto found;
            }
        }

        for (size_t y = buffer->count; y > buffer->cursor.y; --y) {
            const String line = buffer->lines[y - 1];
            x = line.size;

            if (string_search_backward(line, query, &x)) {
                buffer->cursor = Vector(x, y - 1);
                goto found;
            }
        }
    }

    return false;

found:
    buffer_anchor_snap(buffer);
    buffer_anchor_fix(buffer);
    return true;
}

// Editor
typedef struct {
    Buffer *buffers;
    size_t count;
    size_t capacity;

    bool escape;
    Buffer *buffer;

    String query;
    String search;
} Editor;

static Editor editor;

void editor_new_buffer(void)
{
    if (editor.count == editor.capacity) {
        editor.capacity = editor.capacity + INC_CAP;
        editor.buffers = realloc(editor.buffers, editor.capacity * sizeof(Buffer));
        assert(editor.buffers);
    }

    editor.buffer = editor.buffers + editor.count++;
    memset(editor.buffer, 0, sizeof(Buffer));
}

void editor_delete_buffer(void)
{
    if (!editor.count) return;

    if (editor.buffer) {
        size_t index = editor.buffer - editor.buffers;
        buffer_free(editor.buffer);
        string_free(&editor.buffer->path);
        memmove(editor.buffers + index, editor.buffer + index + 1, (editor.count - index) * sizeof(Buffer));

        if (index) index--;
        editor.buffer = editor.buffers + index;
    }
}

String editor_prompt(const char *prompt, void (*callback)(void *userdata), void *userdata)
{
    editor.query.size = 0;

    while (true) {
        term_move(Vector(0, term.size.y + 1));
        fprintf(stdout, "\x1b[J");
        term_color(COLOR_PROMPT);
        printf(prompt);
        term_color_reset();
        fprintf(stdout, "%.*s", (int) editor.query.size, editor.query.data);
        term_color_reset();

        if (callback) callback(userdata);

        const char ch = getchar();
        switch (ch) {
        case 27:
        case CTRL('c'):
            return (String) {0};

        case '\r':
            return editor.query;

        case 127:
            if (editor.query.size) {
                editor.query.size--;
            }
            break;

        default:
            if (isprint(ch)) {
                string_insert(&editor.query, editor.query.size, &ch, 1);
            }
            break;
        }
    }
}

typedef struct {
    Vector start;
    bool forward;
    bool found;
} Search;

void editor_search_callback(void *userdata)
{
    Search *search = (Search *) userdata;
    editor.buffer->cursor = search->start;
    search->found = buffer_search(editor.buffer, editor.query, search->forward);
    buffer_print(*editor.buffer);

    if (search->found) {
        term_color(COLOR_SEARCH);
        printf("%.*s", (int) editor.query.size, editor.buffer->lines[editor.buffer->cursor.y].data + editor.buffer->cursor.x);
        term_color_reset();
    }

    term_move(Vector(0, term.size.y + 1));
    term_color(COLOR_PROMPT);
    printf("Search: ");

    if (search->found) {
        term_color_reset();
    } else {
        term_color(COLOR_FAILED);
    }

    fprintf(stdout, "%.*s", (int) editor.query.size, editor.query.data);
    term_color_reset();
}

void editor_search(bool forward)
{
    if (!editor.count) return;

    editor.search.size = 0;

    Search search = {
        .start = editor.buffer->cursor,
        .found = false,
        .forward = forward
    };

    const String query = editor_prompt("Search: ", editor_search_callback, &search);
    if (query.size && !vector_eq(editor.buffer->cursor, search.start)) {
        string_insert(&editor.search, 0, query.data, query.size);
    } else {
        editor.buffer->cursor = search.start;
    }
}

void editor_search_further(bool forward)
{
    if (!editor.count) return;
    if (editor.search.size) {
        buffer_search(editor.buffer, editor.search, forward);
    }
}

void editor_search_forward(void)
{
    editor_search(true);
}

void editor_search_further_forward(void)
{
    editor_search_further(true);
}

void editor_search_backward(void)
{
    editor_search(false);
}

void editor_search_further_backward(void)
{
    editor_search_further(false);
}

char editor_prompt_char(const char *prompt, const char *valid)
{
    term_move(Vector(0, term.size.y + 1));
    fprintf(stdout, "\x1b[J");
    term_color(COLOR_PROMPT);
    printf("%s (%s): ", prompt, valid);
    term_color_reset();

    while (true) {
        const char ch = tolower(getchar());
        if (ch == 27 || ch == CTRL('c')) {
            return '\0';
        } else if (strchr(valid, ch)) {
            return ch;
        }
    }
}

static String search_save;

void editor_replace(void)
{
    if (!editor.count) return;

    search_save.size = 0;
    string_insert(&search_save, 0, editor.search.data, editor.search.size);

    editor_search_forward();

    String replace_with = editor_prompt("Replace: ", NULL, NULL);
    bool replace_all = false;
    while (editor.search.size) {
        buffer_print(*editor.buffer);
        term_color(COLOR_SEARCH);
        printf("%.*s", (int) editor.search.size, editor.buffer->lines[editor.buffer->cursor.y].data + editor.buffer->cursor.x);
        term_color_reset();

        bool replace = true;
        if (!replace_all) {
            const char ch = tolower(editor_prompt_char("Replace", "ynaq"));
            if (ch && ch != 'q') {
                if (ch == 'a') {
                    replace_all = true;
                } else {
                    replace = ch == 'y';
                }
            } else {
                break;
            }
        }

        if (replace) {
            string_replace(editor.buffer->lines + editor.buffer->cursor.y,
                           editor.buffer->cursor.x, editor.search.size, replace_with);
        }

        if (!buffer_search(editor.buffer, editor.search, true)) {
            break;
        }
    }

    editor.search.size = 0;
    string_insert(&editor.search, 0, search_save.data, search_save.size);
}

void editor_escape_map(void)
{
    editor.escape = true;
}

void editor_error(const char *format, ...)
{
    term_move(Vector(0, term.size.y + 1));
    term_color(COLOR_FAILED);
    printf("Error: ");

    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);

    term_color_reset();
    getchar();
}

bool editor_save_internal(void)
{
    if (!editor.buffer->path.size) {
        const String path = editor_prompt("Save to: ", NULL, NULL);
        if (!path.size) {
            return false;
        }

        editor.buffer->path = string(path.data, path.size);
    }

    if (!buffer_save(editor.buffer)) {
        editor_error("could not save to file '%s': %s", editor.buffer->path.data, strerror(errno));
        return false;
    }

    return true;
}

void editor_save(void)
{
    if (!editor.count) return;
    editor_save_internal();
}

bool cstr_string_eq(const char *a, String b)
{
    return strlen(a) == b.size && !memcmp(a, b.data, b.size);
}

bool editor_switch_buffer_internal(String path)
{
    for (size_t i = 0; i < editor.count; ++i) {
        if (cstr_string_eq(editor.buffers[i].path.data, path)) {
            editor.buffer = editor.buffers + i;
            return true;
        }
    }
    return false;
}

void editor_switch_buffer(void)
{
    const String path = editor_prompt("Switch buffer: ", NULL, NULL);
    if (!path.size) {
        return;
    }

    if (!editor_switch_buffer_internal(path)) {
        editor_error("no such buffer '%.*s'", (int) path.size, path.data);
    }
}

void editor_find_file(void)
{
    const String path = editor_prompt("Find file: ", NULL, NULL);
    if (!path.size) {
        return;
    }

    if (!editor_switch_buffer_internal(path)) {
        editor_new_buffer();
        editor.buffer->path = string(path.data, path.size);
        string_insert(&editor.buffer->path, editor.buffer->path.size, "\0", 1);
        buffer_open(editor.buffer);
        buffer_detect_syntax(editor.buffer);
        editor.buffer->modified = true;
    }
}

void editor_quit(void)
{
    string_free(&editor.search);
    string_free(&editor.query);
    string_free(&search_save);
    term_reset();
    exit(0);
}

void editor_ctrl_x(void)
{
    term_move(Vector(0, term.size.y + 1));
    printf("C-x");
    term_move(vector_sub(editor.buffer->cursor, editor.buffer->anchor));

    switch (getchar()) {
    case CTRL('r'): editor_replace(); break;
    case CTRL('c'): editor_quit(); break;
    case CTRL('s'): editor_save(); break;
    case CTRL('k'): editor_delete_buffer(); break;
    case CTRL('b'): editor_switch_buffer(); break;
    case CTRL('f'): editor_find_file(); break;
    }
}

void editor_switch_syntax(void)
{
    const String name = editor_prompt("Switch syntax: ", NULL, NULL);
    if (!name.size) {
        return;
    }

    const SV pred = sv(name.data, name.size);
    for (size_t i = 0; i < syntaxes_count; ++i) {
        if (sv_eq(syntaxes[i].name, pred)) {
            editor.buffer->syntax = i;
            return;
        }
    }

    editor_error("no such syntax '"SVFmt"'", SVArg(pred));
}

// Mappings
typedef struct {
    BufferAction buffer;
    BufferAction delete;

    void (*editor)(void);
} Mapping;

static const Mapping normal_mappings[KEY_MAX] = {
    [CTRL('s')] = {.editor = editor_search_forward},
    [CTRL('r')] = {.editor = editor_search_backward},
    [CTRL('x')] = {.editor = editor_ctrl_x},

    [27] = {.editor = editor_escape_map},

    [CTRL('b')] = {.buffer = buffer_backward_char},
    [CTRL('f')] = {.buffer = buffer_forward_char},
    [CTRL('p')] = {.buffer = buffer_previous_line},
    [CTRL('n')] = {.buffer = buffer_next_line},
    [CTRL('v')] = {.buffer = buffer_toggle_region},
    [CTRL('d')] = {.delete = buffer_forward_char},
    [127]       = {.delete = buffer_backward_char},

    [CTRL('a')] = {.buffer = buffer_backward_line},
    [CTRL('e')] = {.buffer = buffer_forward_line},

    [CTRL('k')] = {.delete = buffer_forward_line},
};

static const Mapping escape_mappings[KEY_MAX] = {
    ['s'] = {.editor = editor_search_further_forward},
    ['r'] = {.editor = editor_search_further_backward},
    ['x'] = {.editor = editor_switch_syntax},

    ['b'] = {.buffer = buffer_backward_word},
    ['f'] = {.buffer = buffer_forward_word},
    ['p'] = {.buffer = buffer_previous_para},
    ['n'] = {.buffer = buffer_next_para},
    ['d'] = {.delete = buffer_forward_word},

    [127] = {.delete = buffer_backward_word},
};

int main(int argc, char **argv)
{
    term_init();

    for (int i = 1; i < argc; ++i) {
        editor_new_buffer();
        editor.buffer->path = string(argv[i], strlen(argv[i]));
        string_insert(&editor.buffer->path, editor.buffer->path.size, "\0", 1);
        buffer_open(editor.buffer);
        buffer_detect_syntax(editor.buffer);
        editor.buffer->modified = true;
    }

    if (argc == 1) {
        editor_new_buffer();
    }

    while (true) {
        buffer_print(*editor.buffer);

        const char ch = getchar();
        const Mapping mapping = editor.escape ? escape_mappings[(size_t) ch] : normal_mappings[(size_t) ch];
        editor.escape = false;

        if (mapping.editor) {
            mapping.editor();
        } else if (mapping.buffer) {
            mapping.buffer(editor.buffer);
        } else if (mapping.delete) {
            buffer_delete(editor.buffer, mapping.delete);
        } else if (isprint(ch) || ch == '\r') {
            buffer_insert(editor.buffer, ch);
        }
    }
    return 0;
}
