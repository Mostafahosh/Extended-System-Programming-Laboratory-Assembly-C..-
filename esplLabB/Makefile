all: run

run: hexaPrint.o
	@gcc -m32 -g -Wall -o hexaPrint hexaPrint.o

Bubblesort.o: Bubblesort.c
	@gcc -g -Wall -m32 -c -o Bubblesort.o Bubblesort.c

hexaPrint.o: hexaPrint.c
	@gcc -g -Wall -m32 -c -o hexaPrint.o hexaPrint.c

.PHONY: clean

clean:
	@rm -f *.o main