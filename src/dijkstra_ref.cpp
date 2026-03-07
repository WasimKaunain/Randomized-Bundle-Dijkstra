#include "dijkstra_ref.h"
#include <queue>
#include <limits>
#include <tuple>

std::vector<double> DijkstraRef(const Graph &G, int s, Profiler *P){
    int n = (int)G.adj.size();
    const double INF = std::numeric_limits<double>::infinity();
    std::vector<double> dist(n, INF);
    dist[s]=0;
    using PQe = std::pair<double,int>;
    std::priority_queue<PQe, std::vector<PQe>, std::greater<PQe>> pq;
    pq.push({0.0,s});
    while(!pq.empty()){
        auto [d,u] = pq.top(); pq.pop();
        if(d != dist[u]) continue;
        for(auto &e : G.adj[u])
        {
            int v = e.to; double nd = d + e.weight;
            if(nd < dist[v]){
                dist[v] = nd;
                pq.push({nd, v});
            }
        }
    }
    return dist;
}
