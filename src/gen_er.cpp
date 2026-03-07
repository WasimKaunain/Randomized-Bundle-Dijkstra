#include <bits/stdc++.h>
using namespace std;

int main(int argc, char** argv){
    if(argc < 4){
        cerr << "Usage: ./gen_er n m output.txt\n";
        return 1;
    }
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    string fname = argv[3];

    ofstream out(fname);
    mt19937 rng(42); // fixed seed
    uniform_int_distribution<int> U(0, n-1);
    uniform_real_distribution<double> W(1.0, 10.0);

    set<pair<int,int>> edges;
    while((int)edges.size() < m){
        int u = U(rng), v = U(rng);
        if(u==v) continue;
        if(u > v) swap(u,v);
        if(edges.insert({u,v}).second){
            out << u << " " << v << " " << W(rng) << "\n";
        }
    }
    out.close();
}
