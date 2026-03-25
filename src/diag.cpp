/*
 * Diagnostic tool: trace the shortest path to a specific target node,
 * comparing Bundle Dijkstra vs Reference Dijkstra.
 * 
 * Prints the shortest-path tree predecessors and checks where Bundle
 * diverges from the reference.
 */

#include "graph.h"
#include "profiler.h"
#include "dijkstra_ref.h"
#include "bundle_construct.h"
#include "bundle_dijkstra.h"
#include "dijkstra_fibheap.h"

#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <cmath>
#include <set>
#include <functional>

// Reference Dijkstra with predecessor tracking
std::pair<std::vector<double>, std::vector<int>>
DijkstraRefWithPred(const Graph &G, int s)
{
    int N = G.adj.size();
    const double INF = std::numeric_limits<double>::infinity();
    std::vector<double> d(N, INF);
    std::vector<int> pred(N, -1);
    
    using PQe = std::pair<double, int>;
    std::priority_queue<PQe, std::vector<PQe>, std::greater<PQe>> pq;
    d[s] = 0.0;
    pq.push({0.0, s});
    
    while (!pq.empty()) {
        auto [du, u] = pq.top(); pq.pop();
        if (du > d[u]) continue;
        for (auto &e : G.adj[u]) {
            double nd = du + e.weight;
            if (nd < d[e.to]) {
                d[e.to] = nd;
                pred[e.to] = u;
                pq.push({nd, e.to});
            }
        }
    }
    return {d, pred};
}

int main(int argc, char** argv)
{
    if (argc < 4) {
        std::cerr << "Usage: diag <graph> <seed> <target>\n";
        return 1;
    }
    
    std::string graph_file = argv[1];
    int seed = atoi(argv[2]);
    int target = atoi(argv[3]);
    int source = 0;
    
    Graph G = Graph::load_edge_list(graph_file);
    int N = G.adj.size();
    
    int k = (int)std::ceil(
        std::sqrt(std::log((double)N) / std::log(std::log((double)N)))
    );
    
    std::cerr << "N=" << N << " k=" << k << "\n";
    
    // Build bundles
    Profiler P;
    BundleInfo B = BundleConstruction(G, source, k, &P, seed);
    
    // Reference Dijkstra with predecessors
    auto [d_ref, pred] = DijkstraRefWithPred(G, source);
    
    // Bundle Dijkstra
    auto d_bundle = BundleDijkstra(G, source, B, nullptr);
    
    std::cerr << "\n=== Target node " << target << " ===\n";
    std::cerr << "ref dist  = " << d_ref[target] << "\n";
    std::cerr << "bundle dist = " << d_bundle[target] << "\n";
    std::cerr << "isR[target] = " << (int)B.isR[target] << "\n";
    std::cerr << "b[target]   = " << B.b[target] << "\n";
    std::cerr << "dist_to_bv  = " << B.dist_to_bv[target] << "\n";
    std::cerr << "Ball(target) size = " << B.ball[target].size() << "\n";
    
    // Print shortest path from source to target (ref)
    std::cerr << "\n--- Reference shortest path (source -> target) ---\n";
    std::vector<int> path;
    for (int v = target; v != -1; v = pred[v])
        path.push_back(v);
    std::reverse(path.begin(), path.end());
    
    for (size_t i = 0; i < path.size(); ++i) {
        int v = path[i];
        std::cerr << "  " << v 
                  << " [ref=" << d_ref[v] 
                  << " bundle=" << d_bundle[v]
                  << " isR=" << (int)B.isR[v]
                  << " b=" << B.b[v]
                  << "]";
        if (d_bundle[v] != d_ref[v])
            std::cerr << " *** WRONG ***";
        std::cerr << "\n";
    }
    
    // Find first node on path where bundle diverges
    std::cerr << "\n--- First divergence point ---\n";
    for (size_t i = 0; i < path.size(); ++i) {
        int v = path[i];
        if (std::abs(d_bundle[v] - d_ref[v]) > 1e-8) {
            std::cerr << "First wrong node: " << v << "\n";
            std::cerr << "  ref=" << d_ref[v] << " bundle=" << d_bundle[v] << "\n";
            std::cerr << "  isR=" << (int)B.isR[v] << " b(v)=" << B.b[v] << "\n";
            
            if (i > 0) {
                int prev = path[i-1];
                std::cerr << "  predecessor on ref path: " << prev << "\n";
                std::cerr << "  ref[prev]=" << d_ref[prev] 
                          << " bundle[prev]=" << d_bundle[prev] << "\n";
                std::cerr << "  prev isR=" << (int)B.isR[prev] 
                          << " b(prev)=" << B.b[prev] << "\n";
                
                // Find the edge weight
                for (auto &e : G.adj[prev]) {
                    if (e.to == v) {
                        std::cerr << "  edge weight=" << e.weight << "\n";
                        std::cerr << "  bundle[prev] + w = " << d_bundle[prev] + e.weight << "\n";
                        break;
                    }
                }
                
                // Check: is prev in Ball(v)? 
                bool prev_in_ball = false;
                for (size_t j = 0; j < B.ball[v].size(); ++j) {
                    if (B.ball[v][j] == prev) {
                        prev_in_ball = true;
                        std::cerr << "  prev IN Ball(v), dist_ball=" << B.dist_ball[v][j] << "\n";
                    }
                }
                if (!prev_in_ball)
                    std::cerr << "  prev NOT in Ball(v)\n";
                
                // Check: is v in Ball(prev)?
                bool v_in_ball_prev = false;
                for (size_t j = 0; j < B.ball[prev].size(); ++j) {
                    if (B.ball[prev][j] == v) {
                        v_in_ball_prev = true;
                        std::cerr << "  v IN Ball(prev), dist_ball=" << B.dist_ball[prev][j] << "\n";
                    }
                }
                if (!v_in_ball_prev)
                    std::cerr << "  v NOT in Ball(prev)\n";
                    
                // Check: is v in Bundle(b(v))?
                int bv = B.b[v];
                bool found = false;
                for (int x : B.bundles[bv]) {
                    if (x == v) { found = true; break; }
                }
                std::cerr << "  v in Bundle(b(v))=" << found << "\n";
                
                // Check: is prev in Bundle(b(prev))?
                int bp = B.b[prev];
                found = false;
                for (int x : B.bundles[bp]) {
                    if (x == prev) { found = true; break; }
                }
                std::cerr << "  prev in Bundle(b(prev))=" << found << "\n";
                
                // Was b(prev) settled before b(v)?
                std::cerr << "  d_bundle[b(prev)]=" << d_bundle[bp] << "\n";
                std::cerr << "  d_bundle[b(v)]=" << d_bundle[bv] << "\n";
            }
            break;
        }
    }
    
    return 0;
}
