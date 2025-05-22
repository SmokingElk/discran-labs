all: build run

build: main

main: main.c
	gcc -fprofile-arcs -ftest-coverage -fkeep-inline-functions -fkeep-static-functions -O0 -g -pg main.c -o main

run:
	./main < in.txt > /dev/null

valgrind: 
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./main < in.txt > /dev/null

gprof:
	gprof ./main gmon.out > report.txt

clean:
	rm *.out