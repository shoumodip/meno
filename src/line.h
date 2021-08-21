#ifndef LINE_H
#define LINE_H

#include "common.h"

typedef struct {
    char *text;
    size_t length;
    size_t capacity;
} Line;

#define LineFmt "%.*s"
#define LineArg(line) (int) line.length, line.text

void line_free(Line *line);
void line_insert(Line *line, size_t index, char *string, size_t length);
void line_delete(Line *line, size_t index, size_t count);
void line_append(Line *line, char *string, size_t count);
Line line_split(Line *line, size_t index);

#endif // LINE_H
