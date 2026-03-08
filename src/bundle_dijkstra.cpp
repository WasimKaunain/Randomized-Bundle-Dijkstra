#include "bundle_dijkstra.h"
#include <limits>
#include <set>
#include <functional>
#include <iostream>

/*
 * Bundle Dijkstra — Set-based (Algorithm 2 from the paper)
 *
 * Key ideas:
 *   - Only R-vertices are inserted into the priority queue.
 *   - When R-vertex u is extracted (settled), we process Bundle(u):
 *       STEP 1: Relax each v ∈ Bundle(u) using:
 *         (a) d[u] + dist(v, u)            [u = b(v), direct via ball path]
 *         (b) d[y] + dist(v, y)            [y ∈ Ball(v), already settled]
 *         (c) d[z1] + w(z1,z2) + dist(v,z2) [z2 ∈ Ball(v), z1 neighbor of z2]
 *       STEP 2: Relax outgoing neighbors of all bundle members:
 *         For each x ∈ Bundle(u) ∪ {u}, each edge (x,y):
 *           relax y, and relax Ball(y) members via y.
 *
 *   - Ball(v) does NOT contain b(v). We store dist(v, b(v)) separately:
 *     it's the distance from v to its closest R vertex, computed during
 *     bundle construction. We store it in a dedicated field dist_to_bv[v].
 *
 *   - For undirected graphs, dist(v, w) = dist(w, v), so Ball distances
 *     from the construction phase (computed as Dijkstra from v) can be
 *     used in both directions.
 *
 *   - The "decrease_key" for non-R vertex v propagates to b(v):
 *     if d[v] + dist(v, b(v)) < d[b(v)], update b(v) in the heap.
 */

std::vector<double> BundleDijkstra(const Graph &G, int s,
                                    const BundleInfo &B, Profiler *P)
{
    int N = G.adj.size();
    const double INF = std::numeric_limits<double>::infinity();

    std::vector<double> d(N, INF);

    // Set-based heap (distance, vertex) — only R-vertices
    std::set<std::pair<double,int>> Hset;

    // Iterator map for O(log n) decrease-key on R-vertices
    std::vector<std::set<std::pair<double,int>>::iterator> heap_it(N);
    std::vector<char> in_heap(N, 0);

    d[s] = 0.0;

    // Insert only R vertices into heap
    for(int r : B.R_list){
        double key = (r == s) ? 0.0 : INF;
        d[r] = key;
        heap_it[r] = Hset.insert({key, r}).first;
        in_heap[r] = 1;
    }

    // Helper: relax a vertex, propagating to b(v) if non-R
    auto do_relax = [&](int v, double newDist) -> void {
        if(newDist >= d[v]) return;

        d[v] = newDist;

        if(B.isR[v]){
            // Update R-vertex in the heap
            if(in_heap[v]){
                Hset.erase(heap_it[v]);
            }
            heap_it[v] = Hset.insert({newDist, v}).first;
            in_heap[v] = 1;
            if(P) P->incr("decrease_key");
        }
    };

    // Recursive relax: for non-R vertex, also propagate to b(v)
    std::function<void(int, double)> relax = [&](int v, double newDist){
        if(newDist >= d[v]) return;

        do_relax(v, newDist);

        if(!B.isR[v]){
            // Propagate to b(v): dist(s, b(v)) <= dist(s, v) + dist(v, b(v))
            // dist(v, b(v)) was precomputed during bundle construction.
            int bv = B.b[v];
            if(bv >= 0 && bv < N && bv != v){
                double dist_v_bv = B.dist_to_bv[v];
                if(dist_v_bv < INF){
                    double candidate = d[v] + dist_v_bv;
                    if(candidate < d[bv]){
                        do_relax(bv, candidate);
                    }
                }
            }
        }
    };

    while(!Hset.empty())
    {
        auto [du, u] = *Hset.begin();
        Hset.erase(Hset.begin());
        in_heap[u] = 0;

        if(P) P->incr("extracts");

        // Skip stale entries
        if(du > d[u]) continue;

        // ===============================
        // STEP 1: Process Bundle(u)
        // For each v in Bundle(u), try to set d[v]
        // ===============================
        for(int v : B.bundles[u])
        {
            if(v < 0 || v >= N) continue;

            // (a) Relax v via u: d[v] = d[u] + dist(v, b(v))
            //     u = b(v), dist(v, u) = dist_to_bv[v] (precomputed)
            {
                double dist_vu = B.dist_to_bv[v];
                if(dist_vu < INF){
                    relax(v, d[u] + dist_vu);
                }
            }

            // (b) Relax v via Ball(v) members: d[v] = d[y] + dist(y, v) for y ∈ Ball(v)
            //     dist(y, v) = dist(v, y) = dist_ball[v][i] (undirected)
            for(size_t i = 0; i < B.ball[v].size(); ++i){
                int y = B.ball[v][i];
                double dist_vy = B.dist_ball[v][i];
                if(y >= 0 && y < N && d[y] < INF){
                    relax(v, d[y] + dist_vy);
                }
            }

            // (c) Relax v via z1 -> z2 -> v: d[z1] + w(z1,z2) + dist(z2, v)
            //     for z2 ∈ Ball(v), z1 neighbor of z2
            for(size_t i = 0; i < B.ball[v].size(); ++i){
                int z2 = B.ball[v][i];
                if(z2 < 0 || z2 >= N) continue;
                double dist_z2v = B.dist_ball[v][i];

                for(auto &e : G.adj[z2]){
                    int z1 = e.to;
                    if(z1 >= 0 && z1 < N && d[z1] < INF){
                        relax(v, d[z1] + e.weight + dist_z2v);
                    }
                }
            }
        }

        // ===============================
        // STEP 2: Relax outgoing neighbors of u and its bundle
        // ===============================

        // Process u itself
        for(auto &e : G.adj[u]){
            int y = e.to;
            if(y < 0 || y >= N) continue;
            relax(y, d[u] + e.weight);

            // Also relax Ball(y) members via y
            for(size_t i = 0; i < B.ball[y].size(); ++i){
                int z = B.ball[y][i];
                if(z >= 0 && z < N){
                    double dist_yz = B.dist_ball[y][i];
                    relax(z, d[u] + e.weight + dist_yz);
                }
            }
        }

        // Process bundle members of u
        for(int x : B.bundles[u])
        {
            if(x < 0 || x >= N) continue;
            if(d[x] >= INF) continue;  // x not yet reached

            for(auto &e : G.adj[x])
            {
                int y = e.to;
                if(y < 0 || y >= N) continue;

                // Relax y via x
                relax(y, d[x] + e.weight);

                // Relax Ball(y) members via x -> y -> z
                for(size_t i = 0; i < B.ball[y].size(); ++i){
                    int z = B.ball[y][i];
                    if(z >= 0 && z < N){
                        double dist_yz = B.dist_ball[y][i];
                        relax(z, d[x] + e.weight + dist_yz);
                    }
                }
            }
        }
    }

    return d;
}