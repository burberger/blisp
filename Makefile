all: blisp

blisp: prompt.c mpc.c mpc.h
	$(CC) -Wall -g -std=c99 -o blisp prompt.c mpc.c -ledit -lm
