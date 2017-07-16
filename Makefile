CC ?= cc
LD := $(CC)

CFLAGS-base = -std=c99 -Wall -Wextra -pedantic -Werror -fPIC
CFLAGS-release = -O2 -g0 -fomit-frame-pointer
CFLAGS-debug = -g3 -ggdb -O0 -DDEBUG

CFLAGS = $(CFLAGS-base) $(CFLAGS-release)

LDFLAGS =

default: shared

shared:
	$(CC) $(CFLAGS) -c optfetch.c
	$(LD) $(LDFLAGS) -shared -o liboptfetch.so optfetch.o


install: shared
	cp optfetch.h /usr/local/include/
	cp liboptfetch.so /usr/local/lib/

static:
	$(CC) $(CFLAGS) -c optfetch.c
	ar rcs liboptfetch.a optfetch.o

install-static: static
	cp optfetch.h /usr/local/include
	cp liboptfetch.a /usr/local/lib/

example: static
	$(CC) $(CFLAGS) -c example.c
	$(LD) $(LDFLAGS) -o example example.o liboptfetch.a

clean:
	rm optfetch.o liboptfetch.so liboptfetch.a
