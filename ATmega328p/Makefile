CC=gcc
name=mcu

all:
	$(CC) -o $(name) tests.c atmega328p.c

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