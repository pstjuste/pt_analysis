
#include <iostream>
#include <set>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    int size = atoi(argv[1]);
    std::set<int> nodes;

    int start = atoi(argv[2]);
    int count = 0;

    int u, v;
    while (std::cin.good()) {
        count += 1;
        if (count <= start) continue;

        std::cin >> u >> v;

        if (nodes.size() < size) {
            nodes.insert(u);
            nodes.insert(v);
            std::cout << u << " " << v << "\n";
        }
        else if (nodes.find(u) != nodes.end() && 
                 nodes.find(v) != nodes.end()) {
            std::cout << u << " " << v << "\n";
        }

    }
    std::cerr << "set size " << nodes.size() << std::endl;
    return 0;
}
