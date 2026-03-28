#include "bundle_dijkstra.h"
#include <queue>
#include <limits>
using namespace std;

/*
 * Bundle Dijkstra — Binary Heap (priority_queue) version
 *
 * Uses lazy decrease-key:
 *  - multiple entries allowed
 *  - stale entries skipped during pop
 *
 * Only R-vertices are inserted into heap.
 */

vector<double> BundleDijkstra_PQ(const Graph &G, int s, const BundleInfo &B, Profiler *P)
{
    int N = G.adj.size();
    const double INF = numeric_limits<double>::infinity();

    vector<double> d(N, INF);
    d[s] = 0.0;

    // Counters (same style as your other versions)
    long long cnt_extract = 0;
    long long cnt_dk = 0;
    long long cnt_relax_edge = 0;
    long long cnt_ball_access = 0;

    using PQ = priority_queue<pair<double,int>, vector<pair<double,int>>, greater<>>;
    PQ pq;

    // Insert all R vertices
    for(int r : B.R_list){
        pq.push({d[r], r});
    }

    // Lazy decrease-key
    auto dk = [&](int v, double nd){
        if(nd >= d[v]) return;

        cnt_dk++;

        d[v] = nd;

        if(B.isR[v]){
            pq.push({nd, v});   // lazy insert
        }
    };

    while(!pq.empty())
    {
        auto [du, u] = pq.top();
        pq.pop();

        cnt_extract++;

        // Skip stale entries
        if(du != d[u]) continue;

        /* =========================================== */
        /* STEP 1: Process Bundle(u)                   */
        /* =========================================== */
        for(int v : B.bundles[u])
        {
            // (a)
            dk(v, d[u] + B.dist_to_bv[v]);

            // (b)
            for(size_t i = 0; i < B.ball[v].size(); ++i){
                cnt_ball_access++;

                int y = B.ball[v][i];
                if(d[y] < INF)
                    dk(v, d[y] + B.dist_ball[v][i]);
            }

            // (c)
            for(size_t i = 0; i < B.ball[v].size(); ++i){
                cnt_ball_access++;

                int y = B.ball[v][i];
                double dist_yv = B.dist_ball[v][i];

                for(auto &e : G.adj[y]){
                    cnt_relax_edge++;

                    int z = e.to;
                    if(d[z] < INF)
                        dk(v, d[z] + e.weight + dist_yv);
                }
            }

            // (c-extra)
            for(auto &e : G.adj[v]){
                cnt_relax_edge++;

                int z = e.to;
                if(d[z] < INF)
                    dk(v, d[z] + e.weight);
            }
        }

        /* =========================================== */
        /* STEP 2: Relax neighbors                     */
        /* =========================================== */
        auto relax_from = [&](int x){
            for(auto &e : G.adj[x]){
                cnt_relax_edge++;

                int y = e.to;
                double w = e.weight;

                double old_dy = d[y];
                dk(y, d[x] + w);

                if(d[y] < old_dy && !B.isR[y])
                    dk(B.b[y], d[y] + B.dist_to_bv[y]);

                for(size_t i = 0; i < B.ball[y].size(); ++i){
                    cnt_ball_access++;

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

    // Store counters
    if(P){
        P->incr("pq_extract", cnt_extract);
        P->incr("pq_dk", cnt_dk);
        P->incr("pq_edge_relax", cnt_relax_edge);
        P->incr("pq_ball_access", cnt_ball_access);
    }

    return d;
}