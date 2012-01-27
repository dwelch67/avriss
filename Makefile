
all : avriss avrdiss

avrdiss : avrdiss.c
	gcc -O2 avrdiss.c -o avrdiss


avriss : avriss.c
	gcc -O2 avriss.c -o avriss

clean :
	rm -f avriss
	rm -f avrdiss


