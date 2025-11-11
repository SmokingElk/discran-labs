#include <iostream>
#include <vector>
#include <string>

typedef std::vector<std::string> sequence;
typedef std::vector<int> lcs;

void forward(sequence &a, sequence &b, int sa, int sb, int n, int m, lcs &prev, lcs &curr, lcs &ans) {
    for (int i = 0; i < m + 1; i++) {
        prev[i] = 0;
    }

    for (int i = 1; i < n + 1; i++) {
        for (int j = 1; j < m + 1; j++) {
            if (a[sa + i - 1] == b[sb + j - 1]) {
                curr[j] = prev[j - 1] + 1;
            } else {
                curr[j] = std::max(curr[j - 1], prev[j]);
            }
        }

        std::swap(curr, prev);
    }

    for (int i = 0; i < m + 1; i++) ans[i] = prev[i];
}

void backward(sequence &a, sequence &b, int sa, int sb, int n, int m, lcs &prev, lcs &curr, lcs &ans) {
    for (int i = 0; i < m + 1; i++) {
        prev[i] = 0;
    }

    for (int i = 1; i < n + 1; i++) {
        for (int j = 1; j < m + 1; j++) {
            if (a[sa + n - 1 - (i - 1)] == b[sb + m - 1 - (j - 1)]) {
                curr[j] = prev[j - 1] + 1;
            } else {
                curr[j] = std::max(curr[j - 1], prev[j]);
            }
        }

        std::swap(curr, prev);
    }

    for (int i = 0; i < m + 1; i++) ans[m - i] = prev[i];
}

sequence hirshberg(sequence &a, sequence &b, int sa, int sb, int n, int m, lcs &prev, lcs &curr, lcs &ansF, lcs &ansB) {
    if (n == 0) {
        return sequence(0);
    } 

    if (n == 1) {
        for (int i = 0; i < m; i++) {
            if (a[sa] == b[sb + i]) {
                return sequence{a[sa]};
            }
        }

        return sequence(0);
    }

    int mid = sa + n / 2;

    forward(a, b, sa, sb, n / 2, m, prev, curr, ansF);
    backward(a, b, mid, sb, n - n / 2, m, prev, curr, ansB);

    int split = -1;
    int max = -1;

    for (int i = 0; i < m+1; i++) {
        int newMax = ansF[i] + ansB[i];

        if (newMax > max) {
            max = newMax;
            split = i;
        }
    }

    sequence left = hirshberg(a, b, sa, sb, n / 2, split, prev, curr, ansF, ansB);
    sequence right = hirshberg(a, b, mid, sb + split, n - n / 2, m - split, prev, curr, ansF, ansB);

    left.insert(left.end(), right.begin(), right.end());

    return left;
}

int main() {
    std::vector<sequence> sequences(2);

    int seq = 0;

    std::string word;
    while (std::cin >> word) {
        sequences[seq].push_back(word);
        if (std::cin.peek() == '\n') seq = 1;
    }

    size_t n = sequences[0].size();
    size_t m = sequences[1].size();

    lcs prev(m + 1);
    lcs curr(m + 1);
    lcs ansF(m + 1);
    lcs ansB(m + 1);

    sequence res = hirshberg(sequences[0], sequences[1], 0, 0, n, m, prev, curr, ansF, ansB);
    size_t len = res.size();

    std::cout << len << std::endl;

    for (int i = 0; i < len; i++) {
        std::cout << res[i];

        if (i < len - 1) std::cout << " ";
    }

    std::cout << std::endl;
    
    return 0;
}
