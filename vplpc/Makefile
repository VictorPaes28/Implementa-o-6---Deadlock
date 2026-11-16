CC = gcc
CFLAGS = -Wall -Wextra -std=c11

banker: principal.c banqueiro.c banqueiro.h
	$(CC) $(CFLAGS) -o banker principal.c banqueiro.c

clean:
	rm -f banker
