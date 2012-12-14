
#include <iostream>
#include <lemon/list_graph.h>
#include <lemon/lgf_reader.h>

using namespace lemon;


int set_degree_map(ListGraph& g, ListGraph::NodeMap<int>& degree)
{
    for (ListGraph::NodeIt n(g); n != INVALID; ++n) {
        int count = 0;
        for (ListGraph::IncEdgeIt e(g, n); e != INVALID; ++e) count++;
        degree[n] = count;
        std::cout << count << "\n";
    }
    return 0;
}


int main (int argc, char* argv[])
{

    ListGraph g;
    ListGraph::NodeMap<int> label(g);
    ListGraph::NodeMap<int> degree(g);

    try {
        graphReader(g, argv[1]).nodeMap("label", label).run();
    } catch (Exception& error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return -1;
    }

    set_degree_map(g, degree);

    return 0;
}

