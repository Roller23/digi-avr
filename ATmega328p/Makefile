CC=gcc
name=mcu
gui=mcu_gui
AVR_CC=avr-gcc
AVR_flags=-Wall -Wextra -g -Os -mmcu=atmega328p

all:
	$(CC) -O3 -o $(name) tests.c atmega328p.c
	$(CC) -O3 -o $(gui) mcu_gui.c atmega328p.c

program:
	$(AVR_CC) $(AVR_flags) -o program.bin program.c
	avr-objcopy -j .text -j .data -O ihex program.bin program.hex
	rm program.bin

debug:
	$(CC) -O0 -g -o $(name) tests.c atmega328p.c
	gdb ./$(name)

update:
	git stash
	git pull
	git stash apply

push:
	make update
	git add .
	git commit -m "Automated makefile commit"
	git push

test:
	./$(name)

gui:
	./$(gui)