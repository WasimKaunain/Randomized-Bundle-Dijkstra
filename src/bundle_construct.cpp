#include "bundle_construct.h"
#include <random>
#include <queue>
#include <limits>
#include <iostream>

using PQe = std::pair<double,int>;

BundleInfo BundleConstruction(const Graph &G, int s, int k,
                               Profiler *P, int seed)
{
    int N = G.adj.size();

    BundleInfo B;
    B.N = N;

    B.isR.assign(N, 0);
    B.b.assign(N, -1);
    B.dist_to_bv.assign(N, std::numeric_limits<double>::infinity());

    B.ball.assign(N, {});
    B.dist_ball.assign(N, {});
    B.bundles.assign(N, {});

    std::minstd_rand rng(seed);

    // Sample R: Independently sample each vertex v ∈ V \ {s} with probability 1/k
    for(int v=0; v<N; ++v){
        if(v==s){
            B.isR[v] = 1;
        } else {
            std::uniform_int_distribution<int> D(1,k);
            if(D(rng)==1)
                B.isR[v] = 1;
        }

        if(B.isR[v])
            B.R_list.push_back(v);
    }

    if(P) P->incr("R_size", (long long)B.R_list.size());

    const double INF = std::numeric_limits<double>::infinity();

    int processed = 0;

    // For each non-R vertex, compute Ball(v) and b(v).
    //
    // Per the paper (Section 3.1):
    //   Run Dijkstra from v, stop when first R-vertex is extracted.
    //   That R-vertex is b(v). All non-R vertices extracted before it
    //   (except v itself) form Ball(v).
    //   dist(v, b(v)) is stored in dist_to_bv[v].
    //
    // Optimization: allocate dist[] once, filled with INF. After each
    // truncated Dijkstra, reset only the O(k) entries we touched.
    // This avoids O(N) re-initialization per vertex.
    std::vector<double> dist(N, INF);
    std::vector<int>    touched;          // indices we modified in dist[]
    touched.reserve(256);

    for(int v=0; v<N; ++v)
    {
        if(B.isR[v]){
            B.b[v] = v;
            B.dist_to_bv[v] = 0.0;
            continue;
        }

        processed++;

        // Seed source vertex
        dist[v] = 0.0;
        touched.push_back(v);

        std::priority_queue<PQe, std::vector<PQe>, std::greater<PQe>> pq;
        pq.push({0.0, v});

        int bv = -1;
        double bv_dist = INF;

        std::vector<int>    ball_verts;
        std::vector<double> ball_dists;

        while(!pq.empty())
        {
            auto [d, x] = pq.top(); pq.pop();

            // Skip stale entries
            if(d > dist[x]) continue;

            // First R vertex extracted => this is b(v), stop
            if(B.isR[x]){
                bv = x;
                bv_dist = d;
                break;
            }

            // x is non-R and closer than any R vertex => x ∈ Ball(v)
            if(x != v){
                ball_verts.push_back(x);
                ball_dists.push_back(d);
            }

            for(auto &e : G.adj[x]){
                int y = e.to;
                double nd = d + e.weight;
                if(nd < dist[y]){
                    if(dist[y] == INF){
                        touched.push_back(y);   // first time touching y
                    }
                    dist[y] = nd;
                    pq.push({nd, y});
                }
            }
        }

        // Reset only the entries we touched — O(|touched|) = O(k) on average
        for(int idx : touched){
            dist[idx] = INF;
        }
        touched.clear();

        // Fallback (disconnected graph — should not happen)
        if(bv == -1){
            bv = B.R_list[0];
            bv_dist = INF;
            std::cerr << "WARNING: Vertex " << v
                      << " cannot reach any R vertex!\n";
        }

        B.b[v] = bv;
        B.dist_to_bv[v] = bv_dist;
        B.ball[v]      = std::move(ball_verts);
        B.dist_ball[v] = std::move(ball_dists);

        if(processed % 50000 == 0){
            std::cerr << "  Processed " << processed << " non-R vertices\n";
        }
    }

    std::cerr << "Bundle construction: processed " << processed << " non-R vertices, "
              << "R_size=" << B.R_list.size() << "\n";

    // Build bundles: Bundle(u) = {v : b(v) == u}
    long long total_bundle_size = 0;
    for(int v=0; v<N; ++v){
        int bv = B.b[v];
        if(bv != v){
            if(bv < 0 || bv >= N){
                std::cerr << "ERROR: Vertex " << v << " has invalid b(v)=" << bv << "\n";
            } else {
                B.bundles[bv].push_back(v);
                total_bundle_size++;
            }
        }
    }

    std::cerr << "Total vertices in bundles: " << total_bundle_size << "\n";

    // Sanity check
    long long total_ball_size = 0;
    for(int v=0; v<N; ++v){
        if(!B.isR[v]){
            total_ball_size += B.ball[v].size();
        }
    }
    std::cerr << "Total Ball vertices: " << total_ball_size << "\n";

    if(P) P->incr("sum_ball_sizes", total_ball_size);

    return B;
}