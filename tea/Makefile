
ARMGNU = avr

COPS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding 
AOPS = --warn --fatal-warnings


OOPS = -std-compile-opts -strip-debug
LOPS = -Wall -m32 -emit-llvm
LLCOPS = -march=thumb 


all : gteatest.hex

vectors.o : vectors.s
	$(ARMGNU)-as vectors.s -o vectors.o

guart.o : uart.c
	$(ARMGNU)-gcc $(COPS) -c uart.c -o guart.o

gembedded.o : embedded.c
	$(ARMGNU)-gcc $(COPS) -c embedded.c -o gembedded.o

gteatest.o : teatest.c testdata.h
	$(ARMGNU)-gcc $(COPS) -c teatest.c -o gteatest.o

gteatest.hex : memmap vectors.o gembedded.o gteatest.o guart.o
	$(ARMGNU)-ld -T memmap vectors.o gembedded.o gteatest.o guart.o -o gteatest.elf
	$(ARMGNU)-objdump -D gteatest.elf > gteatest.list
	$(ARMGNU)-objcopy gteatest.elf -O ihex gteatest.hex

clean:
	rm -f *.hex
	rm -f *.o
	rm -f *.elf
	rm -f *.list

