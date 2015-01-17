CC=colorgcc
CFLAGS=-g -W -Wall -std=gnu99 -pedantic -I/usr/include/freetype2/
LDFLAGS=-lX11 -lXft -lXpm

all: microwm

microwm: main.o microwm.o
	$(CC) -o microwm main.o microwm.o $(LDFLAGS)

main.o: main.c
	$(CC) -c main.c $(CFLAGS)

microwm.o: microwm.c microwm.h
	$(CC) -c microwm.c $(CFLAGS)

clean:
	rm *.o
