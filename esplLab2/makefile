all: myshell mypipe looper

myshell: myshell.c LineParser.c
	gcc myshell.c LineParser.c -o myshell

mypipe: mypipe.c
	gcc mypipe.c -o mypipe

looper: looper.c
	gcc looper.c -o looper

clean:
	rm -f myshell mypipe looper
