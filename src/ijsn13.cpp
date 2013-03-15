
#include <iostream>
#include <set>
#include <algorithm>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/exponential_distribution.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <lemon/list_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/bfs.h>

using namespace lemon;
int graph_size;
static boost::random::mt19937 rng_;

const int LIMIT = 7 * 24 * 12;
const int OFFSET_RANGE = 12 * 24;

struct Interval {
  int start_time;
  int end_time;
};

typedef boost::shared_ptr<Interval> IntervalPtr;
typedef boost::shared_ptr<std::vector<IntervalPtr> > ListPtr;
typedef std::vector<IntervalPtr>::iterator ListPtrIterator;

struct State {
  std::map<ListGraph::Node,ListPtr> on_schedule;
  std::map<ListGraph::Node,ListPtr> off_schedule;
  std::map<ListGraph::Node,int> visited;
  std::map<ListGraph::Node,int> nodes;
  int on_mean, off_mean;
};

ListGraph::Node get_random_node(ListGraph& g) {
  ListGraph::Node node;
  boost::random::uniform_int_distribution<> dist(0, graph_size);
  int idx = dist(rng_);
  for (ListGraph::NodeIt n(g); n != INVALID; ++n) {
    if (idx-- <= 0) {
      node = n;
      break;
    }
  }
  return node;
}

