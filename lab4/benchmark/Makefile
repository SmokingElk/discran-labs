all: find.out kmp.out

kmp.out: kmp.c
	gcc kmp.c -o kmp.out

find.out: find.cpp
	g++ find.cpp -o find.out

test_kmp:
	./kmp.out < ./in.txt | grep "time"

test_find:
	./find.out < ./in.txt | grep "time"