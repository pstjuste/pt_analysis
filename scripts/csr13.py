#!/usr/bin/env python

import sys
import random
import networkx as nx

def cap_edges(g, cap):

    if cap == -1: return -1

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

    print >> sys.stderr, "total_rm ", total_rm
    return total_rm

def sum_edges(g, hops):

    if hops == -1: return -1

    total_count = 0
    for source in g.nodes():
        count = 0
        paths = nx.single_source_shortest_path(g, source, hops-1)

        for node in paths.iterkeys():
            count += g.degree(node)

        print count

    print >> sys.stderr, "total_count ", total_count
    return total_count

def get_followers_dist(g, dg, follow):

    if follow == -1: return -1

    no_of_paths = 0
    for u in dg.nodes():

        if not g.has_node(u):
            print "no_source"
            continue

        for v in dg.neighbors(u):
            if u == v: continue

            if g.has_node(v):
                try:
                    print nx.shortest_path_length(g, source=u, target=v)
                    no_of_paths += 1
                except nx.exception.NetworkXNoPath as err:
                    print "no_path"
            else:
                print "no_target"

    print >> sys.stderr, "no of paths", no_of_paths
    return no_of_paths


def main():

    g = nx.read_edgelist(sys.argv[1], create_using=nx.Graph())
    dg = g.copy()

    random.seed(-1)

    cap = int(sys.argv[2])
    dist = int(sys.argv[3])
    edges = int(sys.argv[4])

    cap_edges(g, cap)
    get_followers_dist(g, dg, dist)
    sum_edges(g, edges)

    print >> sys.stderr, "g nodes", len(g)
    print >> sys.stderr, "g edges", g.size()

    print >> sys.stderr, "dg nodes", len(dg)
    print >> sys.stderr, "dg edges", dg.size()


if __name__ == "__main__":
    main()

