all:mypipeline run looper

run: myshell.o lineParser.o mypipeline.o
	gcc -m32 -g -Wall -o myshell myshell.o lineParser.o

myshell.o: myshell.c
	gcc -g -m32 -Wall -c -o myshell.o myshell.c

lineParser.o: lineParser.c
	gcc -g -m32 -Wall -c -o lineParser.o lineParser.c

mypipeline: mypipeline.o
	gcc -g -m32 -Wall -o mypipeline mypipeline.o

mypipeline.o: mypipeline.c
	gcc -g -m32 -Wall -c -o mypipeline.o mypipeline.c

looper: looper.o
	gcc -g -m32 -Wall -o looper  looper.o

looper.o: looper.c
	gcc -g -m32 -Wall -c -o looper.o looper.c	

clean:
	@rm -f *.o mypipeline
