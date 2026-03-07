#include <boost/heap/fibonacci_heap.hpp>
#include "dijkstra_fibheap.h"
#include <limits>

std::vector<double> DijkstraFib(const Graph &G, int s,Profiler *P) {
    int n = G.adj.size();
    const double INF = std::numeric_limits<double>::infinity();
    std::vector<double> dist(n, INF);

    using FibHeap = boost::heap::fibonacci_heap<Node, boost::heap::compare<std::greater<Node>>>;
    FibHeap heap;
    std::vector<FibHeap::handle_type> handles(n);

    for (int v = 0; v < n; v++) {
        handles[v] = heap.push({v, INF});
    }

    // decrease-key for source
    dist[s] = 0;
    heap.update(handles[s], {s, 0});

    while (!heap.empty()) {
        Node u = heap.top();
        heap.pop();

        for (auto &e : G.adj[u.vertex]) {
            int v = e.to;
            double nd = dist[u.vertex] + e.weight;
            if (nd < dist[v]) {
                dist[v] = nd;
                heap.update(handles[v], {v, nd});
            }
        }
    }
    return dist;
}
