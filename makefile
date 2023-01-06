all: project1-starter.c
	gcc -g -Wall -o shell352 project1-starter.c

clean:
	$(RM) shell352