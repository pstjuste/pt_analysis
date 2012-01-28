
#include <iostream>
#include <map>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    int in = atoi(argv[1]);
    std::map<int,int> nodes;

    int u, v;
    while (std::cin.good()) {
        if (in > 1)  std::cin >> u >> v;
        else std::cin >> v >> u;

        if (nodes.find(u) == nodes.end()) {
            nodes[u] = 1;
        }
        else nodes[u] = nodes[u] + 1;
    }

    std::map<int,int>::iterator it;
    for (it = nodes.begin(); it != nodes.end(); ++it) {
        std::cout << it->second << "\n";
    }
    return 0;
}
