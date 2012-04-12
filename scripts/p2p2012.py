#!/usr/bin/env python

import random
import sys
import math
import networkx as nx
import socialModels as sm

out_file = None
ids = None

def add_random_ids(g):

    global ids
    ids = {}
    max_int = pow(2,64)
    for n in g.nodes_iter():
        ids[n] = random.randint(0, max_int)


def get_random_node(g, node, query):

    nodes = g.neighbors(node)
    next_hop = random.randint(0, len(nodes)-1)
    return nodes[next_hop]


def get_greedy_node(g, node, query):

    for i in range(2):
        node = get_random_node(g, node, query)

    old_dist = None
    nodes = g.neighbors(node)

    for n in nodes:
        dist = math.fabs(query - ids[n])

        if old_dist == None or dist < old_dist:
            node = n
            old_dist = dist

    return node


def get_greedy2_node(g, node, query):

    for i in range(2):
        node = get_random_node(g, node, query)

    old_dist = None
    nodes = g.neighbors(node)

    for n in nodes:
        dist = math.fabs(query - ids[n])

        if old_dist == None or dist < old_dist:
            node = n
            old_dist = dist

        for nn in g.neighbors(n):
            dist = math.fabs(query - g[nn][0])

            if old_dist == None or dist < old_dist:
                node = n
                old_dist = dist

    return node


def put_random_walk(g, ttl, query, par, next_node_funct):

    node = None
    start_node = None
    ttl = ttl/par
    rnd_idx = random.randint(0, len(g)-1)
    start_node = g.nodes()[rnd_idx]

    put_nodes = set()
    put_nodes.add(start_node)

    for i in range(par):
        node = start_node

        for j in range(ttl):
            node = next_node_funct(g, node, query)
            put_nodes.add(node)

    return put_nodes


def get_random_walk(g, ttl, query, par, next_node_funct, nodes):

    node = None
    start_node = None
    ttl = ttl/par
    rnd_idx = random.randint(0, len(g)-1)
    start_node = g.nodes()[rnd_idx]

    count = 0
    for i in range(par):
        node = start_node

        for j in range(ttl):
            count += 1
            node = next_node_funct(g, node, query)
            if node in nodes:
                return count

    return "miss"


def lookup(g, ttl, tries, par, next_node_funct):

    hits = 0
    max_int = pow(2, 64)
    add_random_ids(g)

    for i in range(tries):
        q = random.randint(0, max_int)
        nodes = put_random_walk(g, ttl, q, par, next_node_funct)
        result = get_random_walk(g, ttl, q, par, next_node_funct, nodes)
        print >> out_file, len(nodes), result

        if result >= 0: hits += 1

    return float(hits)/float(tries)


def sum_edges(g, hops):

    for source in g.nodes_iter():
        count = 0
        paths = nx.single_source_shortest_path(g, source, hops-1)

        for node in paths.iterkeys():
            count += g.degree(node)

        print >> out_file, count


def main():

    msg = "usage: ./p2p2012.py type r|g|g2 ttl par tries"

    if len(sys.argv) < 6:
        print msg
        sys.exit(1)

    global out_file, bcast
    out_file = sys.stdout

    gtype = sys.argv[1]
    walk = sys.argv[2]
    ttl = int(sys.argv[3])
    par = int(sys.argv[4])
    tries = int(sys.argv[5])

    if gtype == "a":
        g = nx.barabasi_albert_graph(97134, 3)
    elif gtype == "b":
        g = nx.barabasi_albert_graph(905668, 12)
    elif gtype == "c":
        g = sm.randomWalk_mod(97134, 0.90, 0.23)
    elif gtype == "d":
        g = sm.randomWalk_mod(905668, 0.93, 0.98)
    elif gtype == "e":
        g = sm.nearestNeighbor_mod(97134, 0.53, 1)
    elif gtype == "f":
        g = sm.nearestNeighbor_mod(905668, 0.90, 5)
    elif gtype == "g":
        g = nx.random_regular_graph(6, 97134)
    elif gtype == "h":
        g = nx.random_regular_graph(20, 905668)
    elif gtype == "i":
        g = nx.read_edgelist(sys.argv[6])

    if walk == "r":
        lookup(g, ttl, tries, par, get_random_node)
    elif walk == "g":
        lookup(g, ttl, tries, par, get_greedy_node)
    elif walk == "g2":
        lookup(g, ttl, tries, par, get_greedy2_node)

if __name__ == "__main__":
    main()

