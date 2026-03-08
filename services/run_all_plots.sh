#!/bin/bash
# Run all plot generation scripts
# Usage: bash services/run_all_plots.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "========================================="
echo "  Generating all plots..."
echo "========================================="

echo ""
echo "[1/4] ER plots..."
python3 "$SCRIPT_DIR/er_plot_gen.py"

echo "[2/4] Sparse plots..."
python3 "$SCRIPT_DIR/sparse_plot_gen.py"

echo "[3/4] Road plots..."
python3 "$SCRIPT_DIR/road_plot_gen.py"

echo "[4/4] Combined plots..."
python3 "$SCRIPT_DIR/combined_plot_gen.py"

echo ""
echo "========================================="
echo "  All plots generated successfully!"
echo "========================================="
