#include "bundle_construct.h"
#include <random>
#include <queue>
#include <limits>
#include <iostream>
using namespace std;
using PQe = pair<double,int>;

/* ================================================================ */
/*  BundleConstruction                                              */
/*                                                                  */
/*  Per the paper (Section 3.1):                                    */
/*    1. Sample R ⊂ V with prob 1/k; source always in R.           */
/*    2. For every non-R vertex v, run truncated Dijkstra from v    */
/*       until the first R-vertex is popped.                        */
/*         - That R-vertex is b(v).                                 */
/*         - All non-R vertices popped before it form Ball(v).      */
/*           b(v) is NOT in Ball(v).                                */
/*         - dist_to_bv[v] = dist(v, b(v)).                        */
/*    3. Bundle(r) = {v : b(v) = r}.                                */
/* ================================================================ */
BundleInfo BundleConstruction(const Graph &G, int s, int k,Profiler *P, int seed)
    {
        int N = G.adj.size();
        const double INF = numeric_limits<double>::infinity();

        BundleInfo B;
        B.N = N;

        B.isR.assign(N, 0);
        B.b.assign(N, -1);
        B.dist_to_bv.assign(N, INF);
        B.ball.assign(N, {});
        B.dist_ball.assign(N, {});
        B.bundles.assign(N, {});

        /* ------ Step 1: Sample R ------ */
            
        minstd_rand rng(seed);

        for(int v = 0; v < N; ++v)
            {
                if(v == s) B.isR[v] = 1;
                else 
                    {
                        uniform_int_distribution<int> D(1, k);
                        if(D(rng) == 1) B.isR[v] = 1;
                    }
                if(B.isR[v]) B.R_list.push_back(v);
            }

        if(P) P->incr("R_size", (long long)B.R_list.size());

        /* ------ Step 2: Compute Ball(v) and b(v) for non-R vertices ------ */

        // Optimization: reuse a single dist[] array, resetting only touched entries
        vector<double> dist(N, INF);
        vector<int> touched;
        touched.reserve(256);

        int processed = 0;

        for(int v = 0; v < N; ++v)
        {
            // R-vertices are their own bundle leader
            if(B.isR[v]){
                B.b[v] = v;
                B.dist_to_bv[v] = 0.0;
                continue;
            }

            processed++;

            // Truncated Dijkstra from v
            dist[v] = 0.0;
            touched.push_back(v);

            priority_queue<PQe, vector<PQe>, greater<PQe>> pq;
            pq.push({0.0, v});

            vector<int>    ball_verts;
            vector<double> ball_dists;

            int   bv      = -1;
            double bv_dist = INF;

            while(!pq.empty())
            {
                auto [d, x] = pq.top(); pq.pop();

                if(d > dist[x]) continue;   // stale entry

                // First R-vertex extracted → b(v) found, stop
                if(B.isR[x]){
                    bv      = x;
                    bv_dist = d;
                    break;
                }

                // x is non-R and d < dist to any R → x ∈ Ball(v)
                if(x != v){  // exclude v itself from Ball(v) since dist(v,v)=0 < dist(v,b(v))
                    ball_verts.push_back(x);
                    ball_dists.push_back(d);
                }

                for(auto &e : G.adj[x]){
                    int y  = e.to;
                    double nd = d + e.weight;
                    if(nd < dist[y]){
                        if(dist[y] == INF)
                            touched.push_back(y);
                        dist[y] = nd;
                        pq.push({nd, y});
                    }
                }
            }

            // Reset touched entries
            for(int idx : touched)
                dist[idx] = INF;
            touched.clear();

            // Fallback: if no R-vertex reachable (should not happen)
            if(bv == -1){
                bv      = B.R_list[0];
                bv_dist = INF;
                cerr << "WARNING: Vertex " << v
                          << " cannot reach any R vertex!\n";
            }

            B.b[v]          = bv;
            B.dist_to_bv[v] = bv_dist;
            B.ball[v]        = move(ball_verts);
            B.dist_ball[v]   = move(ball_dists);

            if(processed % 100000 == 0)
                cerr << "  Ball construction: " << processed << " vertices done\n";
        }

        /* ------ Step 3: Build bundles ------ */

        long long total_bundle = 0;
        for(int v = 0; v < N; ++v){
            int bv = B.b[v];
            if(bv != v){   // v is not its own leader → v ∈ Bundle(bv)
                B.bundles[bv].push_back(v);
                total_bundle++;
            }
        }

        long long total_ball = 0;
        for(int v = 0; v < N; ++v)
            if(!B.isR[v])
                total_ball += B.ball[v].size();

        if(P) P->incr("sum_ball_sizes", total_ball);

        cerr << "Bundle construction done: R=" << B.R_list.size()
                  << " total_ball=" << total_ball
                  << " total_bundle=" << total_bundle << "\n";

        return B;
    }