CC=cc
CFLAGS=-Wall -Wextra -std=c11 -D_XOPEN_SOURCE=700 -g
OBJS=ls.o print.o sort.o

all: ls

ls: $(OBJS)
	$(CC) $(CFLAGS) -o ls $(OBJS)

ls.o: ls.c ls.h sort.h
	$(CC) $(CFLAGS) -c ls.c
print.o: print.c print.h ls.h
	$(CC) $(CFLAGS) -c print.c
sort.o: sort.c sort.h ls.h
	$(CC) $(CFLAGS) -c sort.c

clean:
	rm -f ls $(OBJS)

.PHONY: all clean
