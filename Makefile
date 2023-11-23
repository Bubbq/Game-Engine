all: main.c
	gcc main.c -o test -lraylib -lm -Werror -Wall -std=c11 -pedantic
clean:
	rm test
