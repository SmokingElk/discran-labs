all: countingSort.out qsort.out

countingSort.out: countingSort.c
	gcc countingSort.c -o countingSort.out

qsort.out: qsort.c
	gcc qsort.c -o qsort.out

test_counting:
	./countingSort.out < ./in.txt | grep "time"

test_quick:
	./qsort.out < ./in.txt | grep "time"