#pragma once
#include "graph.h"
#include "bundle_construct.h"
#include "profiler.h"
#include <vector>
using namespace std;

// Set / binary style version
vector<double> BundleDijkstra(const Graph &G,int s,const BundleInfo &Binfo,Profiler *P=nullptr);

// Fibonacci heap version
vector<double> BundleDijkstra_Fib(const Graph &G,int s,const BundleInfo &Binfo,Profiler *P=nullptr);

// binary heap version with priority queue (std::priority_queue)
vector<double> BundleDijkstra_PQ(const Graph &G,int s,const BundleInfo &Binfo,Profiler *P=nullptr);