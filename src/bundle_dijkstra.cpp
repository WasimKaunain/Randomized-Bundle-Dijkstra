#include "bundle_dijkstra.h"
#include <limits>
#include <set>

/*
 * Bundle Dijkstra — Set-based priority queue  (Algorithm 2 from the paper)
 *
 * Only R-vertices live in the priority queue.
 *
 * When R-vertex u is extracted with distance d[u]:
 *
 *   STEP 1 — for every v ∈ Bundle(u):
 *     (a) d[v] = min(d[v], d[u] + dist_to_bv[v])              // u = b(v)
 *     (b) for y ∈ Ball(v): d[v] = min(d[v], d[y]+dist(v,y))
 *     (c) for y ∈ Ball(v), for z ∈ N(y): d[v] = min(d[v], d[z]+w(z,y)+dist(v,y))
 *     (d) d[b(v)] = min(d[b(v)], d[v] + dist_to_bv[v])        // propagate back
 *
 *   STEP 2 — for every x ∈ Bundle(u) ∪ {u}, for every (x,y,w) ∈ E:
 *     d[y] = min(d[y], d[x] + w)
 *     for z ∈ Ball(y): d[z] = min(d[z], d[x] + w + dist(y,z))
 *
 * decrease_key for non-R vertices: just update d[v].
 * decrease_key for R vertices: update d[v] and the set-based heap.
 */

std::vector<double> BundleDijkstra(const Graph &G, int s,
                                    const BundleInfo &B, Profiler * /*P*/)
{
    int N = G.adj.size();
    const double INF = std::numeric_limits<double>::infinity();

    std::vector<double> d(N, INF);
    d[s] = 0.0;

    // Set-based min-heap — only R-vertices
    std::set<std::pair<double,int>> Hset;

    std::vector<std::set<std::pair<double,int>>::iterator> heap_it(N);
    std::vector<char> in_heap(N, 0);

    for(int r : B.R_list){
        auto it = Hset.insert({d[r], r}).first;
        heap_it[r] = it;
        in_heap[r] = 1;
    }

    // decrease_key: always update d[v]; if v is R, update heap
    auto dk = [&](int v, double nd){
        if(nd >= d[v]) return;

        d[v] = nd;

        if(B.isR[v]){
            if(in_heap[v])
                Hset.erase(heap_it[v]);
            heap_it[v] = Hset.insert({nd, v}).first;
            in_heap[v] = 1;
        }
    };

    while(!Hset.empty())
    {
        auto it = Hset.begin();
        auto [du, u] = *it;
        Hset.erase(it);
        in_heap[u] = 0;

        /* =========================================== */
        /*  STEP 1: Process Bundle(u)                  */
        /* =========================================== */
        for(int v : B.bundles[u])
        {
            // (a) d[v] via b(v) = u
            dk(v, d[u] + B.dist_to_bv[v]);

            // (b) d[v] via Ball(v) members y whose d[y] is already known
            for(size_t i = 0; i < B.ball[v].size(); ++i){
                int y = B.ball[v][i];
                if(d[y] < INF)
                    dk(v, d[y] + B.dist_ball[v][i]);
            }

            // (c) d[v] via one-hop: z → y where y ∈ Ball(v), z is any neighbor of y
            for(size_t i = 0; i < B.ball[v].size(); ++i){
                int y = B.ball[v][i];
                double dist_yv = B.dist_ball[v][i];

                for(auto &e : G.adj[y]){
                    int z = e.to;
                    if(d[z] < INF)
                        dk(v, d[z] + e.weight + dist_yv);
                }
            }

            // (d) Propagate back: d[b(v)] via v
            //     b(v) = u, so update d[u]
            dk(u, d[v] + B.dist_to_bv[v]);
        }

        /* =========================================== */
        /*  STEP 2: Relax neighbors of Bundle(u) ∪ {u} */
        /* =========================================== */
        // For every x ∈ Bundle(u) ∪ {u}, relax x's neighbors y and
        // Ball(y) members z.  When any non-R vertex's distance improves,
        // propagate back to its bundle leader.
        auto relax_from = [&](int x){
            for(auto &e : G.adj[x]){
                int y = e.to;
                double w = e.weight;

                double old_dy = d[y];
                dk(y, d[x] + w);

                // If y improved and y is non-R, propagate to b(y)
                if(d[y] < old_dy && !B.isR[y])
                    dk(B.b[y], d[y] + B.dist_to_bv[y]);

                // Also relax Ball(y) members via x → y → z
                for(size_t i = 0; i < B.ball[y].size(); ++i){
                    int z = B.ball[y][i];
                    double old_dz = d[z];
                    dk(z, d[x] + w + B.dist_ball[y][i]);

                    // If z improved and z is non-R, propagate to b(z)
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