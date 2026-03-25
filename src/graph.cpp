#include "graph.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <queue>
#include <limits>

/* ================================================================ */
/*  add_edge: adds an undirected edge (both directions)             */
/* ================================================================ */
void Graph::add_edge(int u, int v, double w)
{
    if(u < 0 || v < 0) return;
    if(u >= (int)adj.size() || v >= (int)adj.size()){
        int newn = std::max(u, v) + 1;
        adj.resize(newn);
        n = newn;
    }
    adj[u].push_back({v, w});
    adj[v].push_back({u, w});
}

/* ================================================================ */
/*  load_edge_list: loads plain "u v w" or DIMACS "a u v w" files   */
/*                                                                  */
/*  DIMACS .gr files list every undirected edge as TWO directed     */
/*  arcs (a u v w) and (a v u w).  Since add_edge() already stores */
/*  both directions, we keep only arcs with u < v to avoid 4x      */
/*  duplication.                                                    */
/* ================================================================ */
Graph Graph::load_edge_list(const std::string &fname)
{
    Graph G;
    std::ifstream in(fname);

    if(!in){
        std::cerr << "Cannot open graph file: " << fname << "\n";
        return G;
    }

    int u, v;
    double w;
    int maxv = -1;

    std::vector<std::tuple<int,int,double>> edges;
    std::string line;
    bool is_dimacs = false;

    while(std::getline(in, line)){
        if(line.empty()) continue;

        // Skip comments / headers
        if(line[0] == '#' || line[0] == '%' || line[0] == 'c' || line[0] == 'p')
            continue;

        std::istringstream ss(line);

        if(line[0] == 'a'){
            // DIMACS arc line: "a u v w"
            is_dimacs = true;
            char a;
            if(!(ss >> a >> u >> v >> w)) continue;
            u--; v--;  // DIMACS is 1-based

            // Deduplicate: keep only one direction
            if(u > v) continue;
        } else {
            // Plain edge list: "u v w" (0-based)
            if(!(ss >> u >> v >> w)) continue;
        }

        edges.emplace_back(u, v, w);
        maxv = std::max(maxv, std::max(u, v));
    }

    G = Graph(maxv + 1);
    for(auto &e : edges){
        std::tie(u, v, w) = e;
        G.add_edge(u, v, w);
    }

    if(is_dimacs)
        std::cerr << "DIMACS format detected, dedup applied\n";

    return G;
}

/* ================================================================ */
/*  constant_degree_transform (for ER graphs)                       */
/*                                                                  */
/*  Each original vertex v with degree d is expanded into a cycle   */
/*  of d copies linked by zero-weight edges.  For every original    */
/*  edge (u,v,w) we add one cross-edge between copy x_uv and x_vu  */
/*  with weight w.                                                  */
/*                                                                  */
/*  To avoid double cross-edges (since the stored adjacency already */
/*  has both directions), we add the cross-edge only when u < v.    */
/* ================================================================ */
Graph constant_degree_transform(const Graph &G_orig)
{
    int n = G_orig.adj.size();

    // Map (vertex, neighbor-index) -> new node id
    std::vector<std::vector<int>> map_to_new(n);
    int new_id = 0;
    for(int v = 0; v < n; ++v){
        int deg = G_orig.adj[v].size();
        map_to_new[v].resize(deg);
        for(int i = 0; i < deg; ++i)
            map_to_new[v][i] = new_id++;
    }

    Graph Gp(new_id);

    // Zero-weight cycles per original vertex
    for(int v = 0; v < n; ++v){
        int deg = G_orig.adj[v].size();
        if(deg <= 1) continue;
        for(int i = 0; i < deg; ++i){
            int a = map_to_new[v][i];
            int b = map_to_new[v][(i + 1) % deg];
            Gp.add_edge(a, b, 0.0);
        }
    }

    // Cross-edges: only for u < v to avoid duplicates
    for(int u = 0; u < n; ++u){
        for(size_t i = 0; i < G_orig.adj[u].size(); ++i){
            int v = G_orig.adj[u][i].to;
            double w = G_orig.adj[u][i].weight;

            if(u >= v) continue;  // process each pair once

            // Find index of u in v's adjacency list
            int j_found = -1;
            for(size_t j = 0; j < G_orig.adj[v].size(); ++j){
                if(G_orig.adj[v][j].to == u){
                    j_found = (int)j;
                    break;
                }
            }
            if(j_found == -1) continue;

            int a = map_to_new[u][i];
            int b = map_to_new[v][j_found];
            Gp.add_edge(a, b, w);
        }
    }

    return Gp;
}

/* ================================================================ */
/*  check_connectivity: BFS reachability from source                */
/* ================================================================ */
void check_connectivity(const Graph &G, int source)
{
    int n = G.adj.size();
    std::vector<bool> visited(n, false);
    std::queue<int> q;

    visited[source] = true;
    q.push(source);

    int reachable = 0;
    while(!q.empty()){
        int u = q.front(); q.pop();
        reachable++;
        for(auto &e : G.adj[u]){
            if(!visited[e.to]){
                visited[e.to] = true;
                q.push(e.to);
            }
        }
    }

    std::cerr << "Connectivity: " << reachable << "/" << n
              << " reachable from source " << source << "\n";

    if(reachable < n)
        std::cerr << "WARNING: Graph is NOT fully connected!\n";
}