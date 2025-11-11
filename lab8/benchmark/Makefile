all: straight.out topological_sort.out

topological_sort.out: topological_sort.c
	gcc topological_sort.c -o topological_sort.out

straight.out: straight.c
	gcc straight.c -o straight.out

test_topological_sort:
	./topological_sort.out < ./test.txt

test_straight:
	./straight.out < ./test.txt