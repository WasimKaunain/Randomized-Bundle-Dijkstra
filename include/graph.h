#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <utility>
using namespace std;

struct edge 
    { 
        int to;
        double weight; 
    };

struct Graph    
    { 
        int n; //original vertex count (before constant-degree transform)
        vector<vector<edge>> adj; // adjacency list

        Graph() : n(0) { };
        Graph(int n_) : n(n_), adj(n_) { };
        void add_edge(int u, int v, double w);
        static Graph load_edge_list(const string &fname); //simple edge list u v w...
    };

Graph constant_degree_transform(const Graph &);

void check_connectivity(const Graph &G, int source);