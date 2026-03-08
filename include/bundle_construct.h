#pragma once
#include "graph.h"
#include "profiler.h"
#include <vector>

/*
Optimized BundleInfo

- R_list         : list of sampled vertices
- isR[v]         : fast membership check
- b[v]           : bundle representative
- bundles[r]     : vertices bundled to r (only valid if r in R)
- ball[v]        : list of vertices in Ball(v)
- dist_to_ball[v]: distances aligned with ball[v]
*/

struct BundleInfo {

    int N;

    std::vector<int> R_list;              // list of R vertices
    std::vector<char> isR;                // membership flag

    std::vector<int> b;                   // bundle representative
    std::vector<double> dist_to_bv;       // dist_to_bv[v] = dist(v, b(v))

    std::vector<std::vector<int>> bundles;  // only meaningful for R vertices

    // For v not in R:
    std::vector<std::vector<int>> ball;           // ball[v][i] = vertex
    std::vector<std::vector<double>> dist_ball;   // aligned distances
};

BundleInfo BundleConstruction(const Graph &G, int s, int k, Profiler *P=nullptr, int seed=1);