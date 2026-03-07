#include "graph.h"
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>

void Graph :: add_edge(int u, int v, double w)
    {
        if (u < 0 || v < 0) return;
        if (u >= (int)adj.size() || v >= (int)adj.size())
            {
                int newn = std::max(u,v) +1;
                adj.resize(newn);
                n = newn;
            }
        
        adj[u].push_back({v,w});
        adj[v].push_back({u,w});
    }

Graph Graph :: load_edge_list(const std::string &fname)
    {
        Graph G;
        std::ifstream in(fname);

        if (!in)
            {
                std::cerr<<"Cannot open Graph file: "<<fname<<std::endl;
                return G;
            }
        int u,v;
        double w;
        int maxv = -1;

        std::vector<std::tuple<int,int,double>> edges;
        std::string line;

        while (std::getline(in, line))
            {
                if (line.empty()) continue;

                // Skip comments / headers
                if (line[0] == '#' || line[0] == '%' || line[0] == 'c' || line[0] == 'p')
                continue;

                std::istringstream ss(line);

                // Case 1: DIMACS road graph -> "a u v w"
                if (line[0] == 'a')
                    {
                        char a;
                        if (!(ss >> a >> u >> v >> w)) continue;

                        // DIMACS is 1-based
                        u--; 
                        v--;
                    }
                // Case 2: Plain edge list -> "u v w"
                else
                    {
                        if (!(ss >> u >> v >> w)) continue;
                        // assume already 0-based
                    }

                edges.emplace_back(u, v, w);

                maxv = std::max(maxv, std::max(u, v));
            }

        G = Graph(maxv + 1);
        for (auto &e : edges)
            {
                std::tie(u,v,w) = e;
                G.add_edge(u,v,w);
            }
        return G;
    }

// Build constant-degree transform G' as in the paper.
// Returns new graph Gp and mapping info: optional (not needed for baseline)
Graph constant_degree_transform(const Graph &G_orig)
{
    // Each original vertex v with deg d -> create a cycle of d nodes x_vw
    // We'll number new nodes consecutively. For each edge (u,v) add edge between x_uv and x_vu with weight w_uv.
    int n = G_orig.adj.size();
    // map (v, neighbor index) -> new id
    std::vector<std::vector<int>> map_to_new(n);
    int new_id = 0;
    for(int v=0; v<n; ++v){
        int deg = G_orig.adj[v].size();
        map_to_new[v].resize(deg);
        for(int i=0;i<deg;i++){
            map_to_new[v][i] = new_id++;
        }
    }
    Graph Gp(new_id);
    // add zero-weight cycles per original vertex
    for(int v=0; v<n; ++v){
        int deg = G_orig.adj[v].size();
        if(deg<=1) continue;
        for(int i=0;i<deg;i++){
            int a = map_to_new[v][i];
            int b = map_to_new[v][(i+1)%deg];
            Gp.add_edge(a,b,0.0);
        }
    }
    // add edges between x_uv and x_vu with weight w_uv
    for(int u=0; u<n; ++u){
        for(size_t i=0;i<G_orig.adj[u].size(); ++i){
            auto e = G_orig.adj[u][i];
            int v = e.to;
            double w = e.weight;
            // find index of u in v's adjacency to map to map_to_new[v][idx]
            // We must find the corresponding adjacency index j such that adj[v][j].to == u
            int j_found = -1;
            for(size_t j=0;j<G_orig.adj[v].size(); ++j){
                if(G_orig.adj[v][j].to == u){
                    j_found = (int)j; break;
                }
            }
            if(j_found==-1) continue; // should not happen in undirected
            int a = map_to_new[u][i];
            int b = map_to_new[v][j_found];
            // add single undirected edge of weight w between a and b
            Gp.add_edge(a,b,w);
        }
    }
    return Gp;
}