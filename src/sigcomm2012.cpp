
#include <iostream>
#include <lemon/list_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/concepts/digraph.h>
#include <lemon/bfs.h>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <set>

using namespace lemon;

int get_digraph(ListDigraph& dg, ListDigraph::NodeMap<int>& dlabel,
    char di_path[])
{
    if (countNodes(dg) > 0) return -1;

    try {
        digraphReader(dg, di_path).nodeMap("label", dlabel).run();
    } catch (Exception& error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return -1;
    }
    return 0;
}

int cap_edges(ListGraph& g, int cap)
{
    if (cap == -1) return -1;

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
    std::cerr << "total_rm " << total_rm << "\n";
    return total_rm;
}


int set_degree_map(ListGraph& g, ListGraph::NodeMap<int>& degree)
{
    for (ListGraph::NodeIt n(g); n != INVALID; ++n) {
        int count = 0;
        for (ListGraph::IncEdgeIt e(g, n); e != INVALID; ++e) count++;
        degree[n] = count;
    }
    return 0;
}


int sum_edges(ListGraph& g, ListGraph::NodeMap<int>& degree, int hops)
{
    if (hops == -1) return -1;

    int total_count = 0;
    Bfs<ListGraph> bfs(g);
    for (ListGraph::NodeIt n(g); n != INVALID; ++n) {
        int count = 0;
        bfs.init();
        bfs.addSource(n);

        while (!bfs.emptyQueue()) {
            ListGraph::Node v = bfs.processNextNode();
            if (bfs.dist(v) > hops-1) break;
            count += degree[v];
        }
        total_count += count;
        std::cout << count << "\n";
    }
    std::cerr << "total_count " << total_count << "\n";
    return total_count;
}


int wasted_packets(ListGraph& g, ListGraph::NodeMap<int>& label,
    ListDigraph & dg, ListDigraph::NodeMap<int>& dlabel,
    std::map<int,ListGraph::Node>& lb_to_id, char di_path[], int enable)
{
    if (enable == -1) return -1;

    get_digraph(dg, dlabel, di_path);

    int total_wasted = 0;
    Bfs<ListGraph> bfs(g);
    for (ListDigraph::NodeIt dn(dg); dn != INVALID; ++dn) {
        int wasted = 0;
        ListGraph::Node n;
        std::map<int, ListGraph::Node>::iterator it;
        int source_lb = dlabel[dn];
        it = lb_to_id.find(source_lb);
        if (it == lb_to_id.end()) continue;

        std::set<int> followers;
        for (ListDigraph::InArcIt a(dg, dn); a != INVALID; ++a) {
            followers.insert(dlabel[dg.source(a)]);
        }

        n = it->second;
        bfs.init();
        bfs.addSource(n);
        while (!bfs.emptyQueue()) {
            ListGraph::Node v = bfs.processNextNode();
            if (bfs.dist(v) > 2) break;
            if (followers.find(label[v]) == followers.end()) wasted++;
        }
        std::cout << wasted << "\n";
        total_wasted += wasted;
    }
    std::cerr << "total_wasted " << total_wasted;
    return total_wasted;
}

int get_followers_dist(ListGraph& g, ListGraph::NodeMap<int>& label, 
    ListDigraph& dg, ListDigraph::NodeMap<int>& dlabel,
    std::map<int,ListGraph::Node>& lb_to_id, char di_path[], int dist) 
{
    if (dist == -1) return -1;

    get_digraph(dg, dlabel, di_path);

    int no_of_paths = 0;
    Bfs<ListGraph> bfs(g);
    for (ListDigraph::NodeIt dn(dg); dn != INVALID; ++dn) {
        ListGraph::Node source, target;
        std::map<int, ListGraph::Node>::iterator it;
        int source_lb = dlabel[dn];
        it = lb_to_id.find(source_lb);

        if (it == lb_to_id.end()) {
            std::cout << "no_source\n";
            continue;
        }

        source = it->second;
        bfs.init();
        bfs.addSource(source);
        for (ListDigraph::OutArcIt a(dg, dn); a!=INVALID; ++a) {
            int target_lb = dlabel[dg.target(a)];
            if (source_lb == target_lb) continue;

            it = lb_to_id.find(target_lb);

            if (it != lb_to_id.end()) {
                target = it->second;

                while (!bfs.reached(target) && !bfs.emptyQueue()) {
                    ListGraph::Node t = bfs.processNextNode();
                    if (bfs.dist(t) > 6) break;
                }

                if (bfs.reached(target)) {
                    std::cout << bfs.dist(target)<< "\n";
                    no_of_paths++;
                }
                else {
                    std::cout << "no_path\n";
                }
            }
            else {
                std::cout << "no_target\n";
            }
        }
    }

    std::cerr << "no of paths " << no_of_paths << std::endl;
    return no_of_paths;
}

