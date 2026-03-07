#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    int n = -1;
    int mean_deg = 3;
    int max_deg = 4;
    int root = 0;
    string output;
    unsigned seed = 42;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--n" && i + 1 < argc) {
            n = stoi(argv[++i]);
        } else if (arg == "--mean-degree" && i + 1 < argc) {
            mean_deg = stoi(argv[++i]);
        } else if (arg == "--max-degree" && i + 1 < argc) {
            max_deg = stoi(argv[++i]);
        } else if (arg == "--root" && i + 1 < argc) {
            root = stoi(argv[++i]);
        } else if (arg == "--seed" && i + 1 < argc) {
            seed = stoi(argv[++i]);
        } else if (arg == "--output" && i + 1 < argc) {
            output = argv[++i];
        } else {
            cerr << "Unknown or incomplete option: " << arg << "\n";
            return 1;
        }
    }

    if (n <= 0 || output.empty()) {
        cerr << "Usage: gen_sparse --n N --mean-degree D --max-degree M "
             << "--root R --seed S --output FILE\n";
        return 1;
    }

    mt19937 rng(seed);
    uniform_int_distribution<int> weight_dist(1, 100);

    vector<vector<pair<int,int>>> adj(n);
    vector<int> outdeg(n, 0);

    /* --------------------------------------------------
       1. Spanning tree rooted at root
       -------------------------------------------------- */
    for (int v = 0; v < n; v++) {
        if (v == root) continue;
        int u = rng() % v;
        int w = weight_dist(rng);

        adj[u].push_back({v, w});
        outdeg[u]++;
    }

    /* --------------------------------------------------
       2. Add random edges
       -------------------------------------------------- */
    int target_edges = n * mean_deg;
    uniform_int_distribution<int> node_dist(0, n - 1);

    while (true) {
        int edges = 0;
        for (int d : outdeg) edges += d;
        if (edges >= target_edges) break;

        int u = node_dist(rng);
        int v = node_dist(rng);
        if (u == v) continue;
        if (outdeg[u] >= max_deg) continue;

        bool exists = false;
        for (auto &p : adj[u]) {
            if (p.first == v) {
                exists = true;
                break;
            }
        }
        if (exists) continue;

        int w = weight_dist(rng);
        adj[u].push_back({v, w});
        outdeg[u]++;
    }

    /* --------------------------------------------------
       3. Write graph
       -------------------------------------------------- */
    ofstream out(output);
    int m = 0;
    for (auto &v : adj) m += v.size();

    out << n << " " << m << "\n";
    for (int u = 0; u < n; u++) {
        for (auto &[v, w] : adj[u]) {
            out << u << " " << v << " " << w << "\n";
        }
    }

    cerr << "Generated sparse graph n=" << n
         << " mean_deg=" << mean_deg
         << " max_deg=" << max_deg << "\n";

    return 0;
}
