#!/usr/bin/env python

import networkx as nx
import numpy as np
import scipy.stats as stats
import sys
import random

def load_graph(path, n):

    """ Loads edges for graph files """

    f = open(path)
    g = nx.Graph()

    for line in f:
        parts = line.split()
        node1 = int(parts[0])
        node2 = int(parts[1])

        if node1 >= n:
            break

        if node1 <= n and node2 <= n:
            g.add_edge(node1, node2)

    f.close()
    return g


def sum_edges(g, source, hops):

    """ Sum edges that are hops away in undirected graph """

    edges = 0

    paths = nx.single_source_shortest_path(g, source, hops)
    for node in paths.iterkeys():
        edges += len(g.neighbors(node))

    return edges


def add_latency(g, a):

    """ Set latency to each edge """

    low, high = get_low_high(a)

    for u,v,d in g.edges_iter(data=True):
        d["latency"] = get_latency(a, low, high)


def get_low_high(a):

    """ Returns the low and high of Pareto cdf """

    return stats.pareto.cdf((10000,1000000),a)


def get_latency(a, low, high):

    """ Provides latency based on Pareto distribution """

    if random.random() > 0.69:
        dist = random.randint(1000000,20000000)
    else:
        q = random.uniform(low,high)
        dist = int(stats.pareto.ppf(q, a))

    return dist/(1000*66) + 5


def get_reached(g, source, hops, timeout):

    """ Returns percentage of friends reached based on timeout """

    paths = nx.single_source_shortest_path(g, source, hops)
    total = len(paths)
    reached = total

    for v in paths.itervalues():
        for i in range(1, len(v)):
            lat = g[v[i-1]][v[i]]["latency"]
            if lat > timeout:
                reached -= 1
                break

    return float(reached)/float(total)


def do_conflict(g, source, prob):

    """ Do a collision scenario """

    count1, count2, count3 = 0, 0, 0

    g.node[source]["m"] = source
    for n in g.neighbors_iter(source):
        g.node[n]["m"] = source

    peer = random.choice(g.neighbors(source))
    for n in g.neighbors_iter(peer):
        if "m" not in g.node[n]:
            g.node[n]["m"] = peer
            count1 += 1

    for n in g.neighbors_iter(peer):
        if g.node[n]["m"] == source:
            result = resolve(g, n, source, peer, prob)
            g.node[n]["m"] = result
            if result == peer:
                count2 += 1
            else:
                count3 += 1

    clear_mappings(g, source)
    clear_mappings(g, peer)

    share = float(count1)/len(g.neighbors(peer))
    wins = float(count2)/float(count2+count3)

    return share, wins

def resolve(g, source, peer1, peer2, prob):

    """ Resolve a conflict between two mappings """

    counter1, counter2 = 0, 0

    for n in g.neighbors_iter(source):
        if "m" in g.node[n]:
            if g.node[n]["m"] == peer1:
                counter1 += 1
            elif g.node[n]["m"] == peer2:
                counter2 += 1

    if counter2 > counter1:
        return peer2
    elif counter2 == counter1:
        rand = random.random()
        if rand > prob:
            return peer2
        else:
            return peer1
    else:
        return peer1


def clear_mappings(g, source):

    """ Clears mappings for neighbors """

    if "m" in g.node[source]:
        del g.node[source]["m"]

    for n in g.neighbors_iter(source):
        if "m" in g.node[n]:
            del g.node[n]["m"]

def main():

    """ The main function """

    g = load_graph("orkut-links.txt", int(sys.argv[1]))

    for node in g.nodes():
        print sum_edges(g, node, int(sys.argv[2]))


if __name__ == "__main__":
    main()
    #pass

