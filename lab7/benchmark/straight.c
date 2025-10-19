#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main() {
    uint64_t n;
    uint32_t m;    

    scanf("%ld %d", &n, &m);

    clock_t start = clock(); 

    size_t cnt = 0;
    char nStr[22];
    sprintf(nStr, "%ld", n);

    for (size_t i = m; i < n; i += m) {
        char iStr[22];
        sprintf(iStr, "%ld", i);

        if (strcmp(iStr, nStr) < 0) cnt++; 
    }

    double timePassed = (double)(clock() - start) / CLOCKS_PER_SEC * 1000.0;
    printf("time: %fms\n", timePassed);

    printf("%ld\n", cnt);

    return 0;
}
