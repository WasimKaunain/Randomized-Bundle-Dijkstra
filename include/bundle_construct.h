#pragma once
#include "graph.h"
#include "profiler.h"
#include <vector>
using namespace std;
/*
 * BundleInfo — data structures for the Randomized Bundle Dijkstra algorithm.
 *
 * As Per the paper (Section 3):
 *   - R              : randomly sampled landmark vertices (source always in R)
 *   - b(v)           : nearest R-vertex to v (b(v) = v if v ∈ R)
 *   - dist_to_bv[v]  : dist(v, b(v)) — distance from v to its bundle leader
 *   - Ball(v)        : set of non-R vertices closer to v than b(v)
 *                       i.e., {x ∉ R : dist(v,x) < dist(v, b(v))}
 *                       Ball(v) does NOT include b(v) itself.
 *   - dist_ball[v][i] : dist(v, ball[v][i]) — precomputed ball distances
 *   - Bundle(r)      : {v : b(v) = r} for each R-vertex r
 */

struct BundleInfo {

    int N;

    vector<int> R_list;                 // list of R vertices
    vector<char> isR;                   // isR[v] = 1 iff v
    vector<int> b;                      // b[v] = bundle representative (nearest R-vertex)
    vector<double> dist_to_bv;          // dist_to_bv[v] = dist(v, b(v))
    vector<vector<int>> bundles;        // bundles[r] = {v : b(v) = r}, only for R-vert
    vector<vector<int>> ball;           // ball[v][i] = vertex in Ball(v)
    vector<vector<double>> dist_ball;   // dist_ball[v][i] = dist(v, ball[v][i])
};

BundleInfo BundleConstruction(const Graph &G, int s, int k, Profiler *P=nullptr, int seed=1);