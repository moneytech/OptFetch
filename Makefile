CC ?= cc

CFLAGS-base = -std=c89 -Wall -Wextra -pedantic -Werror -fPIC
CFLAGS-release = -O2 -fomit-frame-pointer
CFLAGS-debug = -g3 -ggdb -O0 -DDEBUG

CFLAGS = $(CFLAGS-base) $(CFLAGS-release)

optfetch:
	$(CC) $(CFLAGS) -c optfetch.c
	$(CC) -shared -o liboptfetch.so optfetch.o

install: optfetch
	cp optfetch.h /usr/local/include/
	cp liboptfetch.so /usr/local/lib/

clean:
	rm optfetch.o liboptfetch.so
