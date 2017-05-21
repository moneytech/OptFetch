CC ?= cc

CFLAGS = -std=c99 -Wall -Wextra -pedantic -Werror -O2 -fPIC

optfetch:
	$(CC) $(CFLAGS) -c optfetch.c
	$(CC) -shared -o liboptfetch.so optfetch.o

install: optfetch
	cp optfetch.h /usr/local/include/
	cp liboptfetch.so /usr/local/lib/
