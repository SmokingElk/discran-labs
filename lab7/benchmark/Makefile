all: straight.out dp.out

dp.out: dp.c
	gcc dp.c -o dp.out

straight.out: straight.c
	gcc straight.c -o straight.out

test_dp:
	./dp.out < ./test.txt

test_straight:
	./straight.out < ./test.txt