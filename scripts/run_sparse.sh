#!/bin/bash
set -e

RUNS=5
RESULT_DIR=results/sparse
mkdir -p $RESULT_DIR/out $RESULT_DIR/err

echo "Running Sparse Random graph experiments"

for graph in $(ls graphs/sparse_random/sparse_*.txt | sort -V); 
do
  base=$(basename "$graph" .txt)
  for run in $(seq 1 $RUNS); 
  do
    echo "Sparse: $base (run $run)"
    ./bin/bundle --graph "$graph" --source 0 --seed $run >> "$RESULT_DIR/out/${base}.out" 2>> "$RESULT_DIR/err/${base}.err"
  done
done

echo "Sparse graph experiments completed"
