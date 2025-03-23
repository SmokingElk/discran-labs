#include <stdio.h>
#include <stdlib.h>

void __merge(int *arr, int *tmp, int l, int mid, int r) {
    for (int i = l; i < r; i++) {
        tmp[i] = arr[i];
    } 

    int i = l;
    int j = mid;
    int k = l;

    while (i < mid && j < r) {
        if (tmp[i] < tmp[j]) {
            arr[k++] = tmp[i++];
        } else {
            arr[k++] = tmp[j++];
        }
    }
    
    while (i < mid) {
        arr[k++] = tmp[i++];
    }

    while (j < r) {
        arr[k++] = tmp[j++];
    }
}

void __mergeSort(int *arr, int *tmp, int l, int r) {
    if (r - l < 2) return;

    int mid = (l + r) / 2;

    __mergeSort(arr, tmp, l, mid);
    __mergeSort(arr, tmp, mid, r);

    __merge(arr, tmp, l, mid, r);
}

void mergeSort(int *arr, int n) {
    int *tmp = malloc(sizeof(int) * n);
    __mergeSort(arr, tmp, 0, n);
    free(tmp);
}

int main() {
    int arr[] = {3, 14, 1, 5, 9, 2, 6, 5, 11, 4};
    int n = sizeof(arr) / sizeof(int);    

    mergeSort(arr, n);

    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");
    return 0;
}
