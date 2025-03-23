#include <stdio.h>
#include <stdlib.h>

void countSort(int *array, int n) {
    int min = array[0];
    int max = array[0];

    for (int i = 1; i < n; i++) {
        if (array[i] > max) max = array[i];
        if (array[i] < min) min = array[i];
    }

    int range = max - min + 1;

    int *counts = malloc(sizeof(int) * range);
    for (int i = 0; i < range; i++) counts[i] = 0;

    for (int i = 0; i < n; i++) counts[array[i] - min]++;

    for (int i = 1; i <= range; i++) counts[i] += counts[i - 1];

    int *res = malloc(sizeof(int) * n);

    for (int i = n - 1; i >= 0; i--) {
        res[--counts[array[i] - min]] = array[i];
    }

    for (int i = 0; i < n; i++) array[i] = res[i];
    
    free(counts);
    free(res);
}

int main() {
    int array[] = {3, 4, -1, 5, 2, -8, 2, 5, 5, -1};
    int n = sizeof(array) / sizeof(int);

    countSort(array, n);

    for (int i = 0; i < n; i++) printf("%d ", array[i]);
    printf("\n");
    return 0;
}