struct tmp_edge {
    ListDigraph::Node source;
    ListDigraph::Node target;
};

// Flawed logic needs to be fixed
int add_pseudo_edges(ListGraph& g, ListGraph::NodeMap<int>& label,
    ListDigraph& dg, ListDigraph::NodeMap<int>& dlabel,
    std::map<int,ListGraph::Node>& lb_to_id, char di_path[], int threshold)
{
    if (threshold == -1) return -1;

    get_digraph(dg, dlabel, di_path);

    std::vector<tmp_edge> new_edges;
    std::map<int,ListDigraph::Node> id_to_node;
    for (ListDigraph::NodeIt dn(dg); dn != INVALID; ++dn) {
        id_to_node[dlabel[dn]] = dn;
    }

    Bfs<ListGraph> bfs(g);
    for (ListDigraph::NodeIt dn(dg); dn != INVALID; ++dn) {
        ListGraph::Node n;
        std::set<ListGraph::Node> nodes;
        std::map<int,int> fw_count;
        std::map<int, ListGraph::Node>::iterator git;

        int source_lb = dlabel[dn];
        git = lb_to_id.find(source_lb);
        if (git == lb_to_id.end()) continue;

        n = git->second;
        bfs.init();
        bfs.addSource(n);

        while (!bfs.emptyQueue()) {
            ListGraph::Node v = bfs.processNextNode();
            if (bfs.dist(v) > 4) break;
            nodes.insert(v);
        }

        std::set<int> followings;
        for (ListDigraph::OutArcIt a(dg, dn); a != INVALID; ++a) {
            followings.insert(dlabel[dg.target(a)]);
        }

        std::set<ListGraph::Node>::iterator it;
        for (it = nodes.begin(); it != nodes.end(); ++it) {

            if (bfs.dist(*it) > 2) continue;

            ListDigraph::Node k = id_to_node[label[*it]];
            for (ListDigraph::OutArcIt a(dg, k); a != INVALID; ++a) {
                int f_label = dlabel[dg.target(a)];
                if (followings.find(f_label) != followings.end()) {
                    // do nothing
                }
                else if (fw_count.find(f_label) != fw_count.end()) {
                    fw_count[f_label] = fw_count[f_label] + 1;
                }
                else {
                    fw_count[f_label] = 1;
                }
            }
        }

        std::map<int,int>::iterator itt;
        for (itt = fw_count.begin(); itt != fw_count.end(); ++itt) {
            tmp_edge t_edge;
            t_edge.source = dn;
            t_edge.target = id_to_node[itt->first];

            git = lb_to_id.find(itt->first);
            if (git == lb_to_id.end()) continue;

            n = git->second;
            if (itt->second >= threshold && bfs.reached(n) 
                && bfs.dist(n) <= 4) {
                new_edges.push_back(t_edge);
            }
        }
    }

    std::vector<tmp_edge>::iterator it;
    for (it = new_edges.begin(); it != new_edges.end(); ++it) {
        tmp_edge t_edge = *it;
        dg.addArc(t_edge.source, t_edge.target);
    }

    std::cerr << "new edges " << new_edges.size() << "\n";
    return 0;
}

int check_path(ListGraph& g, ListGraph::Node& source, Bfs<ListGraph>& bfs,
    std::set<int>& followers, ListGraph::NodeMap<int>& label)
{
    int count = 0;
    Bfs<ListGraph> nbfs(g);
    nbfs.init();
    nbfs.addSource(source);

    std::set<ListGraph::Node> nodes;
    while (!nbfs.emptyQueue()) {
        ListGraph::Node v = nbfs.processNextNode();
        if (nbfs.dist(v) > 2) break;
        nodes.insert(v);
    }

    std::set<ListGraph::Node>::iterator it;
    for (it = nodes.begin(); it != nodes.end(); ++it) {
        ListGraph::Node n = *it;
        if (followers.find(label[n]) != followers.end() && bfs.reached(n) &&
            bfs.dist(n) <= 4) count++;
    }
    return count;
}

int random_walk(ListGraph& g, ListGraph::NodeMap<int>& label,
    ListGraph::Node& source, ListGraph::Node& target, 
    ListGraph::NodeMap<int>& degree, Bfs<ListGraph>& bfs, 
    std::set<int>& followers, int ttl)
{
    if (ttl == 0) return 0;

    int next_hop = rand() % degree[source];
    ListGraph::IncEdgeIt e(g, source);

    for (int i = 0; i < next_hop; ++i) ++e;

    ListGraph::Node n = g.target(e);
    if (followers.find(label[n]) != followers.end() && bfs.reached(n) &&
        bfs.dist(n) <= 4) return 1;

    if (!bfs.reached(n) || bfs.dist(n) > 2) {
        return random_walk(g, label, n, target, degree, bfs, followers, ttl-1);
    }
    else {
        return 1;
    }
}


