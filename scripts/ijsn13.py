#!/usr/bin/env python

import sys
import random
import networkx as nx
import scipy.stats as stats

LIMIT = 7 * 24 * 12
OFFSET_RANGE = 12 * 24

class State():
    def __init__(self):
        self.on_schedule = {}
        self.off_schedule = {}
        self.visited = {}

def cap_edges(g, cap):
    if cap == 0: return 0
    total_rm = 0
    for n in g.nodes():
        edges = g.edges(n)
        if len(edges) > cap:

            no_to_rm = len(edges) - cap
            total_rm += no_to_rm
            random.shuffle(edges)

            for edge in edges:
                g.remove_edge(*edge)
                no_to_rm -= 1
                if no_to_rm == 0: break

    return total_rm

#=======================Timing Analysis=======================================
def set_schedule(state, node, limit=LIMIT):
    on_times = []
    off_times = []
    current_time = int(stats.uniform.rvs(scale=OFFSET_RANGE))

    while current_time < limit:
        on_interval = int(stats.expon.rvs(scale=state.on_mean))
        on_times.append((current_time, current_time + on_interval))
        current_time += (on_interval + 1)

        off_interval = int(stats.expon.rvs(scale=state.off_mean))
        off_times.append((current_time, current_time + off_interval))
        current_time += (off_interval + 1)

    state.on_schedule[node] = on_times
    state.off_schedule[node] = off_times

def find_overlap(state, source, target, limit=LIMIT):
    if source not in state.on_schedule:
         set_schedule(state, source)
    if target not in state.on_schedule:
        set_schedule(state, target)

    found = 0
    stimes = state.on_schedule[source]
    ttimes = state.on_schedule[target]
    old_time = state.visited.get(target, limit)
    if old_time <= state.visited[source]: return found

    for stime in stimes:
        if found == 1: break

        for ttime in ttimes:
            if not (ttime[1] <= stime[0] or ttime[0] >= stime[1]) and\
                stime[1] >= state.visited[source]:
                new_time = max(stime[0], ttime[0], state.visited[source])
                if new_time < old_time:
                    state.visited[target] = min(old_time, new_time)
                    found = 1
                break
    return found

def set_message_time(state, node):
    set_schedule(state, node)
    interval = state.on_schedule[node][0]
    message_time = int(stats.uniform.rvs(scale=(interval[1]-interval[0])))
    state.visited[node] = interval[0] + message_time

def get_pub_time(g, state, source):
    nodes = {}
    nodes[source] = 0
    set_message_time(state, source)

    for node in g.neighbors(source):
        nodes[node] = 1
        find_overlap(state, source, node)

    while True:
        counter = 0
        for node, dist in nodes.items():
            if dist == 1:
                for foaf in g.neighbors(node):
                    if foaf not in nodes: nodes[foaf] = 2
                    if node in state.visited:
                        counter += find_overlap(state, node, foaf)
        if counter == 0: break

    for i in range(2,4):
        for node, dist in nodes.items():
            if dist == i:
                for foaf in g.neighbors(node):
                    if foaf not in nodes: nodes[foaf] = i + 1
                    if node in state.visited:
                        find_overlap(state, node, foaf)
    return nodes

def publish_time(g, on_mean, off_mean, tries):
    for i in range(tries):
        state = State()
        state.on_mean = on_mean
        state.off_mean = off_mean
        source = int(stats.uniform.rvs(scale=len(g)))
        nodes = get_pub_time(g, state, source)
        print "stats %s %s" % (g.degree(source), nx.clustering(g, source))
        for n, d in nodes.iteritems():
            print d, state.visited.get(n, -LIMIT) - state.visited[source]
        del state

#=======================Bandwidth Analysis====================================
def get_bw_cost(g, source, hops):
    counter = 0
    paths = nx.single_source_shortest_path(g, source, hops)
    no_friends = len(paths) - 1
    for k, v in paths.iteritems():
      if (len(v) <= hops):
          counter += g.degree(k)
    return no_friends, counter - no_friends

def bandwidth_cost(g, hops, visits):
    for i in range(visits):
        source = int(stats.uniform.rvs(scale=len(g)))
        if not g.has_node(source): continue
        print "%s %s" % get_bw_cost(g, source, hops)

#=======================Replication Analysis==================================
def get_high_neighbor(g, node, visited):
    nodes = sorted(g.neighbors(node), key=lambda n: -g.degree(n))
    for node in nodes:
        if node not in visited: return node
    return None

def get_rand_neighbor(g, node, visited):
    nodes = g.neighbors(node)
    # commented out because it gets stuck at nodes with degree = 1
    #random.shuffle(nodes)
    #for node in nodes:
    #    if node not in visited: return node
    return nodes[int(stats.uniform.rvs(scale=len(nodes)))]

def do_random_walk(g, ttl, wtype, visited):
    node = int(stats.uniform.rvs(scale=len(g)))
    if not g.has_node(node): return {}
    for i in range(ttl):
        if wtype == 1:
            node = get_high_neighbor(g, node, visited)
        elif wtype == 2:
            node = get_rand_neighbor(g, node, visited)

        if node == None: break
        visited[node] = i
    return visited

def random_walk(g, ttl, wtype, walks):
    for i in range(walks):
        put_nodes = do_random_walk(g, ttl, wtype, {})
        get_nodes = do_random_walk(g, ttl, wtype, {})
        result = "%s %s" % (len(get_nodes), len(put_nodes))
        for k, v in get_nodes.iteritems():
            if k in put_nodes:
                result += " %s %s h" % (v, put_nodes[k])
                break
        print result

def main():
    msg = "help: ijsn13.py gtype cap hops visits on_mean off_mean tries " \
          "ttl wtype walks"
    if len(sys.argv) < 11: print msg; return -1

    gtype = sys.argv[1]
    cap = int(sys.argv[2])
    hops = int(sys.argv[3])
    visits = int(sys.argv[4])
    on_mean = 12 * int(sys.argv[5])
    off_mean = 12 * int(sys.argv[6])
    tries = int(sys.argv[7])
    ttl = int(sys.argv[8])
    wtype = int(sys.argv[9])
    walks = int(sys.argv[10])

    g = nx.read_edgelist(gtype, nodetype=int, create_using=nx.Graph())
    bandwidth_cost(g, hops, visits)
    publish_time(g, on_mean, off_mean, tries)
    random_walk(g, ttl, wtype, walks)

if __name__ == "__main__":
    main()

