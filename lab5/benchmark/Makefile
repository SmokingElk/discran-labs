all: find.out suffix_tree.out

suffix_tree.out: suffix_tree.c
	gcc suffix_tree.c -o suffix_tree.out

find.out: find.cpp
	g++ find.cpp -o find.out

test_suffix_tree:
	./suffix_tree.out < ./test.txt | grep "time"

test_find:
	./find.out < ./test.txt | grep "time"