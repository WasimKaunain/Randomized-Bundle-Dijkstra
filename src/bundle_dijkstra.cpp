#include "bundle_dijkstra.h"
#include <limits>
#include <set>
using namespace std;

vector<double> BundleDijkstra(const Graph &G, int s, const BundleInfo &B, Profiler *P)
{
    int N = G.adj.size();
    const double INF = numeric_limits<double>::infinity();

    vector<double> d(N, INF);
    d[s] = 0.0;

    // Counters
    long long cnt_extract = 0;
    long long cnt_dk = 0;
    long long cnt_relax_edge = 0;
    long long cnt_ball_access = 0;

    set<pair<double,int>> Hset;
    vector<set<pair<double,int>>::iterator> heap_it(N);
    vector<char> in_heap(N, 0);

    // Insert only R vertices into heap
    for(int r : B.R_list)
        {
            auto it = Hset.insert({d[r], r}).first;
            heap_it[r] = it;
            in_heap[r] = 1;
        }


    auto Relax = [&](auto&& self, int v, double D) -> void
        {
            if(D >= d[v]) return;

            d[v] = D;
            cnt_dk++;

            if(B.isR[v])
                {
                    if(in_heap[v])
                        Hset.erase(heap_it[v]);
                    heap_it[v] = Hset.insert({D, v}).first;
                    in_heap[v] = 1;
                }
            else self(self, B.b[v], d[v] + B.dist_to_bv[v]);
        };

    while(!Hset.empty())
        {
            auto it = Hset.begin();
            auto [du, u] = *it;
            Hset.erase(it);
            in_heap[u] = 0;

            cnt_extract++;

            /* ===================== STEP 1 ===================== */
            for(int v : B.bundles[u])
                {
                    // Relax(v, d(u) + dist(u,v))
                    Relax(Relax, v, d[u] + B.dist_to_bv[v]);

                    // Relax via Ball(v)
                    for(size_t i = 0; i < B.ball[v].size(); ++i)
                        {
                            cnt_ball_access++;
                        
                            int y = B.ball[v][i];
                            Relax(Relax, v, d[y] + B.dist_ball[v][i]);
                        }

                    // z2 in Ball(v)
                    for(size_t i = 0; i < B.ball[v].size(); ++i)
                        {
                            cnt_ball_access++;

                            int z2 = B.ball[v][i];
                            double dist_z2v = B.dist_ball[v][i];

                            for(auto &e : G.adj[z2])
                                {
                                    cnt_relax_edge++;
                                
                                    int z1 = e.to;
                                    Relax(Relax, v, d[z1] + e.weight + dist_z2v);
                                }
                        }

                    // z2 = v case
                    for(auto &e : G.adj[v])
                        {
                            cnt_relax_edge++;

                            int z1 = e.to;
                            Relax(Relax, v, d[z1] + e.weight);
                        }
                }

            /* ===================== STEP 2 ===================== */

            // First process u itself
            for(auto &e : G.adj[u])
            {
                cnt_relax_edge++;

                int y = e.to;
                double w = e.weight;

                Relax(Relax, y, d[u] + w);

                for(size_t i = 0; i < B.ball[y].size(); ++i)
                {
                    cnt_ball_access++;

                    int z = B.ball[y][i];
                    double dist_yz = B.dist_ball[y][i];

                    Relax(Relax, z, d[u] + w + dist_yz);
                }
            }

            // Then process bundles
            for(int x : B.bundles[u])
            {
                for(auto &e : G.adj[x])
                {
                    cnt_relax_edge++;

                    int y = e.to;
                    double w = e.weight;

                    Relax(Relax, y, d[x] + w);

                    for(size_t i = 0; i < B.ball[y].size(); ++i)
                    {
                        cnt_ball_access++;

                        int z = B.ball[y][i];
                        double dist_yz = B.dist_ball[y][i];

                        Relax(Relax, z, d[x] + w + dist_yz);
                    }
                }
            }
        }

    // Store counters
    if(P)
        {
            P->incr("set_extract", cnt_extract);
            P->incr("set_dk", cnt_dk);
            P->incr("set_edge_relax", cnt_relax_edge);
            P->incr("set_ball_access", cnt_ball_access);
        }

    return d;
}