int find_paths(ListGraph& g, ListGraph::NodeMap<int>& label,
    ListDigraph& dg, ListDigraph::NodeMap<int>& dlabel,
    std::map<int,ListGraph::Node>& lb_to_id, ListGraph::NodeMap<int>& degree, 
    int hops, int tries, int ttl, double prob, char di_path[])
{
    if (hops == -1) return -1;

    get_digraph(dg, dlabel, di_path);

    int path_count = 0;
    Bfs<ListGraph> bfs(g);
    for (ListDigraph::NodeIt dn(dg); dn != INVALID; ++dn) {

        double rnd = ((double) rand())/((double) RAND_MAX);
        if (rnd > prob) continue;

        ListGraph::Node n;
        std::map<int, ListGraph::Node>::iterator git;

        int source_lb = dlabel[dn];
        git = lb_to_id.find(source_lb);
        if (git == lb_to_id.end()) continue;

        std::set<ListGraph::Node> nodes;
        std::set<int> followers;
        for (ListDigraph::InArcIt a(dg, dn); a != INVALID; ++a) {
            followers.insert(dlabel[dg.source(a)]);
        }

        n = git->second;
        bfs.init();
        bfs.addSource(n);
        while (!bfs.emptyQueue()) {
            ListGraph::Node v = bfs.processNextNode();
            if (bfs.dist(v) > hops) break;

            rnd = ((double) rand())/((double) RAND_MAX);
            if (rnd < prob && bfs.dist(v) == hops && 
                followers.find(label[v]) != followers.end()) {
                nodes.insert(v);
            }
        }

        std::set<ListGraph::Node>::iterator it;
        for (it = nodes.begin(); it != nodes.end(); ++it) {
            int miss = 0;
            path_count++;

            ListGraph::Node k = *it;
            int f_count = check_path(g, k, bfs, followers, label);
            if (f_count == 0) {
                while (miss < tries) {
                    if (random_walk(g, label, k, n, degree, bfs, followers, 
                        ttl) != 0) break;
                    miss++;
                }
                std::cout << miss << "\n";
            }
            else std::cout << "f " << f_count << "\n";
        }
    }
    std::cerr << "path_count " << path_count << "\n";
    return path_count;
}


int main(int argc, char* argv[])
{

    char msg[] = "sigcomm graph1 graph2 cap dist edges wasted thld "
                 "hops tries ttl prob";
    if (argc < 12) {
        std::cout << msg << "\n";
        return -1;
    }

    ListGraph g;
    ListGraph::NodeMap<int> label(g);
    ListGraph::NodeMap<int> degree(g);
    ListDigraph dg;
    ListDigraph::NodeMap<int> dlabel(dg);

    try {
        graphReader(g, argv[1]).nodeMap("label", label).run();
    } catch (Exception& error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return -1;
    }

    srand(-1);

    std::map<int,ListGraph::Node> lb_to_id;
    for (ListGraph::NodeIt n(g); n != INVALID; ++n) {
        lb_to_id[label[n]] = n;
    }

    int cap = atoi(argv[3]);
    int dist = atoi(argv[4]);
    int edges = atoi(argv[5]);
    int wasted = atoi(argv[6]);
    int threshold = atoi(argv[7]);
    int hops = atoi(argv[8]);
    int tries = atoi(argv[9]);
    int ttl = atoi(argv[10]);
    double prob = atof(argv[11]);

    cap_edges(g, cap);
    set_degree_map(g, degree);
    get_followers_dist(g, label, dg, dlabel, lb_to_id, argv[2], dist);
    sum_edges(g, degree, edges);
    wasted_packets(g, label, dg, dlabel, lb_to_id, argv[2], wasted);
    add_pseudo_edges(g, label, dg, dlabel, lb_to_id, argv[2], threshold);
    find_paths(g, label, dg, dlabel, lb_to_id, degree, hops, tries, ttl, 
        prob, argv[2]);

    std::cerr << "g node count " << countNodes(g) << std::endl;
    std::cerr << "g edge count " << countEdges(g) << std::endl;

    std::cerr << "dg node count " << countNodes(dg) << std::endl;
    std::cerr << "dg edge count " << countArcs(dg) << std::endl;

    return 0;
}

