#include "bundle_construct.h"
#include <random>
#include <queue>
#include <limits>

using PQe = std::pair<double,int>;

BundleInfo BundleConstruction(const Graph &G, int s, int k,
                               Profiler *P, int seed)
{
    int N = G.adj.size();

    BundleInfo B;
    B.N = N;

    B.isR.assign(N, 0);
    B.b.assign(N, -1);

    B.ball.assign(N, {});
    B.dist_ball.assign(N, {});
    B.bundles.assign(N, {});

    std::minstd_rand rng(seed);

    // Sample R
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

    if(P) P->incr("R_size", B.R_list.size());

    const double INF = std::numeric_limits<double>::infinity();

    // For each non-R vertex
    for(int v=0; v<N; ++v)
    {
        if(B.isR[v]){
            B.b[v] = v;
            continue;
        }

        std::vector<double> dist(N, INF);
        std::vector<char> visited(N, 0);

        std::priority_queue<PQe, std::vector<PQe>, std::greater<PQe>> pq;

        dist[v] = 0;
        pq.push({0, v});

        std::vector<int> extracted;

        while(!pq.empty())
        {
            auto [d,x] = pq.top(); pq.pop();
            if(visited[x]) continue;
            visited[x] = 1;

            extracted.push_back(x);

            if(B.isR[x]) break;

            for(auto &e : G.adj[x]){
                int y = e.to;
                double nd = d + e.weight;
                if(nd < dist[y]){
                    dist[y] = nd;
                    pq.push({nd,y});
                }
            }
        }

        // find nearest R
        int bv = -1;
        for(int x : extracted){
            if(B.isR[x]){
                bv = x;
                break;
            }
        }

        if(bv == -1)
            bv = extracted.back();

        B.b[v] = bv;

        double threshold = dist[bv];

        for(int x : extracted){
            if(x==bv) continue;
            if(dist[x] < threshold){
                B.ball[v].push_back(x);
                B.dist_ball[v].push_back(dist[x]);
            }
        }

        // include b(v) at end
        B.ball[v].push_back(bv);
        B.dist_ball[v].push_back(dist[bv]);
    }

    // Build bundles
    for(int v=0; v<N; ++v){
        int bv = B.b[v];
        B.bundles[bv].push_back(v);
    }

    return B;
}