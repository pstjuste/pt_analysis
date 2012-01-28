
#include <iostream>
#include <set>

int main(int argc, char* argv[])
{
    std::set<unsigned long> nodes;

    unsigned long u, v, t;
    while (std::cin.good()) {
        std::cin >> u >> v;

        t = (u << 32) + v;

        if (nodes.find(t) != nodes.end()) {
            std::cout << u << " " << v << "\n";
        }
        else {
            t = (v << 32) + u;
            nodes.insert(t);
        }
    }
    std::cerr << "set size " << nodes.size() << std::endl;
    return 0;
}

