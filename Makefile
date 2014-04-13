all: blisp

blisp: prompt.c mpc.c mpc.h lval.c lval.h
	$(CC) -Wall -g -std=c99 -o blisp prompt.c mpc.c lval.c -ledit -lm
