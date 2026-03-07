#pragma once
#include "graph.h"
#include "profiler.h"
#include <vector>

struct Node {
    int vertex;
    double dist;
    bool operator>(const Node &other) const {
        return dist > other.dist;
    }
};
std::vector<double> DijkstraFib(const Graph &G, int s,Profiler *P=nullptr);