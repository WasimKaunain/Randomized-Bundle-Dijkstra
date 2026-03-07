#include <boost/heap/fibonacci_heap.hpp>
#include "bundle_dijkstra.h"
#include "dijkstra_fibheap.h"
#include <limits>

std::vector<double> BundleDijkstra_Fib(const Graph &G,int s,const BundleInfo &B,Profiler *P)
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

    // Handle storage for decrease-key
    std::vector<FibHeap::handle_type> handles(n);

    // Insert only R vertices into heap
    for (int r : B.R_list) {
        double key = (r == s) ? 0.0 : INF;
        dist[r] = key;
        handles[r] = heap.push({r, key});
    }

    // If source not in R, still ensure correct initialization
    if (!B.isR[s]) {
        dist[s] = 0;
    }

    while (!heap.empty()) {

        Node uNode = heap.top();
        heap.pop();

        int u = uNode.vertex;

        if (P) P->incr("extracts");

        // =========================
        // STEP 1: Process Bundle(u)
        // =========================
        for (int v : B.bundles[u]) {

            // Find dist(u,v)
            double duv = INF;
            for (size_t i = 0; i < B.ball[v].size(); ++i) {
                if (B.ball[v][i] == u) {
                    duv = B.dist_ball[v][i];
                    break;
                }
            }

            if (duv < INF) {
                double nd = dist[u] + duv;
                if (nd < dist[v]) {
                    dist[v] = nd;
                    if (B.isR[v]) {
                        heap.update(handles[v], {v, nd});
                        if (P) P->incr("decrease_key");
                    }
                }
            }

            // Relax via Ball(v)
            for (size_t i = 0; i < B.ball[v].size(); ++i) {
                int y = B.ball[v][i];
                double dyv = B.dist_ball[v][i];

                if (dist[y] < INF) {
                    double nd = dist[y] + dyv;
                    if (nd < dist[v]) {
                        dist[v] = nd;
                        if (B.isR[v]) {
                            heap.update(handles[v], {v, nd});
                            if (P) P->incr("decrease_key");
                        }
                    }
                }
            }

            // Relax via neighbors of Ball(v)
            for (size_t i = 0; i < B.ball[v].size(); ++i) {
                int z2 = B.ball[v][i];
                double distz2v = B.dist_ball[v][i];

                for (auto &e : G.adj[z2]) {
                    int z1 = e.to;
                    double w = e.weight;

                    if (dist[z1] < INF) {
                        double nd = dist[z1] + w + distz2v;
                        if (nd < dist[v]) {
                            dist[v] = nd;
                            if (B.isR[v]) {
                                heap.update(handles[v], {v, nd});
                                if (P) P->incr("decrease_key");
                            }
                        }
                    }
                }
            }

            // Relax b(v)
            int bv = B.b[v];
            if (bv != v) {
                double dvbv = INF;
                for (size_t i = 0; i < B.ball[v].size(); ++i) {
                    if (B.ball[v][i] == bv) {
                        dvbv = B.dist_ball[v][i];
                        break;
                    }
                }

                if (dvbv < INF) {
                    double nd = dist[v] + dvbv;
                    if (nd < dist[bv]) {
                        dist[bv] = nd;
                        if (B.isR[bv]) {
                            heap.update(handles[bv], {bv, nd});
                            if (P) P->incr("decrease_key");
                        }
                    }
                }
            }
        }

        // =========================
        // STEP 2: Process neighbors
        // =========================
        for (int x : B.bundles[u]) {
            for (auto &e : G.adj[x]) {
                int y = e.to;
                double w = e.weight;

                double nd = dist[x] + w;
                if (nd < dist[y]) {
                    dist[y] = nd;
                    if (B.isR[y]) {
                        heap.update(handles[y], {y, nd});
                        if (P) P->incr("decrease_key");
                    }
                }

                // Relax Ball(y)
                for (size_t i = 0; i < B.ball[y].size(); ++i) {
                    int z1 = B.ball[y][i];
                    double distyz1 = B.dist_ball[y][i];

                    double nd2 = dist[x] + w + distyz1;
                    if (nd2 < dist[z1]) {
                        dist[z1] = nd2;
                        if (B.isR[z1]) {
                            heap.update(handles[z1], {z1, nd2});
                            if (P) P->incr("decrease_key");
                        }
                    }
                }
            }
        }
    }

    return dist;
}