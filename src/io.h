#ifndef IO_H
#define IO_H

#include "buffer.h"

void io_render(Buffer *buffer, bool status);
String io_query(const char *prompt);
void io_buffer(Buffer *buffer);

#endif // IO_H
