
#include <iostream>
#include <set>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <lemon/list_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/bfs.h>

using namespace lemon;
int graph_size;

int set_degree_map(ListGraph& g, ListGraph::NodeMap<int>& degree) {
  graph_size = 0;
  for (ListGraph::NodeIt n(g); n != INVALID; ++n) {
    int count = 0;
    for (ListGraph::IncEdgeIt e(g, n); e != INVALID; ++e) {
      count++;
    }
    degree[n] = count;
    graph_size++;
  }
  return graph_size;
}

int cap_edges(ListGraph& g, int cap) {
  int total_rm = 0;
  for (ListGraph::NodeIt n(g); n != INVALID; ++n) {
    std::vector<ListGraph::Edge> edges;
    for (ListGraph::IncEdgeIt e(g, n); e != INVALID; ++e) {
      edges.push_back(e);
    }

    if (edges.size() > cap) {
      std::vector<ListGraph::Edge>::iterator it;
      int no_to_rm = edges.size() - cap;
      total_rm += no_to_rm;
      random_shuffle(edges.begin(), edges.end());

      for (it=edges.begin(); it != edges.end(); ++it) {
        g.erase(*it);
        if (--no_to_rm == 0) break;
      }
    }
  }
  return total_rm;
}

int get_bw_cost(ListGraph& g, ListGraph::Node source, int hops) {
  int counter = 0;
  std::set<ListGraph::Node> nodes;
  Bfs<ListGraph> bfs(g);
  bfs.init();
  bfs.addSource(source);
  while (!bfs.emptyQueue()) {
    ListGraph::Node v = bfs.processNextNode();
    if (bfs.dist(v) > hops-1) break;
    counter++;
    nodes.insert(v);
  }
  std::cout << nodes.size() << " " << counter << std::endl;
  return 0;
}

int bandwidth_cost(ListGraph& g, int hops, int visits) {
  static boost::random::mt19937 rng;
  static boost::random::uniform_int_distribution<> dist(0, graph_size);
  for (int i = 0; i < visits; ++i) {
    int idx = dist(rng);
    int counter = 0;
    ListGraph::Node source;
    for (ListGraph::NodeIt n(g); n != INVALID; ++n) {
      if (counter++ >= idx) {
        source = n;
        break;
      }
    }
    get_bw_cost(g, source, hops);
  }
  return 0;
}

ListGraph::Node get_random_node(ListGraph& g) {
  static boost::random::mt19937 rng;
  static boost::random::uniform_int_distribution<> dist(0, graph_size);
  ListGraph::Node node;
  int rand_int = dist(rng);
  int counter = 0;
  for (ListGraph::NodeIt n(g); n != INVALID; ++n) {
    if (counter >= rand_int) {
      node = n;
      break;
    }
    counter++;
  }
  return node;
}

int fill_edges(ListGraph& g, ListGraph::NodeMap<int>& degree, int cap, 
               int limit) {
  int total_added = 0;
  for (ListGraph::NodeIt u(g); u != INVALID; ++u) {
    int node_deg = degree[u];
    if (node_deg < cap) {
      int no_to_add = cap - node_deg;
      total_added += no_to_add;
      while (no_to_add > 0) {
        ListGraph::Node v = get_random_node(g);
        if (degree[v] < limit) {
          g.addEdge(u, v);
          no_to_add--;
        }
      }
    }
  }
  return total_added;  
}

int main (int argc, char* argv[]) {
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

