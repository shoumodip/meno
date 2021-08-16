#ifndef COMMON_H
#define COMMON_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MINIMUM_CAPACITY 128

#define ASSERT(condition, ...)                  \
    if (!(condition)) {                         \
        fprintf(stderr, "error: ");             \
        fprintf(stderr, __VA_ARGS__);           \
        fprintf(stderr, "\n");                  \
        exit(1);                                \
    }

#define GROW_CAPACITY(capacity)                                         \
    ((capacity < MINIMUM_CAPACITY) ? MINIMUM_CAPACITY : capacity * 2)

#define GROW_ARRAY(array, size, capacity)       \
    ((array)                                    \
     ? realloc(array, sizeof(size) * capacity)  \
     : malloc(sizeof(size) * capacity))

#define SHIFT_ARRAY(array, index, space, type, length)  \
    memmove(array + index + space,                      \
            array + index,                              \
            sizeof(type) * (length - index))

#endif // COMMON_H
