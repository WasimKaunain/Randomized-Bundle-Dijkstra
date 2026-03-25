# Randomized Bundle Dijkstra

This repository implements and benchmarks a **Randomized Bundle Dijkstra** shortest-path approach against a reference Dijkstra implementation.

## What this project contains

- `bin/bundle`: main experiment runner.
- `bin/gen_er`: Erdos-Renyi graph generator.
- `bin/gen_sparse`: sparse random graph generator.
- scripts for batch experiments (`scripts/`) and SLURM submission (`slurm/`).
- post-processing and plotting utilities (`services/`).

## Core pipeline (`bin/bundle`)

1. Load an input graph from edge-list or DIMACS-like `.gr` format.
2. Infer graph category from filename prefix:
   - `er*` → ER graph.
   - `sparse*` → sparse random graph.
   - otherwise treated as road graph.
3. For ER graphs only, apply a constant-degree transformation.
4. Build randomized bundles (`R`, `b(v)`, `Ball(v)`, `Bundle(r)`).
5. Run three SSSP variants:
   - reference Dijkstra (`DijkstraRef`)
   - bundle Dijkstra with `std::set`
   - bundle Dijkstra with Boost Fibonacci heap
6. Validate bundle variants vs reference distances.
7. Append timing/correctness rows to `Output/*.csv`.

## Algorithms and data structures

### Graph model

- `Graph` stores an undirected weighted adjacency list.
- `add_edge(u,v,w)` inserts both directions.
- loader supports:
  - plain `u v w`
  - DIMACS arc lines `a u v w` (1-based), deduplicated by keeping only `u < v` before adding undirected edges.

### Bundle construction

`BundleConstruction` samples landmarks `R` with probability `1/k` (source always included), then for each non-landmark vertex runs truncated Dijkstra until first landmark is reached to compute:

- `b(v)` nearest landmark,
- `dist_to_bv[v] = dist(v, b(v))`,
- `Ball(v)` and precomputed `dist_ball[v]`.

It also builds reverse membership lists `bundles[r] = {v : b(v)=r}`.

### Bundle Dijkstra variants

Both bundle variants execute the same high-level relaxations:

- priority queue tracks **only landmark nodes**,
- updates non-landmark distances and propagates improvements to their bundle leader.

Difference is PQ implementation:

- `BundleDijkstra`: `std::set`-based min-queue.
- `BundleDijkstra_Fib`: Boost `fibonacci_heap` decrease-key handles.

## Build and run

```bash
make
./bin/bundle --graph graphs/er/er_n1000_m5000.txt --source 0 --seed 1
```

Batch runs:

```bash
scripts/run_er.sh
scripts/run_sparse.sh
scripts/run_road.sh
```

Aggregation:

```bash
python3 services/aggregate_results.py
```

## Notes and caveats

- `src/gen_sparse.cpp` writes a leading `n m` header and directed edges, but runtime graph loading (`Graph::load_edge_list`) reads only parseable `u v w` lines and always inserts undirected edges. In practice this yields an undirected interpretation of generated edges.
- Output CSVs are append-only; duplicate header rows can appear after repeated runs. Aggregation code removes duplicated header rows before averaging.
