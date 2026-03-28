#include <boost/heap/fibonacci_heap.hpp>
#include "bundle_dijkstra.h"
#include "dijkstra_fibheap.h"
#include <limits>
using namespace std;

vector<double> BundleDijkstra_Fib(const Graph &G, int s, const BundleInfo &B, Profiler *P)
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

    using FibHeap = boost::heap::fibonacci_heap<
        Node, boost::heap::compare<greater<Node>>>;

    FibHeap heap;
    vector<FibHeap::handle_type> handles(N);
    vector<char> in_heap(N, 0);

    // Insert all R-vertices
    for(int r : B.R_list){
        handles[r] = heap.push({r, d[r]});
        in_heap[r] = 1;
    }

    // decrease_key
    auto dk = [&](int v, double nd){
        if(nd >= d[v]) return;

        cnt_dk++;   // 👈 count decrease-key

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

        cnt_extract++;   // 👈 count extract-min

        int u = top.vertex;
        in_heap[u] = 0;

        if(top.dist > d[u]) continue;

        /* STEP 1 */
        for(int v : B.bundles[u])
        {
            dk(v, d[u] + B.dist_to_bv[v]);

            for(size_t i = 0; i < B.ball[v].size(); ++i){
                cnt_ball_access++;   // 👈

                int y = B.ball[v][i];
                if(d[y] < INF)
                    dk(v, d[y] + B.dist_ball[v][i]);
            }

            for(size_t i = 0; i < B.ball[v].size(); ++i){
                cnt_ball_access++;

                int y = B.ball[v][i];
                double dist_yv = B.dist_ball[v][i];

                for(auto &e : G.adj[y]){
                    cnt_relax_edge++;   // 👈

                    int z = e.to;
                    if(d[z] < INF)
                        dk(v, d[z] + e.weight + dist_yv);
                }
            }

            // z2 = v
            for(auto &e : G.adj[v]){
                cnt_relax_edge++;

                int z = e.to;
                if(d[z] < INF)
                    dk(v, d[z] + e.weight);
            }
        }

        /* STEP 2 */
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
        P->incr("fib_extract", cnt_extract);
        P->incr("fib_dk", cnt_dk);
        P->incr("fib_edge_relax", cnt_relax_edge);
        P->incr("fib_ball_access", cnt_ball_access);
    }

    return d;
}