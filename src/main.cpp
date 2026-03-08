#include "graph.h"
#include "profiler.h"
#include "dijkstra_ref.h"
#include "bundle_construct.h"
#include "bundle_dijkstra.h"
#include "dijkstra_fibheap.h"

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <sstream>
#include <cstdlib>

void print_usage(){
    std::cout << "Usage: bin/bundle --graph path --source s --seed S\n";
}

int main(int argc, char** argv){
    std::string graph_file;
    int source = 0;
    int seed = 1;

    for(int i=1;i<argc;i++){
        std::string arg = argv[i];
        if(arg == "--graph" && i+1<argc) graph_file = argv[++i];
        else if(arg == "--source" && i+1<argc) source = atoi(argv[++i]);
        else if(arg == "--seed" && i+1<argc) seed = atoi(argv[++i]);
        else { print_usage(); return 1; }
    }

    if(graph_file.empty()){
        print_usage(); 
        return 1;
    }

    /* ---------------------------------------------------- */
    /* Determine graph type                                 */
    /* ---------------------------------------------------- */

    std::string filename = graph_file;
    auto pos = filename.find_last_of("/\\");
    if (pos != std::string::npos)
        filename = filename.substr(pos + 1);

    bool is_er_graph = false;
    bool is_sparse_graph = false;
    bool is_road_graph = false;

    if (filename.rfind("er", 0) == 0)
        is_er_graph = true;
    else if (filename.rfind("sparse", 0) == 0)
        is_sparse_graph = true;
    else
        is_road_graph = true;

    /* ---------------------------------------------------- */
    /* Load graph                                           */
    /* ---------------------------------------------------- */

    Profiler P;
    P.start("total");

    P.start("load");
    Graph G = Graph::load_edge_list(graph_file);
    P.stop("load");

    int N = G.adj.size();
    long long mcount = 0;
    for(const auto &v : G.adj)
        mcount += v.size();
    mcount /= 2;

    std::cerr << "Loaded graph: n=" << N << " m~" << mcount << "\n";

    int k = (int)std::ceil(
        std::sqrt(std::log((double)N) / std::log(std::log((double)N)))
    );

    /* ---------------------------------------------------- */
    /* Transformation (ER only)                             */
    /* ---------------------------------------------------- */

    Graph Gp;
    int Nt = 0;
    long long Mt = 0;

    if (is_er_graph) {
        P.start("transform");
        Gp = constant_degree_transform(G);
        P.stop("transform");

        Nt = Gp.adj.size();
        for (const auto &nbrs : Gp.adj)
            Mt += nbrs.size();
        Mt /= 2;

        std::cerr << "Transformed graph size: n'=" << Nt
                  << " m'=" << Mt << "\n";
    } else {
        Gp = G;
        Nt = N;
        Mt = mcount;
        std::cerr << "Transformation skipped\n";
    }
    
    // ADD THIS - Check if graph is connected
    std::cerr << "\n";
    check_connectivity(Gp, source);
    std::cerr << "\n";

        /* ---------------------------------------------------- */
    /* Bundle Construction                                  */
    /* ---------------------------------------------------- */

    P.start("bundle_construct");
    BundleInfo Binfo = BundleConstruction(Gp, source, k, &P, seed);
    P.stop("bundle_construct");

    // ADD DIAGNOSTIC CHECKS
    std::cerr << "\n=== Bundle Diagnostics ===\n";
    std::cerr << "Source vertex: " << source << "\n";
    std::cerr << "Source in R: " << (Binfo.isR[source] ? "YES" : "NO") << "\n";
    std::cerr << "b[source] = " << Binfo.b[source] << "\n";
    std::cerr << "R_list size: " << Binfo.R_list.size() << "\n";
    
    // Check first few vertices
    std::cerr << "\nFirst 5 non-R vertices:\n";
    int count = 0;
    for(int v=0; v<std::min(Nt, 10); ++v){
        if(!Binfo.isR[v]){
            std::cerr << "  v=" << v << " b(v)=" << Binfo.b[v] 
                      << " ball_size=" << Binfo.ball[v].size() << "\n";
            count++;
            if(count >= 5) break;
        }
    }
    std::cerr << "===========================\n\n";

    /* ---------------------------------------------------- */
    /* Bundle Dijkstra (Set version)                        */
    /* ---------------------------------------------------- */

    P.start("bundle_dijkstra");
    auto d_bundle = BundleDijkstra(Gp, source, Binfo, &P);
    P.stop("bundle_dijkstra");

    /* ---------------------------------------------------- */
    /* Bundle Dijkstra (Fibonacci version)                  */
    /* ---------------------------------------------------- */

    P.start("bundle_dijkstra_fib");
    auto d_bundle_fib = BundleDijkstra_Fib(Gp, source, Binfo, &P);
    P.stop("bundle_dijkstra_fib");

    /* ---------------------------------------------------- */
    /* Reference Dijkstra                                   */
    /* ---------------------------------------------------- */

    P.start("dijkstra_ref");
    auto d_ref = DijkstraRef(Gp, source, &P);
    P.stop("dijkstra_ref");

    P.start("dijkstra_fib");
    auto d_fib = DijkstraFib(Gp, source, &P);
    P.stop("dijkstra_fib");

    /* ---------------------------------------------------- */
    /* Correctness Checks                                   */
    /* ---------------------------------------------------- */

    const double EPS = 1e-8;

auto debug_compare = [&](const std::vector<double>& A, const std::string& name){
    for(size_t i=0;i<d_ref.size();++i){
        if(std::abs(d_ref[i] - A[i]) > EPS){
            std::cerr << "Mismatch in " << name 
                      << " at node " << i
                      << " ref=" << d_ref[i]
                      << " val=" << A[i]
                      << "\n";
            return false;
        }
    }
    return true;
};

bool bundle_ok = debug_compare(d_bundle, "Bundle");
bool bundle_fib_ok = debug_compare(d_bundle_fib, "Bundle+Fib");
bool fib_ok = debug_compare(d_fib, "Fib");

    std::cerr << "Bundle correctness: " << (bundle_ok ? "OK" : "FAIL") << "\n";
    std::cerr << "Bundle+Fib correctness: " << (bundle_fib_ok ? "OK" : "FAIL") << "\n";
    std::cerr << "Fib correctness: " << (fib_ok ? "OK" : "FAIL") << "\n";

    /* ---------------------------------------------------- */
    /* CSV Output                                           */
    /* ---------------------------------------------------- */

    std::string csv_file;

    if (is_er_graph)
        csv_file = "Output/er_results.csv";
    else if (is_sparse_graph)
        csv_file = "Output/sparse_results.csv";
    else
        csv_file = "Output/road_results.csv";

    std::ofstream fout(csv_file, std::ios::app);
    std::ifstream fin(csv_file);
    bool empty = (fin.peek() == std::ifstream::traits_type::eof());
    fin.close();

    if (empty) {
        fout << "Graph,#Nodes,#Edges,#Nodes_T,#Edges_T,K,Seed,"
             << "Ref_ms,Fib_ms,"
             << "Bundle_Set_ms,Bundle_Fib_ms,"
             << "Bundle_construct_ms,"
             << "R_size,sum_ball_sizes,"
             << "extracts,decrease_key,"
             << "bundle_correct,bundle_fib_correct,fib_correct\n";
    }

    fout << filename << "," << N << "," << mcount << ","
         << Nt << "," << Mt << "," << k << "," << seed << ","
         << P.get_time_ms("dijkstra_ref") << ","
         << P.get_time_ms("dijkstra_fib") << ","
         << P.get_time_ms("bundle_dijkstra") << ","
         << P.get_time_ms("bundle_dijkstra_fib") << ","
         << P.get_time_ms("bundle_construct") << ","
         << P.get_counter("R_size") << ","
         << P.get_counter("sum_ball_sizes") << ","
         << P.get_counter("extracts") << ","
         << P.get_counter("decrease_key") << ","
         << (bundle_ok ? 1 : 0) << ","
         << (bundle_fib_ok ? 1 : 0) << ","
         << (fib_ok ? 1 : 0) << "\n";

    fout.close();

    return 0;
}