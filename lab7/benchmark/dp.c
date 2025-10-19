#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>

uint64_t inRange(uint64_t from, uint64_t to, uint64_t m) {
    uint64_t lower = (from / m) * m;
    if (from % m != 0) lower += m;

    uint64_t upper = ((to - 1) / m) * m;

    if (upper < lower) return 0;

    return (upper - lower) / m + 1;
}

int main() {
    char nStr[22];
    uint32_t m;    

    scanf("%s %d", nStr, &m);
    int length = strlen(nStr);

    clock_t start = clock(); 

    uint64_t dp[22];

    uint64_t before;
    uint64_t from = 1;
    uint64_t to = nStr[0] - '0';

    dp[0] = inRange(from, to, m);

    for (int i = 1; i < length; i++) {
        from *= 10;

        before = to;
        to = to * 10 + (nStr[i] - '0');
        
        dp[i] = dp[i - 1] + inRange(from, to, m);
        if (before % m == 0) dp[i]++;
    }

    printf("%ld\n", dp[length - 1]);

    double timePassed = (double)(clock() - start) / CLOCKS_PER_SEC * 1000.0;
    printf("time: %fms\n", timePassed);

    return 0;
}
