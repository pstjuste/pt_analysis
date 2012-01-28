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


def wasted_packets(g, dg, enable):

    if enable == -1: return -1

    if len(dg) == 0:
        dg = nx.read_edgelist(sys.argv[2], create_using=dg)

    total_wasted = 0
    for u in dg.nodes():
        if not g.has_node(u): continue

        wasted = 0
        followers = set(dg.predecessors(u))
        n_dists = nx.single_source_shortest_path_length(g, u, 2)

        for n in n_dists.iterkeys():
            if n not in followers: wasted += 1

        print wasted
        total_wasted += wasted

    print >> sys.stderr, "total_wasted ", total_wasted
    return total_wasted


def get_followers_dist(g, dg, follow):

    if follow == -1: return -1

    if len(dg) == 0:
        dg = nx.read_edgelist(sys.argv[2], create_using=dg)

    no_of_paths = 0
    for u in dg.nodes():

        if not g.has_node(u):
            print "no_source"
            continue

        for v in dg.successors(u):
            if u == v: continue

            if g.has_node(v):
                try:
                    print nx.shortest_path_length(g, source=u, target=v)
                    no_of_paths += 1
                except nx.exception.NetworkXError as err:
                    print "no_path"
            else:
                print "no_target"

    print >> sys.stderr, "no of paths", no_of_paths
    return no_of_paths


def add_pseudo_edges(g, dg, threshold):
    """ flawed logic, needs to be fixed """

    if threshold == -1 : return -1

    if len(dg) == 0:
        dg = nx.read_edgelist(sys.argv[2], create_using=dg)

    new_edges = []
    for n in dg.nodes():

        if not g.has_node(n): continue

        fw_count = {}
        n_dists = nx.single_source_shortest_path_length(g,n,4)
        followings = set(dg.successors(n))

        for node, dist in n_dists.iteritems():
            if dist > 2: continue

            for f in dg.successors(node):
                if f not in followings:
                    if f in fw_count:
                        fw_count[f] = fw_count[f] + 1
                    else: fw_count[f] = 1

        for k,v in fw_count.iteritems():
            if v >= threshold and k in n_dists and n_dists[k] <= 4: 
                new_edges.append((n,k))

    for e in new_edges: dg.add_edge(*e)
    print >> sys.stderr, "new edges", len(new_edges)
    return 0


def check_path(g, source, dists, followers):

    nodes = nx.single_source_shortest_path_length(g,source,2)
    count = 0

    for n in nodes.iterkeys():
        if n in followers and n in dists and dists[n] <= 4: count += 1

    return count


def random_walk(g, source, target, dists, followers, ttl):

    if ttl == 0: return 0

    next_hop = random.randint(0,g.degree(source)-1)
    n = g.neighbors(source)[next_hop]

    if n in followers and n in dists and dists[n] <= 4: return 1

    if n not in dists or dists[n] > 2:
        return random_walk(g, n, target, dists, followers, ttl-1)
    else:
        return 1


def find_paths(g, dg, hops, tries, ttl, prob):

    if hops == -1: return -1

    if len(dg) == 0:
        dg = nx.read_edgelist(sys.argv[2], create_using=dg)

    path_count = 0
    for n in g.nodes():
        rnd = random.random()
        if rnd > prob: continue

        followers = set(dg.predecessors(n))
        dists = nx.single_source_shortest_path_length(g, n, hops)
        nodes = set()

        for k in dists.iterkeys():
            rnd = random.random()
            if rnd < prob and dists[k] == hops and k in followers: 
                nodes.add(k)

        for node in nodes:
            miss = 0
            path_count += 1
            f_count = check_path(g, node, dists, followers)

            if f_count == 0:
                while miss < tries:
                    if random_walk(g, node, n, dists, followers,ttl) != 0: 
                        break
                    miss += 1
                print miss
            else: print "f ", f_count

    print >> sys.stderr, "path_count", path_count
    return path_count


def main():

    msg = "help: sigcomm graph1 graph2 cap dist edges wasted thld " \
          "hops tries ttl prob"
    if len(sys.argv) < 12: print msg; return -1

    g = nx.read_edgelist(sys.argv[1], create_using=nx.Graph())
    dg = nx.DiGraph()

    random.seed(-1)

    cap = int(sys.argv[3])
    dist = int(sys.argv[4])
    edges = int(sys.argv[5])
    wasted = int(sys.argv[6])
    threshold = int(sys.argv[7])
    hops = int(sys.argv[8])
    tries = int(sys.argv[9])
    ttl = int(sys.argv[10])
    prob = float(sys.argv[11])

    cap_edges(g, cap)
    get_followers_dist(g, dg, dist)
    sum_edges(g, edges)
    wasted_packets(g, dg, wasted)
    add_pseudo_edges(g, dg, threshold)
    find_paths(g, dg, hops, tries, ttl, prob)

    print >> sys.stderr, "g nodes", len(g)
    print >> sys.stderr, "g edges", g.size()

    print >> sys.stderr, "dg nodes", len(dg)
    print >> sys.stderr, "dg edges", dg.size()


if __name__ == "__main__":
    main()

