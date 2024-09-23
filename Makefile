all: 
	gcc raycaster.c -o run -lraylib -lm -Werror -Wall -std=c11 -pedantic
clean:
	rm run
	clear
