#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <ctype.h>
#include <assert.h>

#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#define INC_CAP 128
#define KEY_MAX 127

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
    size_t x, y;
} Vector;

bool vector_eq(Vector a, Vector b)
{
    return a.x == b.x && a.y == b.y;
}

// Term
typedef struct {
    size_t bg;
} Color;

#define COLOR_NORMAL (Color) {.bg = 0}
#define COLOR_VISUAL (Color) {.bg = 239}

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
    printf("\x1b[48;5;%zum", color.bg);
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

void string_insert(String *string, size_t index, char ch)
{
    if (string->size >= string->capacity) {
        string->capacity = MAX(INC_CAP, string->capacity + INC_CAP);
        string->data = realloc(string->data, string->capacity);
        assert(string->data);
    }

    memmove(string->data + index + 1, string->data + index, string->size - index);
    string->data[index] = ch;
    string->size++;
}

// Buffer
typedef struct {
    String *strings;
    size_t count;
    size_t capacity;

    bool region;
    Vector cursor;
    Vector marker;
} Buffer;

void buffer_free(Buffer *buffer)
{
    for (size_t i = 0; i < buffer->count; ++i) {
        string_free(buffer->strings + i);
    }
    free(buffer->strings);
    memset(buffer, 0, sizeof(Buffer));
}

void buffer_grow(Buffer *buffer)
{
    if (buffer->count >= buffer->capacity) {
        buffer->capacity = MAX(INC_CAP, buffer->capacity + INC_CAP);
        buffer->strings = realloc(buffer->strings, buffer->capacity * sizeof(String));
        assert(buffer->strings);
    }
}

void buffer_push(Buffer *buffer, String string)
{
    buffer_grow(buffer);
    buffer->strings[buffer->count++] = string;
}

void buffer_insert(Buffer *buffer, char ch)
{
    buffer_grow(buffer);

    if (buffer->count == 0) {
        memset(buffer->strings, 0, sizeof(String));
        buffer->count = 1;
    }

    if (isprint(ch)) {
        string_insert(buffer->strings + buffer->cursor.y, buffer->cursor.x++, ch);
    } else if (ch == '\r') {
        String *prev = buffer->strings + buffer->cursor.y;

        memmove(buffer->strings + buffer->cursor.y + 1,
                buffer->strings + buffer->cursor.y,
                (buffer->count - buffer->cursor.y) * sizeof(String));

        buffer->strings[++buffer->cursor.y] =
            string(prev->data + buffer->cursor.x,
                   prev->size - buffer->cursor.x);

        buffer->count++;

        prev->size = buffer->cursor.x;
        buffer->cursor.x = 0;
    }
}

void buffer_cursor_left(Buffer *buffer)
{
    if (buffer->count) {
        if (buffer->cursor.x) {
            buffer->cursor.x--;
        }
    }
}

void buffer_cursor_right(Buffer *buffer)
{
    if (buffer->count) {
        if (buffer->cursor.x < buffer->strings[buffer->cursor.y].size) {
            buffer->cursor.x++;
        }
    }
}

void buffer_cursor_fix(Buffer *buffer)
{
    if (buffer->count) {
        buffer->cursor.x = MIN(buffer->cursor.x, buffer->strings[buffer->cursor.y].size);
    }
}

void buffer_cursor_up(Buffer *buffer)
{
    if (buffer->cursor.y) {
        buffer->cursor.y--;
        buffer_cursor_fix(buffer);
    }
}

void buffer_cursor_down(Buffer *buffer)
{
    if (buffer->cursor.y + 1 < buffer->count) {
        buffer->cursor.y++;
        buffer_cursor_fix(buffer);
    }
}

void buffer_get_region(Buffer buffer, Vector *start, Vector *end)
{
    if (!buffer.region) {
        return;
    }

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
    buffer_get_region(buffer, &start, &end);

    Vector pen;
    for (pen.y = 0; pen.y < buffer.count; ++pen.y) {
        const String string = buffer.strings[pen.y];

        for (pen.x = 0; pen.x < string.size; ++pen.x) {
            if (buffer.region && vector_eq(pen, start)) {
                term_color(COLOR_VISUAL);
            }

            putchar(string.data[pen.x]);

            if (buffer.region && vector_eq(pen, end)) {
                term_color(COLOR_NORMAL);
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
    void (*buffer)(Buffer *);
    void (*editor)(void);
} Mapping;

static const Mapping normal_mappings[127] = {
    [CTRL('q')] = {.editor = editor_quit},
    [CTRL('b')] = {.buffer = buffer_cursor_left},
    [CTRL('f')] = {.buffer = buffer_cursor_right},
    [CTRL('p')] = {.buffer = buffer_cursor_up},
    [CTRL('n')] = {.buffer = buffer_cursor_down},
    [CTRL(' ')] = {.buffer = buffer_toggle_region},
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
        } else if (isprint(ch) || ch == '\r') {
            buffer_insert(&editor.buffer, ch);
        }
    }

    buffer_free(&editor.buffer);
    term_reset();
    return 0;
}
