CC=gcc
name=mcu
gui=mcu_gui

all:
	$(CC) -O3 -o $(name) tests.c atmega328p.c
	$(CC) -O3 -o $(gui) mcu_gui.c atmega328p.c

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