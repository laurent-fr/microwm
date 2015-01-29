#CC=clang
CC=colorgcc
#CC=gcc

CFLAGS= -g -W -Wall -std=gnu99 -pedantic
CFLAGS+= $(shell pkg-config --cflags xft)
CFLAGS+= $(shell pkg-config --cflags xpm)
CFLAGS+= $(shell pkg-config --cflags x11)

LDFLAGS=$(shell pkg-config --libs x11)
LDFLAGS+=$(shell pkg-config --libs xft)
LDFLAGS+=$(shell pkg-config --libs xpm)

all: microwm

microwm: main.o microwm.o widgets.o icccm.o
	$(CC) -o microwm widgets.o icccm.o microwm.o main.o $(LDFLAGS)

main.o: main.c
	$(CC) -c main.c $(CFLAGS)

microwm.o: microwm.c microwm.h
	$(CC) -c microwm.c $(CFLAGS)

widgets.o: widgets.c widgets.h
	$(CC) -c widgets.c $(CFLAGS)

icccm.o: icccm.c icccm.h
	$(CC) -c icccm.c $(CFLAGS)

clean:
	rm *.o
