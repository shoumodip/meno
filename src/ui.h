#ifndef UI_H
#define UI_H

#include "buffer.h"

void ui_render(Buffer *buffer, bool status);
bool ui_query(String *input, const char *prompt);
void ui_buffer(Buffer *buffer);

#endif // UI_H
