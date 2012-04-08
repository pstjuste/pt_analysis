#!/usr/bin/env python

import random
import sys
import networkx as nx
import socialModels as sm

def do_random_walk(g, ttl):

    node = None
    node_it = g.nodes_iter()
    visited_nodes = set()

    for i in range(random.randint(0,len(g))):
        node = node_it.next()

    for i in range(ttl):
        node_it = g.neighbors_iter(node)
        next_hop = random.randint(0, g.degree(node))

        for j in range(next_hop):
            node = node_it.next()

        visited_nodes.add(node)

    return visited_nodes


def random_lookup(g, ttl, tries):

    hits = 0
    for i in range(tries):
        put_set = do_random_walk(g, ttl)
        get_set = do_random_walk(g, ttl)
        intersect_set = put_set & get_set

        if len(intersect_set) > 0: hits += 1

    return float(hits)/float(tries)

def sum_edges(g, hops, out_file):

    for source in g.nodes_iter():
        count = 0
        paths = nx.single_source_shortest_path(g, source, hops-1)

        for node in paths.iterkeys():
            count += g.degree(node)

        print >> out_file, count


def main():
    g = sm.nearestNeighbor_mod(97134, 0.53, 1)
    #sum_edges(g, sys.stdout)
    print random_lookup(g, 200, 10)

if __name__ == "__main__":
    main()

