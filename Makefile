#CC=clang
CC=colorgcc
#CC=gcc

CFLAGS= -g -W -Wall -Werror -std=c11 -pedantic -D_GNU_SOURCE -DDEBUG
CFLAGS+= $(shell pkg-config --cflags xft)
CFLAGS+= $(shell pkg-config --cflags xpm)
CFLAGS+= $(shell pkg-config --cflags x11)

LDFLAGS=$(shell pkg-config --libs x11)
LDFLAGS+=$(shell pkg-config --libs xft)
LDFLAGS+=$(shell pkg-config --libs xpm)

all: microwm

microwm: main.o microwm.o widgets.o icccm.o config.o
	$(CC) -o microwm widgets.o icccm.o microwm.o config.o main.o $(LDFLAGS)

main.o: main.c
	$(CC) -c main.c $(CFLAGS)

microwm.o: microwm.c microwm.h
	$(CC) -c microwm.c $(CFLAGS)

widgets.o: widgets.c widgets.h
	$(CC) -c widgets.c $(CFLAGS)

icccm.o: icccm.c icccm.h
	$(CC) -c icccm.c $(CFLAGS)

config.o: config.c config.h
	$(CC) -c config.c $(CFLAGS)

clean:
	rm *.o
