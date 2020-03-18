CC ?= cc

CFLAGS-base = -Wall -Wextra -pedantic -Werror -fPIC

CFLAGS = $(CFLAGS-base) -std=c89

LDFLAGS =

default: shared

shared:
	$(CC) $(CFLAGS) -c optfetch.c
	$(CC) $(LDFLAGS) -shared -o liboptfetch.so optfetch.o


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
	$(CC) $(CFLAGS-base) -std=c99 -c example.c
	$(CC) $(LDFLAGS) -o example example.o liboptfetch.a

clean:
	rm optfetch.o liboptfetch.so liboptfetch.a example.o example
