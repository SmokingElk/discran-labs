#include <iostream>
#include <algorithm>
#include <string>

int main() {
    std::string p = "";
    std::getline(std::cin, p, '\n');

    clock_t start = clock();        
    std::string t;
    std::getline(std::cin, t, '\n');

    std::cout << t.length() << " " << p.length() << std::endl;

    size_t pos = t.find(p);
    double timePassed = (double)(clock() - start) / CLOCKS_PER_SEC * 1000.0;
    printf("time: %fms\n", timePassed);
    
    return 0;
}
