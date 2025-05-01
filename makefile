CC = gcc
CFLAGS = -g `pkg-config --cflags --libs gtk4`


all: main.o window.o parser.o -lxml -lzip
	gcc window.o parser.o main.o -I./xml.c/src -L./xml.c/build -lxml -lzip `pkg-config --cflags --libs gtk4` -o epub

main.o: main.c
	gcc -c main.c $(CFLAGS)

window.o: window.c
	gcc -c window.c $(CFLAGS)

parser.o: parser.c
	gcc -c parser.c $(CFLAGS)

-lxml:
	$(MAKE) -C ./xml.c/build/
