// Copyright 2021 Shoumodip Kar

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:

// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SV_H
#define SV_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// String Views - An immutable "view" into a char*
typedef struct {
    const char *data;
    size_t size;
} SV;

// Formatting macros for SV:
//   SVFmt - To be used in format string portion
//   SVArg - To be used in the variadics portion
//
// Example:
//   printf(SVFmt "\n", SVArg(s));
//
// Caveats:
//   printf(SVFmt, SvArg(sv_split(s, ' ')));
//
// The above code will split the string view *twice* because of
// the macro expansion. Essentially it becomes:
//   printf("%.*s", (int) sv_split(s, ' ').size, sv_split(s, ' ').data);

#define SVFmt "%.*s"
#define SVArg(s) (int) (s).size, (s).data
#define SVStatic(s) {.data = s, .size = sizeof(s) - 1}

// Create a string view.
SV sv(const char *data, size_t size);

// Create a string view from a C-string, with automatic size
// determination using strlen()
SV sv_cstr(const char *data);

// Trim all 'CH' characters from the left side of the string view.
//
// Example:
//   sv_ltrim(sv_cstr("  foo  ", ' '))            => "foo  "
SV sv_ltrim(SV sv, char ch);

// Like sv_ltrim() but takes a predicate function.
//
// Example:
//   sv_ltrim_pred(sv_cstr("69foo69", isdigit))   => "foo69"
SV sv_ltrim_pred(SV sv, bool (*predicate)(char ch));

// Like sv_ltrim() but from the right.
//
// Example:
//   sv_rtrim(sv_cstr("  foo  ", ' '))            => "  foo"
SV sv_rtrim(SV sv, char ch);

// Like sv_ltrim_pred() but from the right.
//
// Example:
//   sv_rtrim_pred(sv_cstr("69foo69", isdigit))   => "69foo"
SV sv_rtrim_pred(SV sv, bool (*predicate)(char ch));

// Combination of sv_ltrim() and sv_rtrim().
//
// Example:
//   sv_trim(sv_cstr("  foo  ", ' '))             => "foo"
SV sv_trim(SV sv, char ch);

// Combination of sv_ltrim_pred() and sv_rtrim_pred().
//
// Example:
//   sv_trim_pred(sv_cstr("69foo69", isdigit))    => "foo"
SV sv_trim_pred(SV sv, bool (*predicate)(char ch));

// Split a string view at INDEX.
// It generates a new SV with the part of the original SV before INDEX. The
// original SV is changed to after INDEX.
//
// Example:
//   SV a = sv_cstr("foo bar")
//   sv_split_at(&a, 3)         => "foo"
//   a                          => "bar"
SV sv_split_at(SV *sv, size_t index);

// Split a string view by the DELIM character.
// It generates a new SV with the part of the original SV before the
// character. The original SV is changed to after the character.
//
// Example:
//   SV a = sv_cstr("foo bar")
//   sv_split(&a, ' ')          => "foo"
//   a                          => "bar"
SV sv_split(SV *sv, char delim);

// Like sv_split() but splits by a predicate function.
//
// Example:
//   SV a = sv_cstr("foo0bar")
//   sv_split_pred(&a, isdigit) => "foo"
//   a                          => "bar"
SV sv_split_pred(SV *sv, bool (*predicate)(char ch));

// Parse an int and advance the SV.
//
// Example:
//   SV a = sv_cstr("69text")
//   int n
//
//   vs_parse_int(&a, &n) => 2
//   n                    => 69
//   a                    => "text"
//
//   vs_parse_int(&a, &n) => 0
//   a                    => "text"
size_t sv_parse_int(SV *sv, int *dest);

// Like sv_parse_int(), but parses longs instead.
size_t sv_parse_long(SV *sv, long *dest);

// Like sv_parse_int(), but parses floats instead.
size_t sv_parse_float(SV *sv, float *dest);

// Like sv_parse_int(), but parses doubles instead.
size_t sv_parse_double(SV *sv, double *dest);

// Check if two SVs are equal.
//
// Examples:
//   sv_eq(sv_cstr("foo"), sv_cstr("foo")) => true
//   sv_eq(sv_cstr("foo"), sv_cstr("Foo")) => false
//   sv_eq(sv_cstr("foo"), sv_cstr("bar")) => false
//   sv_eq(sv_cstr("foo"), sv_cstr("fo"))  => false
bool sv_eq(SV a, SV b);

// Check if SV starts with PREFIX.
//
// Examples:
//   sv_starts_with(sv_cstr("foo bar"), sv_cstr("foo")) => true
//   sv_starts_with(sv_cstr("foo"), sv_cstr("foo"))     => true
//   sv_starts_with(sv_cstr("foo bar"), sv_cstr("bar")) => false
bool sv_starts_with(SV sv, SV prefix);

// Check if SV ends with PREFIX.
//
// Examples:
//   sv_ends_with(sv_cstr("foo bar"), sv_cstr("bar")) => true
//   sv_ends_with(sv_cstr("foo"), sv_cstr("foo"))     => true
//   sv_ends_with(sv_cstr("foo bar"), sv_cstr("foo")) => false
bool sv_ends_with(SV sv, SV suffix);

// Find the index of CH in SV. Returns -1 if not found.
//
// Examples:
//   sv_find(sv_cstr("foo"), "o") => 1
//   sv_find(sv_cstr("foo"), "a") => -1
int sv_find(SV sv, char ch);

