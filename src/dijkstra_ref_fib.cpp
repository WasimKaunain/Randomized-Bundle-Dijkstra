#include "dijkstra_ref.h"
#include <vector>
#include <limits>
#include <boost/heap/fibonacci_heap.hpp>

struct Node {
    int vertex;
    double dist;

    bool operator>(const Node &other) const {
        return dist > other.dist;
    }
};

std::vector<double> DijkstraRefFib(const Graph &G, int s, Profiler *P) {
    int n = (int)G.adj.size();
    const double INF = std::numeric_limits<double>::infinity();

    std::vector<double> dist(n, INF);
    dist[s] = 0.0;

    // Fibonacci heap
    using FibHeap = boost::heap::fibonacci_heap<Node,boost::heap::compare<std::greater<Node> > >;

    FibHeap heap;

    // Store handles for decrease-key
    std::vector<FibHeap::handle_type> handles(n);

    // Insert all nodes initially
    for (int i = 0; i < n; i++) {
        handles[i] = heap.push({i, dist[i]});
    }

    while (!heap.empty()) {
        Node cur = heap.top();
        heap.pop();

        int u = cur.vertex;
        double d = cur.dist;

        if (d > dist[u]) continue;

        for (auto &e : G.adj[u]) {
            int v = e.to;
            double nd = d + e.weight;

            if (nd < dist[v]) {
                dist[v] = nd;

                // decrease-key
                heap.update(handles[v], {v, nd});
            }
        }
    }

    return dist;
}