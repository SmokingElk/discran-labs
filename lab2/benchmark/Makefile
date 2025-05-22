all: rb.out map.out

rb.out: rb.c
	gcc rb.c -o rb.out

map.out: map.cpp
	g++ map.cpp -o map.out

test_rb:
	./rb.out < ./in.txt | grep "time"

test_map:
	./map.out < ./in.txt | grep "time"