#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#define MAX_INPUT_LENGTH 30

typedef struct Pair {
    uint16_t key;
    uint64_t value;
} Pair;

Pair* countingSort (Pair *arr, int n) {
    if (n < 1) return NULL; 
    uint16_t min = arr[0].key, max = arr[0].key;

    for (int i = 1; i < n; i++) {
        if (arr[i].key < min) min = arr[i].key;
        if (arr[i].key > max) max = arr[i].key;
    }

    int range = max - min + 1;
    
    int *counts = malloc(sizeof(unsigned int) * range);
    for (int i = 0; i < range; i++) counts[i] = 0;
    
    for (int i = 0; i < n; i++) counts[arr[i].key - min]++;
    
    for (int i = 1; i < range; i++) counts[i] += counts[i - 1];

    Pair *res = malloc(sizeof(Pair) * n);

    for (int i = n - 1; i >= 0; i--) {
        res[--counts[arr[i].key - min]] = arr[i];
    }
    
    free(counts);

    return res;
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
    Pair *sorted = countingSort(arr, size);
    double timePassed = (double)(clock() - start) / CLOCKS_PER_SEC * 1000.0;
    
    for (int i = 0; i < size; i++) {
        printf("%hu\t%lu\n", sorted[i].key, sorted[i].value);
    }

    printf("time: %fms\n", timePassed);

    free(arr);
    free(sorted);
    
    return 0;
}
