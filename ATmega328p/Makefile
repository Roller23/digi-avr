CC=gcc
name=mcu
gui=mcu_gui
AVR_CC=avr-gcc
AVR_flags=-Wall -Wextra -Os -mmcu=atmega328p

all:
	$(CC) -O3 -o $(name) tests.c atmega328p.c -lm
	$(CC) -O3 -pthread -o $(gui) mcu_gui.c atmega328p.c -lm

program:
	$(AVR_CC) $(AVR_flags) -o program.bin program.c
	avr-objcopy -j .text -j .data -O ihex program.bin program.hex
	rm program.bin

shared:
	$(CC) -O3 -fPIC -shared -o mcu_shared.so atmega328p.c -lm -D SHARED

disasm:
	avr-objdump -m avr -D program.hex

debug:
	$(CC) -O3 -g -o $(name) tests.c atmega328p.c
	lldb ./$(name)

run:
	./$(name)

gui:
	./$(gui)