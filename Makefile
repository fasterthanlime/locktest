
.PHONY: all

all: locktest

locktest: locktest.c
	gcc locktest.c -o locktest