#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

int main() {
    std::string t = "";
    std::getline(std::cin, t, '\n');

    std::string p = "";
    std::getline(std::cin, p, '\n');
    
    std::cout << t.length() << " " << p.length() << std::endl;
    
    clock_t start = clock();        
    std::vector<size_t> positions;
    size_t pos = t.find(p);

    std::cout << "lol" << std::endl;
    
    while (pos != std::string::npos) {
        positions.push_back(pos);
        pos = t.find(p, pos + 1);
        std::cout << pos << std::endl;
    }
    double timePassed = (double)(clock() - start) / CLOCKS_PER_SEC * 1000.0;
    std::cout << "time: " << timePassed << "ms\n";
    
    return 0;
}
