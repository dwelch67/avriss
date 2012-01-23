
all : simavr dissavr

dissavr : dissavr.c
	gcc -O2 dissavr.c -o dissavr


simavr : simavr.c
	gcc -O2 simavr.c -o simavr

clean :
	rm -f simavr
	rm -f dissavr


