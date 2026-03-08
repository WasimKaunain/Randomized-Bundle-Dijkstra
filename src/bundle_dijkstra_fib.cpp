#include <boost/heap/fibonacci_heap.hpp>
#include "bundle_dijkstra.h"
#include "dijkstra_fibheap.h"
#include <limits>
#include <functional>
#include <iostream>

/*
 * Bundle Dijkstra — Fibonacci heap version (Algorithm 2 from the paper)
 *
 * Same algorithm as the set-based version, but uses Boost Fibonacci heap
 * for O(1) amortized decrease-key on R-vertices.
 *
 * Only R-vertices are in the heap.
 * Ball(v) does NOT contain b(v).
 */

std::vector<double> BundleDijkstra_Fib(
    const Graph &G,
    int s,
    const BundleInfo &B,
    Profiler *P)
{
    int n = G.adj.size();
    const double INF = std::numeric_limits<double>::infinity();

    std::vector<double> dist(n, INF);

    using FibHeap =
        boost::heap::fibonacci_heap<
            Node,
            boost::heap::compare<std::greater<Node>>
        >;

    FibHeap heap;
    std::vector<FibHeap::handle_type> handles(n);
    std::vector<char> in_heap(n, 0);

    dist[s] = 0.0;

    // Insert only R vertices into heap
    for (int r : B.R_list) {
        double key = (r == s) ? 0.0 : INF;
        dist[r] = key;
        handles[r] = heap.push({r, key});
        in_heap[r] = 1;
    }

    // Helper: relax an R-vertex in the heap
    auto do_relax = [&](int v, double newDist) {
        if(newDist >= dist[v]) return;

        dist[v] = newDist;

        if(B.isR[v]){
            if(in_heap[v]){
                heap.update(handles[v], {v, newDist});
            } else {
                handles[v] = heap.push({v, newDist});
                in_heap[v] = 1;
            }
            if(P) P->incr("decrease_key");
        }
    };

    // Relax with propagation to b(v) for non-R vertices
    std::function<void(int, double)> relax = [&](int v, double newDist){
        if(newDist >= dist[v]) return;

        do_relax(v, newDist);

        if(!B.isR[v]){
            // Propagate to b(v): dist(s, b(v)) <= dist(s, v) + dist(v, b(v))
            // dist(v, b(v)) was precomputed during bundle construction.
            int bv = B.b[v];
            if(bv >= 0 && bv < n && bv != v){
                double dist_v_bv = B.dist_to_bv[v];
                if(dist_v_bv < INF){
                    double candidate = dist[v] + dist_v_bv;
                    if(candidate < dist[bv]){
                        do_relax(bv, candidate);
                    }
                }
            }
        }
    };

    while (!heap.empty()) {
        Node uNode = heap.top();
        heap.pop();

        int u = uNode.vertex;
        double du = uNode.dist;
        in_heap[u] = 0;

        if (P) P->incr("extracts");

        // Skip stale entries
        if(du > dist[u]) continue;

        // ===============================
        // STEP 1: Process Bundle(u)
        // ===============================
        for (int v : B.bundles[u]) {
            if(v < 0 || v >= n) continue;

            // (a) Relax v via u: d[v] = d[u] + dist(v, b(v))
            //     u = b(v), dist(v, u) = dist_to_bv[v] (precomputed)
            {
                double dist_vu = B.dist_to_bv[v];
                if(dist_vu < INF){
                    relax(v, dist[u] + dist_vu);
                }
            }

            // (b) Relax v via Ball(v): d[v] = d[y] + dist(y, v) for y ∈ Ball(v)
            for (size_t i = 0; i < B.ball[v].size(); ++i) {
                int y = B.ball[v][i];
                double dist_vy = B.dist_ball[v][i];
                if(y >= 0 && y < n && dist[y] < INF){
                    relax(v, dist[y] + dist_vy);
                }
            }

            // (c) Relax v via z1 -> z2 -> v for z2 ∈ Ball(v)
            for (size_t i = 0; i < B.ball[v].size(); ++i) {
                int z2 = B.ball[v][i];
                if(z2 < 0 || z2 >= n) continue;
                double dist_z2v = B.dist_ball[v][i];

                for (auto &e : G.adj[z2]) {
                    int z1 = e.to;
                    if(z1 >= 0 && z1 < n && dist[z1] < INF){
                        relax(v, dist[z1] + e.weight + dist_z2v);
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
            if(y < 0 || y >= n) continue;
            relax(y, dist[u] + e.weight);

            for(size_t i = 0; i < B.ball[y].size(); ++i){
                int z = B.ball[y][i];
                if(z >= 0 && z < n){
                    double dist_yz = B.dist_ball[y][i];
                    relax(z, dist[u] + e.weight + dist_yz);
                }
            }
        }

        // Process bundle members
        for (int x : B.bundles[u]) {
            if(x < 0 || x >= n) continue;
            if(dist[x] >= INF) continue;

            for (auto &e : G.adj[x]) {
                int y = e.to;
                if(y < 0 || y >= n) continue;

                relax(y, dist[x] + e.weight);

                for (size_t i = 0; i < B.ball[y].size(); ++i) {
                    int z = B.ball[y][i];
                    if(z >= 0 && z < n){
                        double dist_yz = B.dist_ball[y][i];
                        relax(z, dist[x] + e.weight + dist_yz);
                    }
                }
            }
        }
    }

    return dist;
}