int cap_edges(ListGraph& g, int cap) {
  if (cap == 0) { return 0; }
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

/*==========================Timing Analysis=================================*/
int set_schedule(State& state, ListGraph::Node node, int limit) {
  ListPtr on_times(new std::vector<IntervalPtr>);
  ListPtr off_times(new std::vector<IntervalPtr>);
  boost::random::uniform_int_distribution<> dist(0, OFFSET_RANGE);
  boost::random::exponential_distribution<> on_expon(1.0/state.on_mean);
  boost::random::exponential_distribution<> off_expon(1.0/state.off_mean);
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

int find_overlap(State& state, ListGraph::Node source, ListGraph::Node target,
                 int limit) {
  if (state.on_schedule.find(source) == state.on_schedule.end()) {
    set_schedule(state, source, limit);
  }
  if (state.on_schedule.find(target) == state.on_schedule.end()) {
    set_schedule(state, target, limit);
  }

  int found = 0;
  ListPtr& stimes = state.on_schedule[source];
  ListPtr& ttimes = state.on_schedule[target];
  int old_time = limit;
  if (state.visited.find(target) != state.visited.end()) {
    old_time = state.visited[target];
  }
  if (old_time <= state.visited[source]) {
    return found;
  }
  for (ListPtrIterator it = stimes->begin(); it != stimes->end(); it++) {
    if (found == 1) {
      break;
    }
    for (ListPtrIterator itt = ttimes->begin(); itt != ttimes->end(); itt++) {
      if ((!((*itt)->end_time <= (*it)->start_time) || 
          ((*itt)->start_time >= (*it)->end_time)) &&
          ((*it)->end_time >= state.visited[source])) {
        int new_time = std::max((*it)->start_time,
                       std::max((*itt)->start_time,
                       state.visited[source]));
        if (new_time < old_time) {
          state.visited[target] = std::min(old_time, new_time);
          found = 1;
        }
        break;
      }
    }
  }
  return found;
}

int set_message_time(State& state, ListGraph::Node node) {
  set_schedule(state, node, LIMIT);
  ListPtr list = state.on_schedule[node];
  IntervalPtr interval = (*list.get())[0];
  int range = interval->end_time - interval->start_time;
  boost::random::uniform_int_distribution<> dist(1, range);
  state.visited[node] = dist(rng_) + interval->start_time;
  return 0;
}

int get_pub_time(ListGraph& g, State& state, ListGraph::Node source) {
  state.nodes[source] = 0;
  set_message_time(state, source);
  for (ListGraph::IncEdgeIt e(g, source); e != INVALID; ++e) {
    ListGraph::Node node = g.oppositeNode(source, e);
    state.nodes[node] = 1;
    find_overlap(state, source, node, LIMIT);
  }

  while (true) {
    int counter = 0;
    std::map<ListGraph::Node,int>::iterator it;
    for (it = state.nodes.begin(); it != state.nodes.end(); it++) {
      ListGraph::Node node = it->first;
      if (it->second == 1) {
        for (ListGraph::IncEdgeIt e(g, node); e != INVALID; ++e) {
          ListGraph::Node foaf = g.oppositeNode(node, e);
          if (state.nodes.find(foaf) == state.nodes.end()) {
            state.nodes[foaf] = 2;
          }
          if (state.visited.find(node) != state.visited.end()) {
            counter += find_overlap(state, node, foaf, LIMIT);
          }
        }
      }
    }
    if (counter == 0) { break; }
  }

  for (int i = 2; i < 4; i++) {
    std::map<ListGraph::Node,int>::iterator it;
    for (it = state.nodes.begin(); it != state.nodes.end(); it++) {
      ListGraph::Node node = it->first;
      if (it->second == i) {
        for (ListGraph::IncEdgeIt e(g, node); e != INVALID; ++e) {
          ListGraph::Node foaf = g.oppositeNode(node, e);
          if (state.nodes.find(foaf) == state.nodes.end()) {
            state.nodes[foaf] = i + 1;
          }
          if (state.visited.find(node) != state.visited.end()) {
            find_overlap(state, node, foaf, LIMIT);
          }
        }
      }
    }
  }
  return 0;
}

int publish_time(ListGraph& g, ListGraph::NodeMap<int>& degree, int on_mean,
                 int off_mean, int tries) {
  for (int i = 0; i < tries; i++) {
    boost::scoped_ptr<State> state_ptr(new State);
    state_ptr->on_mean = on_mean;
    state_ptr->off_mean = off_mean;
    ListGraph::Node source = get_random_node(g);
    get_pub_time(g, *(state_ptr.get()), source);
    std::cout << "stats " << degree[source] << "\n";
    std::map<ListGraph::Node,int>::iterator it;
    for (it = state_ptr->nodes.begin(); it != state_ptr->nodes.end(); it++) {
      int d = it->second;
      int msg_time = LIMIT * -1;
      ListGraph::Node n = it->first;
      if (state_ptr->visited.find(n) != state_ptr->visited.end()) {
        msg_time = state_ptr->visited[n];
      }
      std::cout << d << " " << msg_time - state_ptr->visited[source] << "\n";
    }
  }
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
  std::cout << no_friends << " " << counter - no_friends << "\n";
  return 0;
}

int bandwidth_cost(ListGraph& g, ListGraph::NodeMap<int>& degree, int hops,
                   int visits) {
  for (int i = 0; i < visits; ++i) {
    int counter = 0;
    ListGraph::Node source = get_random_node(g);
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
  ListGraph::Node node = get_random_node(g);
  for (int i = 1; i <= ttl; i++) {
    if (wtype == 1) {
      node = get_high_neighbor(g, degree_map, node, visited);
    }
    else if (wtype == 2) {
      node = get_rand_neighbor(g, degree_map, node, visited);
    }
    if (!g.valid(node)) {
      break;
    }
    visited[node] = i;
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
    std::cout << "\n";
  }
  return 0;
}

int main (int argc, char* argv[]) {
  ListGraph g;
  ListGraph::NodeMap<int> label(g);
  ListGraph::NodeMap<int> degree(g);
  DegreeMap degree_map(degree);

  char msg[] = "help: ijsn13 gfile cap hops visits on_mean off_mean tries "
               "ttl wtype walks";

  if (argc < 11) {
    std::cout << msg << std::endl;
    return -1;
  }

  try {
    graphReader(g, argv[1]).nodeMap("label", label).run();
  } catch (Exception& error) {
    std::cerr << "Error: " << error.what() << std::endl;
    return -1;
  }

  int cap = atoi(argv[2]);
  int hops = atoi(argv[3]);
  int visits = atoi(argv[4]);
  int on_mean = atoi(argv[5]);
  int off_mean = atoi(argv[6]);
  int tries = atoi(argv[7]);
  int ttl = atoi(argv[8]);
  int wtype = atoi(argv[9]);
  int walks = atoi(argv[10]);

  degree_map.init(g);
  bandwidth_cost(g, degree, hops, visits);
  publish_time(g, degree, on_mean, off_mean, tries);
  random_walk(g, degree_map, ttl, wtype, walks);
  return 0;
}

