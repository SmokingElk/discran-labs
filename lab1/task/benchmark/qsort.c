#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#define MAX_INPUT_LENGTH 30

typedef struct Pair {
    uint16_t key;
    uint64_t value;
} Pair;

int cmp (const void * a, const void * b) {
    Pair *pairA = (Pair*)a;
    Pair *pairB = (Pair*)b;
    if (pairA->key > pairB->key) return 1;
    if (pairA->key < pairB->key) return -1;
    return 0;
}

int main () {
    Pair *arr = NULL;
    int capacity = 0;
    int size = 0;

    char str[MAX_INPUT_LENGTH];

    while (fgets(str, MAX_INPUT_LENGTH, stdin)) {
        if (str[0] == '\n' || str[0] == '\0') continue;

        if (size >= capacity) {
            capacity += 10;
            arr = realloc(arr, sizeof(Pair) * capacity);
        }

        int scanRes = sscanf(str, "%hu %lu", &(arr[size].key), &(arr[size].value)); 
        if (scanRes == 2) size++;
    }

    clock_t start = clock();
    qsort(arr, size, sizeof(Pair), cmp);
    double timePassed = (double)(clock() - start) / CLOCKS_PER_SEC * 1000.0;
    
    for (int i = 0; i < size; i++) {
        printf("%hu\t%lu\n", arr[i].key, arr[i].value);
    }

    printf("time: %fms\n", timePassed);

    free(arr);
    
    return 0;
}
