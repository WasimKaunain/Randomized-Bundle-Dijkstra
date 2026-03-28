#include "graph.h"
#include "profiler.h"
#include "dijkstra_ref.h"
#include "bundle_construct.h"
#include "bundle_dijkstra.h"

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <fstream>
#include <cstdlib>

void print_usage(){
    std::cout << "Usage: bin/bundle --graph path --source s --seed S\n";
}

int main(int argc, char** argv){
    std::string graph_file;
    int source = 0;
    int seed = 1;

    for(int i = 1; i < argc; i++){
        std::string arg = argv[i];
        if(arg == "--graph" && i+1 < argc)       graph_file = argv[++i];
        else if(arg == "--source" && i+1 < argc)  source = atoi(argv[++i]);
        else if(arg == "--seed" && i+1 < argc)    seed = atoi(argv[++i]);
        else { print_usage(); return 1; }
    }

    if(graph_file.empty()){
        print_usage();
        return 1;
    }

    /* ---------------------------------------------------- */
    /* Determine graph type from filename                   */
    /* ---------------------------------------------------- */

    std::string filename = graph_file;
    auto pos = filename.find_last_of("/\\");
    if(pos != std::string::npos)
        filename = filename.substr(pos + 1);

    bool is_er_graph     = (filename.rfind("er", 0) == 0);
    bool is_sparse_graph = (filename.rfind("sparse", 0) == 0);
    bool is_road_graph   = (!is_er_graph && !is_sparse_graph);

    /* ---------------------------------------------------- */
    /* Load graph                                           */
    /* ---------------------------------------------------- */

    Profiler P;

    P.start("load");
    Graph G = Graph::load_edge_list(graph_file);
    P.stop("load");

    int N = G.adj.size();
    long long mcount = 0;
    for(const auto &v : G.adj)
        mcount += v.size();
    mcount /= 2;

    std::cerr << "Loaded graph: n=" << N << " m~" << mcount << "\n";

    // k = ceil(sqrt(log(N) / log(log(N))))
    int k = std::max(2, (int)std::ceil(
        std::sqrt(std::log((double)N) / std::log(std::log((double)N)))
    ));

    /* ---------------------------------------------------- */
    /* Constant-degree transformation (ER graphs only)      */
    /* ---------------------------------------------------- */

    Graph Gp;
    int Nt = 0;
    long long Mt = 0;

    if(is_er_graph){
        P.start("transform");
        Gp = constant_degree_transform(G);
        P.stop("transform");

        Nt = Gp.adj.size();
        for(const auto &nbrs : Gp.adj)
            Mt += nbrs.size();
        Mt /= 2;

        std::cerr << "Transformed: n'=" << Nt << " m'=" << Mt << "\n";
    } else {
        Gp = G;
        Nt = N;
        Mt = mcount;
        std::cerr << "Transformation skipped\n";
    }

    /* ---------------------------------------------------- */
    /* Bundle Construction                                  */
    /* ---------------------------------------------------- */

    P.start("bundle_construct");
    BundleInfo Binfo = BundleConstruction(Gp, source, k, &P, seed);
    P.stop("bundle_construct");

    std::cerr << "k=" << k
              << " R_size=" << Binfo.R_list.size() << "\n";

    /* ---------------------------------------------------- */
    /* Reference Dijkstra                                   */
    /* ---------------------------------------------------- */

    P.start("dijkstra_ref");
    auto d_ref = DijkstraRef(Gp, source, &P);
    P.stop("dijkstra_ref");

    /* ---------------------------------------------------- */
    /* Bundle Dijkstra (Binary heap)                          */
    /* ---------------------------------------------------- */

    P.start("bundle_dijkstra_pq");
    auto d_bundle_pq = BundleDijkstra_PQ(Gp, source, Binfo, &P);
    P.stop("bundle_dijkstra_pq");


    /* ---------------------------------------------------- */
    /* Bundle Dijkstra (Set-based)                          */
    /* ---------------------------------------------------- */

    P.start("bundle_dijkstra");
    auto d_bundle = BundleDijkstra(Gp, source, Binfo, &P);
    P.stop("bundle_dijkstra");

    /* ---------------------------------------------------- */
    /* Bundle Dijkstra (Fibonacci heap)                     */
    /* ---------------------------------------------------- */

    P.start("bundle_dijkstra_fib");
    auto d_bundle_fib = BundleDijkstra_Fib(Gp, source, Binfo, &P);
    P.stop("bundle_dijkstra_fib");

    /* ---------------------------------------------------- */
    /* Correctness                                          */
    /* ---------------------------------------------------- */

    const double EPS = 1e-8;

    auto check = [&](const std::vector<double>& A, const char* name) -> bool {
        for(int i = 0; i < Nt; i++){
            if(std::abs(d_ref[i] - A[i]) > EPS){
                std::cerr << "Mismatch in " << name
                          << " at node " << i
                          << " ref=" << d_ref[i]
                          << " got=" << A[i] << "\n";
                return false;
            }
        }
        return true;
    };

    bool bundle_pq_ok  = check(d_bundle_pq,  "Bundle_PQ");
    bool bundle_ok     = check(d_bundle,     "Bundle_Set");
    bool bundle_fib_ok = check(d_bundle_fib, "Bundle_Fib");

    std::cerr << "Bundle_PQ correctness:  " << (bundle_pq_ok  ? "OK" : "FAIL") << "\n";
    std::cerr << "Bundle_Set correctness: " << (bundle_ok     ? "OK" : "FAIL") << "\n";
    std::cerr << "Bundle_Fib correctness: " << (bundle_fib_ok ? "OK" : "FAIL") << "\n";

    /* ---------------------------------------------------- */
    /* CSV Output                                           */
    /* ---------------------------------------------------- */

    std::string csv_file;
    if(is_er_graph)          csv_file = "Output/er_results.csv";
    else if(is_sparse_graph) csv_file = "Output/sparse_results.csv";
    else                     csv_file = "Output/road_results.csv";

    {
        std::ifstream fin(csv_file);
        bool empty = (!fin.good() || fin.peek() == std::ifstream::traits_type::eof());
        fin.close();

        std::ofstream fout(csv_file, std::ios::app);

        if(empty){
            fout << "Graph,#Nodes,#Edges,#Nodes_T,#Edges_T,K,Seed,"
                 << "Ref_ms,"
                 << "Bundle_PQ_ms,Bundle_Set_ms,Bundle_Fib_ms,"
                 << "Bundle_construct_ms,"
                 << "R_size,sum_ball_sizes,"
                     
                 << "pq_extract,pq_dk,pq_edge_relax,pq_ball_access,"
                 << "set_extract,set_dk,set_edge_relax,set_ball_access,"
                 << "fib_extract,fib_dk,fib_edge_relax,fib_ball_access,"
                     
                 << "bundle_pq_correct,bundle_correct,bundle_fib_correct\n";
        }

        fout << filename << ","
             << N << "," << mcount << ","
             << Nt << "," << Mt << ","
             << k << "," << seed << ","
            
             << P.get_time_ms("dijkstra_ref") << ","
             << P.get_time_ms("bundle_dijkstra_pq") << ","
             << P.get_time_ms("bundle_dijkstra") << ","
             << P.get_time_ms("bundle_dijkstra_fib") << ","
            
             << P.get_time_ms("bundle_construct") << ","
             << P.get_counter("R_size") << ","
             << P.get_counter("sum_ball_sizes") << ","
            
             // PQ
             << P.get_counter("pq_extract") << ","
             << P.get_counter("pq_dk") << ","
             << P.get_counter("pq_edge_relax") << ","
             << P.get_counter("pq_ball_access") << ","
            
             // SET
             << P.get_counter("set_extract") << ","
             << P.get_counter("set_dk") << ","
             << P.get_counter("set_edge_relax") << ","
             << P.get_counter("set_ball_access") << ","
            
             // FIB
             << P.get_counter("fib_extract") << ","
             << P.get_counter("fib_dk") << ","
             << P.get_counter("fib_edge_relax") << ","
             << P.get_counter("fib_ball_access") << ","
            
             << (bundle_pq_ok ? 1 : 0) << ","
             << (bundle_ok ? 1 : 0) << ","
             << (bundle_fib_ok ? 1 : 0) << "\n";
            
                fout.close();
    }

    return 0;
}