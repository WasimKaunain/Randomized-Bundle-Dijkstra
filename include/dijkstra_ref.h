#pragma once
#include "graph.h"
#include "profiler.h"
#include <vector>

std::vector<double> DijkstraRef(const Graph &G, int s, Profiler *P=nullptr);
