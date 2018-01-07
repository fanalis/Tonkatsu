CC = gcc
CFLAGS=-lcurl -lgumbo -Wall

all: clean libs main

main:
	$(CC) mangastream.o main.c -o mangastream_downloader $(CFLAGS)
libs:
	$(CC) -c mangastream.c

clean:
	rm -rf mangastream_downloader
	rm -rf *.o
