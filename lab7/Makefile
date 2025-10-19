all: build run

test: build
	./app.out < test.txt

build: app.out

app.out: main.c
	gcc main.c -o app.out

run:
	./app.out