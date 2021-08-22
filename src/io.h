#ifndef IO_H
#define IO_H

#include "buffer.h"

void io_render(Buffer *buffer, bool status);
bool io_query(String *input, const char *prompt);
void io_buffer(Buffer *buffer);

#endif // IO_H