// Read a file into a string view.
//
// Examples:
//   SV contents = sv_read_file("foo.log");
//   printf(SVFmt, SVArg(contents));
//   free((char *) contents);
SV sv_read_file(const char *path);

void sv_advance(SV *sv, size_t count);

///////////////////////////////////////////////////////

SV sv(const char *data, size_t size)
{
    return (SV) {.data = data, .size = size};
}

SV sv_cstr(const char *data)
{
    return (SV) {
        .data = data,
            .size = strlen(data)
    };
}

SV sv_split_at(SV *sv, size_t index)
{
    SV result = *sv;
    result.size = index;
    sv->data += index;
    sv->size -= index;
    return result;
}

SV sv_split(SV *sv, char delim)
{
    if (!sv) return (SV) {0};

    SV result = *sv;

    for (size_t i = 0; i < sv->size; ++i) {
        if (sv->data[i] == delim) {
            result.size = i;
            sv->data += i + 1;
            sv->size -= i + 1;
            return result;
        }
    }

    sv->data += sv->size;
    sv->size = 0;
    return result;
}

SV sv_split_pred(SV *sv, bool (*predicate)(char ch))
{
    if (!sv) return (SV) {0};

    SV result = *sv;

    for (size_t i = 0; i < sv->size; ++i) {
        if (predicate(sv->data[i])) {
            result.size = i;
            sv->data += i + 1;
            sv->size -= i + 1;
            return result;
        }
    }

    sv->data += sv->size;
    sv->size = 0;
    return result;
}

SV sv_ltrim(SV sv, char ch)
{
    for (size_t i = 0; i < sv.size; ++i) {
        if (sv.data[i] != ch) {
            sv.data += i;
            sv.size -= i;
            break;
        }
    }

    return sv;
}

SV sv_ltrim_pred(SV sv, bool (*predicate)(char ch))
{
    for (size_t i = 0; i < sv.size; ++i) {
        if (!predicate(sv.data[i])) {
            sv.data += i;
            sv.size -= i;
            break;
        }
    }

    return sv;
}

SV sv_rtrim(SV sv, char ch)
{
    if (sv.size) {
        for (size_t i = sv.size; i > 0; --i) {
            if (sv.data[i - 1] != ch) {
                sv.size = i;
                break;
            }
        }
    }

    return sv;
}

SV sv_rtrim_pred(SV sv, bool (*predicate)(char ch))
{
    if (sv.size) {
        for (size_t i = sv.size; i > 0; --i) {
            if (!predicate(sv.data[i - 1])) {
                sv.size = i;
                break;
            }
        }
    }

    return sv;
}

SV sv_trim(SV sv, char ch)
{
    return sv_ltrim(sv_rtrim(sv, ch), ch);
}

SV sv_trim_pred(SV sv, bool (*predicate)(char ch))
{
    return sv_ltrim_pred(sv_rtrim_pred(sv, predicate), predicate);
}

int sv_find(SV sv, char ch)
{
    if (!sv.size) return -1;
    const char *p = memchr(sv.data, ch, sv.size);
    return p ? p - sv.data : -1;
}

size_t sv_parse_int(SV *sv, int *dest)
{
    char *endp = NULL;
    *dest = strtol(sv->data, &endp, 10);

    const size_t size = endp - sv->data;
    sv->data = endp;
    sv->size -= size;

    return size;
}

size_t sv_parse_long(SV *sv, long *dest)
{
    char *endp = NULL;
    *dest = strtol(sv->data, &endp, 10);

    const size_t size = endp - sv->data;
    sv->data = endp;
    sv->size -= size;

    return size;
}

size_t sv_parse_float(SV *sv, float *dest)
{
    char *endp = NULL;
    *dest = strtof(sv->data, &endp);

    const size_t size = endp - sv->data;
    sv->data = endp;
    sv->size -= size;

    return size;
}

size_t sv_parse_double(SV *sv, double *dest)
{
    char *endp = NULL;
    *dest = strtod(sv->data, &endp);

    const size_t size = endp - sv->data;
    sv->data = endp;
    sv->size -= size;

    return size;
}

bool sv_eq(SV a, SV b)
{
    return a.size == b.size &&
        memcmp(a.data, b.data, a.size) == 0;
}

bool sv_starts_with(SV sv, SV prefix)
{
    return sv.size >= prefix.size &&
        memcmp(sv.data, prefix.data, prefix.size) == 0;
}

bool sv_ends_with(SV sv, SV suffix)
{
    return sv.size >= suffix.size &&
        memcmp(sv.data + sv.size - suffix.size,
                suffix.data, suffix.size) == 0;
}

SV sv_read_file(const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "error: could not read file '%s'\n", path);
        exit(1);
    }

    if (fseek(file, 0, SEEK_END) < 0) {
        fprintf(stderr, "error: could not read file '%s'\n", path);
        exit(1);
    }

    long size = ftell(file);
    if (size < 0) {
        fprintf(stderr, "error: could not read file '%s'\n", path);
        exit(1);
    }

    rewind(file);

    char *contents = malloc(size);
    if (!contents) {
        fprintf(stderr, "error: could not allocate memory for file '%s'\n", path);
        exit(1);
    }

    if (fread(contents, sizeof(char), size, file) != (size_t) size) {
        fprintf(stderr, "error: could not read file '%s'\n", path);
        exit(1);
    }

    fclose(file);
    return sv(contents, size);
}

void sv_advance(SV *sv, size_t count)
{
    if (sv->size >= count) {
        sv->data += count;
        sv->size -= count;
    }
}

#endif // SV_H
