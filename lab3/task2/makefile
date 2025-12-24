all: task2

task2: start.o main.o util.o
	ld -m elf_i386 start.o main.o util.o -o task2

start.o: start.s
	nasm -f elf32 start.s -o start.o

main.o: main.c util.h
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o

util.o: util.c util.h
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

clean:
	rm -f *.o task2
