#include "bundle_dijkstra.h"
#include <limits>
#include <set>

// Optimized set-based heap with iterator tracking
std::vector<double> BundleDijkstra(const Graph &G,int s,const BundleInfo &B,Profiler *P)
{
    int N = G.adj.size();
    const double INF = std::numeric_limits<double>::infinity();

    std::vector<double> d(N, INF);
    d[s] = 0.0;

    // Set-based heap (distance, vertex)
    std::set<std::pair<double,int>> Hset;

    // Iterator map for O(log n) decrease-key
    std::vector<std::set<std::pair<double,int>>::iterator> heap_it(N);
    std::vector<char> inHeap(N, 0);
    // Insert only R vertices
    for(int r : B.R_list){
        double key = (r == s) ? 0.0 : INF;
        d[r] = key;
        heap_it[r] = Hset.insert({key, r}).first;
        inHeap[r] = 1;
    }

    auto decrease_key = [&](int v, double newDist){
        if(newDist >= d[v]) return;

        d[v] = newDist;

        if(B.isR[v]){
            if(inHeap[v]){
            Hset.erase(heap_it[v]);}
            heap_it[v] = Hset.insert({newDist, v}).first;
            inHeap[v]=1;
            if(P) P->incr("decrease_key");
        }
    };

    while(!Hset.empty())
    {
        auto [du, u] = *Hset.begin();
        inHeap[u]=0;
        Hset.erase(Hset.begin());

        if(P) P->incr("extracts");

        // ===============================
        // STEP 1: Process Bundle(u)
        // ===============================
        for(int v : B.bundles[u])
        {
            // Find dist(u,v) from ball structure
            double duv = INF;

            for(size_t i = 0; i < B.ball[v].size(); ++i){
                if(B.ball[v][i] == u){
                    duv = B.dist_ball[v][i];
                    break;
                }
            }

            if(duv < INF)
                decrease_key(v, d[u] + duv);

            // Relax via Ball(v)
            for(size_t i = 0; i < B.ball[v].size(); ++i){
                int y = B.ball[v][i];
                double distvy = B.dist_ball[v][i];

                if(d[y] < INF)
                    decrease_key(v, d[y] + distvy);
            }

            // Relax via neighbors of Ball(v)
            for(size_t i = 0; i < B.ball[v].size(); ++i){
                int z2 = B.ball[v][i];
                double distz2v = B.dist_ball[v][i];

                for(auto &e : G.adj[z2]){
                    int z1 = e.to;
                    double w = e.weight;

                    if(d[z1] < INF)
                        decrease_key(v, d[z1] + w + distz2v);
                }
            }

            // Relax b(v)
            int bv = B.b[v];
            if(bv != v){
                double dvbv = INF;
                for(size_t i = 0; i < B.ball[v].size(); ++i){
                    if(B.ball[v][i] == bv){
                        dvbv = B.dist_ball[v][i];
                        break;
                    }
                }

                if(dvbv < INF)
                    decrease_key(bv, d[v] + dvbv);
            }
        }

        // ===============================
        // STEP 2: Process neighbors
        // ===============================
        for(int x : B.bundles[u])
        {
            for(auto &e : G.adj[x])
            {
                int y = e.to;
                double w = e.weight;

                decrease_key(y, d[x] + w);

                // Relax Ball(y)
                for(size_t i = 0; i < B.ball[y].size(); ++i){
                    int z1 = B.ball[y][i];
                    double distyz1 = B.dist_ball[y][i];

                    decrease_key(z1, d[x] + w + distyz1);
                }
            }
        }
    }

    return d;
}