all: encoder

encoder: encoder.o
	@gcc -m32 -g -Wall -o encoder encoder.o

encoder.o: encoder.c
	@gcc -g -Wall -m32 -c -o encoder.o encoder.c
# edit: main.o numbers.o add.o
# 	@gcc -m32 -g -Wall -o edit main.o numbers.o add.o

# main.o: main.c
# 	@gcc -g -Wall -m32 -c -o main.o main.c

# numbers.o: numbers.c
# 	@gcc -g -Wall -m32 -c -o numbers.o numbers.c

# add.o: add.s
# 	@nasm -g -f elf32 -w+all -o add.o add.s


.PHONY: clean

clean:
	@rm -f *.o edit
	