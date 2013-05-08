#!/usr/bin/make -f
CC=gcc
CFLAGS+=-O2 -Wall
LDLIBS+=-lsqlite3

default: 4images

all: 4images fr.db en.db

en.db: 4images dic_en.txt
	./4images init en.db dic_en.txt

fr.db: 4images dic_fr.txt
	./4images init fr.db dic_fr.txt

.PHONY: clean

clean:
	rm -f ./4images ./fr.db ./en.db
