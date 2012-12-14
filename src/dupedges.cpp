
#include <iostream>
#include <set>

int main(int argc, char* argv[])
{
    std::set<unsigned long> edges;

    unsigned long u, v, t;
    while (std::cin.good()) {
        std::cin >> u >> v;

        t = (u << 32) + v;

        if (edges.find(t) != edges.end()) {
            // duplicate edge
            std::cout << "F " << u << " " << v << "\n";
        }
        else {
            t = (v << 32) + u;
            edges.insert(t);
            // unique edge
            std::cout << "N " << u << " " << v << "\n";
        }
    }
    std::cerr << "no of edges " << edges.size() << std::endl;
    return 0;
}

