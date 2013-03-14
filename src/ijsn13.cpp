
#include <iostream>
#include <set>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/exponential_distribution.hpp>
#include <boost/shared_ptr.hpp>

#include <lemon/list_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/bfs.h>

using namespace lemon;
int graph_size;
static boost::random::mt19937 rng_;

/*
 * These functions will see the light of day in future work
 * 
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

ListGraph::Node get_random_node(ListGraph& g) {
  ListGraph::Node node;
  int rand_int = dist_(rng_);
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
*/

/*==========================Timing Analysis=================================*/
const int LIMIT = 7 * 24 * 12;
const int OFFSET_RANGE = 12 * 24;

struct Interval {
  int start_time;
  int end_time;
};

typedef boost::shared_ptr<Interval> IntervalPtr;
typedef boost::shared_ptr<std::vector<IntervalPtr> > ListPtr;

struct State {
  std::map<ListGraph::Node,ListPtr> on_schedule;
  std::map<ListGraph::Node,ListPtr> off_schedule;
  std::map<ListGraph::Node,int> visited;
  int on_mean, off_mean;
};

int set_schedule(State& state, ListGraph::Node node, int limit) {
  ListPtr on_times(new std::vector<IntervalPtr>);
  ListPtr off_times(new std::vector<IntervalPtr>);
  boost::random::uniform_int_distribution<> dist(0, OFFSET_RANGE);
  boost::random::exponential_distribution<> on_expon(1/state.on_mean);
  boost::random::exponential_distribution<> off_expon(1/state.off_mean);
  int current_time = dist(rng_);

  while (current_time < limit) {
    int on_interval = static_cast<int>(on_expon(rng_));
    IntervalPtr interval_on(new Interval);
    interval_on->start_time = current_time;
    interval_on->end_time = current_time + on_interval;
    on_times->push_back(interval_on);
    current_time += (on_interval + 1);

    int off_interval = static_cast<int>(off_expon(rng_));
    IntervalPtr interval_off(new Interval);
    interval_off->start_time = current_time;
    interval_off->end_time = current_time + on_interval;
    off_times->push_back(interval_off);
    current_time += (off_interval + 1);
  }
  state.on_schedule[node] = on_times;
  state.off_schedule[node] = off_times;
  return 0;
}

/*==========================Bandwidth Analysis==============================*/
class DegreeMap {
 public:
  DegreeMap(ListGraph::NodeMap<int>& degree) : degree_(degree) {}

  int init(ListGraph& g) {
    graph_size = 0;
    for (ListGraph::NodeIt n(g); n != INVALID; ++n) {
      int count = 0;
      for (ListGraph::IncEdgeIt e(g, n); e != INVALID; ++e) {
        count++;
      }
      degree_[n] = count;
      graph_size++;
    }
    return graph_size;
  }

  bool operator() (ListGraph::Node a, ListGraph::Node b) {
    return (degree_[b] < degree_[a]);
  }

  int degree(ListGraph::Node node) { return degree_[node]; }

 private:
  ListGraph::NodeMap<int>& degree_;

};

int get_bw_cost(ListGraph& g, ListGraph::NodeMap<int>& degree,
                ListGraph::Node& source, int hops) {
  int counter = 0;
  std::set<ListGraph::Node> nodes;

  Bfs<ListGraph> bfs(g);
  bfs.init();
  bfs.addSource(source);
  while (!bfs.emptyQueue()) {
    ListGraph::Node v = bfs.processNextNode();
    if (bfs.dist(v) > hops) {
      break;
    }
    nodes.insert(v);
    if (bfs.dist(v) < hops) {
      counter += degree[v];
    }
  }
  int no_friends = nodes.size() - 1;
  std::cout << no_friends << " " << counter - no_friends << std::endl;
  return 0;
}

int bandwidth_cost(ListGraph& g, ListGraph::NodeMap<int>& degree, int hops,
                   int visits) {
  boost::random::uniform_int_distribution<> dist(0, graph_size);
  for (int i = 0; i < visits; ++i) {
    int idx = dist(rng_);
    int counter = 0;
    ListGraph::Node source;
    for (ListGraph::NodeIt n(g); n != INVALID; ++n) {
      if (counter++ >= idx) {
        source = n;
        break;
      }
    }
    get_bw_cost(g, degree, source, hops);
  }
  return 0;
}

/*==========================Replication Analysis============================*/

