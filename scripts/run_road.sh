#!/bin/bash
set -ex

RUNS=5
RESULT_DIR=results/road
GRAPH_DIR=graphs/roads

mkdir -p $RESULT_DIR/out $RESULT_DIR/err

echo "Running Road Network experiments"

graphs=(
  "USA-road-t.NY.gr"
  "USA-road-t.BAY.gr"
  "USA-road-t.COL.gr"
  "USA-road-t.FLA.gr"
  "USA-road-t.NW.gr"
  "USA-road-t.NE.gr"
  "USA-road-t.CAL.gr"
  "USA-road-t.LKS.gr"
  "USA-road-t.E.gr"
  "USA-road-t.W.gr"
  "USA-road-t.CTR.gr"
)

for base in "${graphs[@]}"; do
  graph="$GRAPH_DIR/$base"

  if [[ ! -f "$graph" ]]; then
    echo "Error: missing graph file $graph"
    exit 1
  fi

  for run in $(seq 1 $RUNS); 
  do
    echo "Road: $base (run $run)"
    ./bin/bundle --graph "$graph" --source 0 --seed $run >> "$RESULT_DIR/out/${base}.out" 2>> "$RESULT_DIR/err/${base}.err"
  done
done

echo "Road graph experiments completed"
