all: blisp

blisp: prompt.c
	$(CC) -Wall -g -std=c99 -o blisp prompt.c