ListGraph::Node get_high_neighbor(ListGraph& g, DegreeMap& degree_map, 
    ListGraph::Node& node, std::map<ListGraph::Node,int>& visited) {
  ListGraph::Node neighbor;
  std::vector<ListGraph::Node> nodes;
  for (ListGraph::IncEdgeIt e(g, node); e != INVALID; ++e) {
    nodes.push_back(g.oppositeNode(node, e));
  }
  std::sort(nodes.begin(), nodes.end(), degree_map);
  std::vector<ListGraph::Node>::iterator it;
  for (it = nodes.begin(); it != nodes.end(); it++) {
    if (visited[*it] == 0) {
      neighbor = *it;
      break;
    }
  }
  return neighbor;
}

ListGraph::Node get_rand_neighbor(ListGraph& g, DegreeMap& degree_map,
    ListGraph::Node& node, std::map<ListGraph::Node,int>& visited) {
  ListGraph::Node neighbor;
  std::vector<ListGraph::Node> nodes;
  for (ListGraph::IncEdgeIt e(g, node); e != INVALID; ++e) {
    nodes.push_back(g.oppositeNode(node, e));
  }
  boost::random::uniform_int_distribution<> dist(0, nodes.size()-1);
  neighbor = nodes[dist(rng_)];
  return neighbor;
}

std::map<ListGraph::Node,int>& do_random_walk(ListGraph& g,
    DegreeMap& degree_map, int ttl, int wtype,
    std::map<ListGraph::Node,int>& visited) {
  boost::random::uniform_int_distribution<> dist(0, graph_size);
  int idx = dist(rng_);
  ListGraph::Node node;
  for (ListGraph::NodeIt n(g); n != INVALID; ++n) {
    idx--;
    if (idx <= 0) {
      node = n;
      break;
    }
  }

  ListGraph::Node next_node = node;
  for (int i = 1; i <= ttl; i++) {
    if (wtype == 1) {
      next_node = get_high_neighbor(g, degree_map, next_node, visited);
    }
    else if (wtype == 2) {
      next_node = get_rand_neighbor(g, degree_map, next_node, visited);
    }
    if (!g.valid(next_node)) {
      break;
    }
    visited[next_node] = i;
  }
  return visited;
}

int random_walk(ListGraph& g, DegreeMap& degree_map, int ttl, int wtype,
                int walks) {
  for (int i = 0; i < walks; i++) {
    std::map<ListGraph::Node,int> put_nodes;
    std::map<ListGraph::Node,int> get_nodes;
    do_random_walk(g, degree_map, ttl, wtype, put_nodes);
    do_random_walk(g, degree_map, ttl, wtype, get_nodes);
    std::cout << get_nodes.size() << " " << put_nodes.size() << " ";

    std::map<ListGraph::Node,int>::iterator it;
    for (it = get_nodes.begin(); it != get_nodes.end(); it++) {
      int tmp = put_nodes[it->first];
      if (tmp > 0) {
        std::cout << it->second << " " << tmp << " h";
        break;
      }
    }
    std::cout << std::endl;
  }
  return 0;
}

int main (int argc, char* argv[]) {
  ListGraph g;
  ListGraph::NodeMap<int> label(g);
  ListGraph::NodeMap<int> degree(g);
  DegreeMap degree_map(degree);

  char msg[] = "help: ijsn13 gfile hops visits on_mean off_mean tries "
               "ttl wtype walks";

  if (argc < 10) {
    std::cout << msg << std::endl;
    return -1;
  }

  std::cout << "node " << sizeof(ListGraph::Node) << std::endl;
  std::cout << "int " << sizeof(int) << std::endl;

  try {
    graphReader(g, argv[1]).nodeMap("label", label).run();
  } catch (Exception& error) {
    std::cerr << "Error: " << error.what() << std::endl;
    return -1;
  }

  int hops = atoi(argv[2]);
  int visits = atoi(argv[3]);
  int on_mean = atoi(argv[4]);
  int off_mean = atoi(argv[5]);
  int tries = atoi(argv[6]);
  int ttl = atoi(argv[7]);
  int wtype = atoi(argv[8]);
  int walks = atoi(argv[9]);

  degree_map.init(g);
  bandwidth_cost(g, degree, hops, visits);
  //publish_time(g, on_mean_off_mean, tries);
  random_walk(g, degree_map, ttl, wtype, walks);
  return 0;
}

