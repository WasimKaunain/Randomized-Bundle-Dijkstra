#!/bin/bash
set -e

########################################
# Create required directories
########################################
mkdir -p graphs/er
mkdir -p graphs/sparse_random

########################################
# 1. Generate ER graphs
########################################
echo "======================================"
echo " Generating ER graphs"
echo "======================================"

for n in 1000 10000 100000 1000000; do
  for m in $((2*n)) $((5*n)) $((10*n)); do
    OUT="graphs/er/er_${n}_${m}.txt"

    if [ ! -f "$OUT" ]; then
      echo "Generating ER graph n=$n m=$m"
      ./bin/gen_er "$n" "$m" "$OUT"
    else
      echo "Already exists: $OUT"
    fi
  done
done

########################################
# 2. Generate Sparse Random graphs
########################################
echo "======================================"
echo " Generating Sparse Random graphs"
echo "======================================"

for exp in $(seq 7 21); do
  n=$((2**exp))
  OUT="graphs/sparse_random/sparse_${n}.txt"

  if [ ! -f "$OUT" ]; then
    echo "Generating sparse random graph n=$n"
    ./bin/gen_sparse \
      --n "$n" \
      --mean-degree 3 \
      --max-degree 4 \
      --root 0 \
      --seed 42 \
      --output "$OUT"
  else
    echo "Already exists: $OUT"
  fi
done

echo "======================================"
echo " All graph generation completed"
echo "======================================"
