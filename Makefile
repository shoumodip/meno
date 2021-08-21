LIBS = -lncurses
CFLAGS = -Wall -Wextra -std=c11 -pedantic
SOURCES = $(wildcard src/*)

meno: $(SOURCES)
    $(CC) -o meno $(SOURCES) $(CFLAGS) $(LIBS)
