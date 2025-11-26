all: run

run: menu.o
	@gcc -m32 -g -Wall -o main menu.o

countwords.o: count-words.c
	@gcc -g -Wall -m32 -c -o count-words.o count-words.c

addresses.o: addresses.c
	@gcc -g -Wall -m32 -c -o addresses.o addresses.c

base.o: base.c
	@gcc -g -Wall -m32 -c -o base.o base.c

menu.o: menu.c
	@gcc -g -Wall -m32 -c -o menu.o menu.c

.PHONY: clean

clean:
	@rm -f *.o main
