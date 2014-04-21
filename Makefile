all: lval mpc blisp

lval: lval.c lval.h
	$(CC) -Wall -g -std=c99 -c lval.c

mpc: mpc.c mpc.h
	$(CC) -Wall -g -std=c99 -c mpc.c

blisp: prompt.c mpc.o lval.o
	$(CC) -Wall -g -std=c99 -o blisp prompt.c mpc.o lval.o -ledit -lm
