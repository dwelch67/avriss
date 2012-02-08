
ARMGNU = avr

COPS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding 
AOPS = --warn --fatal-warnings

all : primes.hex host

host : host.c primes.c
	gcc host.c primes.c -o host

vectors.o : vectors.s
	$(ARMGNU)-as vectors.s -o vectors.o

uart.o : uart.c
	$(ARMGNU)-gcc $(COPS) -c uart.c -o uart.o

embedded.o : embedded.c
	$(ARMGNU)-gcc $(COPS) -c embedded.c -o embedded.o

primes.o : primes.c 
	$(ARMGNU)-gcc $(COPS) -c primes.c -o primes.o

primes.hex : memmap vectors.o embedded.o primes.o uart.o
	$(ARMGNU)-ld -T memmap vectors.o embedded.o primes.o uart.o -o primes.elf
	$(ARMGNU)-objdump -D primes.elf > primes.list
	$(ARMGNU)-objcopy primes.elf -O ihex primes.hex

clean:
	rm -f *.hex
	rm -f *.o
	rm -f *.elf
	rm -f *.list
	rm -f host

