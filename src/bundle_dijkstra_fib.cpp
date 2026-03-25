#include <boost/heap/fibonacci_heap.hpp>
#include "bundle_dijkstra.h"
#include "dijkstra_fibheap.h"
#include <limits>

/*
 * Bundle Dijkstra — Fibonacci heap version  (Algorithm 2 from the paper)
 *
 * Same algorithm as the set-based version, but uses Boost Fibonacci heap
 * for O(1) amortized decrease-key on R-vertices.
 */

std::vector<double> BundleDijkstra_Fib(
    const Graph &G, int s, const BundleInfo &B, Profiler * /*P*/)
{
    int N = G.adj.size();
    const double INF = std::numeric_limits<double>::infinity();

    std::vector<double> d(N, INF);
    d[s] = 0.0;

    using FibHeap = boost::heap::fibonacci_heap<
        Node, boost::heap::compare<std::greater<Node>>>;

    FibHeap heap;
    std::vector<FibHeap::handle_type> handles(N);
    std::vector<char> in_heap(N, 0);

    // Insert all R-vertices
    for(int r : B.R_list){
        handles[r] = heap.push({r, d[r]});
        in_heap[r] = 1;
    }

    // decrease_key: always update d[v]; if v is R, update Fib heap
    auto dk = [&](int v, double nd){
        if(nd >= d[v]) return;

        d[v] = nd;

        if(B.isR[v]){
            if(in_heap[v]){
                heap.update(handles[v], {v, nd});
            } else {
                handles[v] = heap.push({v, nd});
                in_heap[v] = 1;
            }
        }
    };

    while(!heap.empty())
    {
        Node top = heap.top();
        heap.pop();

        int u = top.vertex;
        in_heap[u] = 0;

        // Skip stale (already processed with a better distance)
        if(top.dist > d[u]) continue;

        /* =========================================== */
        /*  STEP 1: Process Bundle(u)                  */
        /* =========================================== */
        for(int v : B.bundles[u])
        {
            // (a) d[v] via b(v) = u
            dk(v, d[u] + B.dist_to_bv[v]);

            // (b) d[v] via Ball(v) members
            for(size_t i = 0; i < B.ball[v].size(); ++i){
                int y = B.ball[v][i];
                if(d[y] < INF)
                    dk(v, d[y] + B.dist_ball[v][i]);
            }

            // (c) d[v] via one-hop: z → y, y ∈ Ball(v)
            for(size_t i = 0; i < B.ball[v].size(); ++i){
                int y = B.ball[v][i];
                double dist_yv = B.dist_ball[v][i];

                for(auto &e : G.adj[y]){
                    int z = e.to;
                    if(d[z] < INF)
                        dk(v, d[z] + e.weight + dist_yv);
                }
            }

            // (d) Propagate back to b(v) = u
            dk(u, d[v] + B.dist_to_bv[v]);
        }

        /* =========================================== */
        /*  STEP 2: Relax neighbors of Bundle(u) ∪ {u} */
        /* =========================================== */
        auto relax_from = [&](int x){
            for(auto &e : G.adj[x]){
                int y = e.to;
                double w = e.weight;

                double old_dy = d[y];
                dk(y, d[x] + w);

                if(d[y] < old_dy && !B.isR[y])
                    dk(B.b[y], d[y] + B.dist_to_bv[y]);

                for(size_t i = 0; i < B.ball[y].size(); ++i){
                    int z = B.ball[y][i];
                    double old_dz = d[z];
                    dk(z, d[x] + w + B.dist_ball[y][i]);

                    if(d[z] < old_dz && !B.isR[z])
                        dk(B.b[z], d[z] + B.dist_to_bv[z]);
                }
            }
        };

        relax_from(u);
        for(int x : B.bundles[u])
            relax_from(x);
    }

    return d;
}