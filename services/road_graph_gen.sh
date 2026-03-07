#!/bin/bash
set -euo pipefail

# Project root = Dijkstra/
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ROOT_DIR="$SCRIPT_DIR"
ROAD_DIR="$ROOT_DIR/graphs/roads"

mkdir -p "$ROAD_DIR"

# DIMACS-style road graphs (reference-compatible)
URLS=(
     "https://www.diag.uniroma1.it/~challenge9/data/USA-road-t/USA-road-t.NY.gr.gz"
     "https://www.diag.uniroma1.it/~challenge9/data/USA-road-t/USA-road-t.BAY.gr.gz"
     "https://www.diag.uniroma1.it/~challenge9/data/USA-road-t/USA-road-t.COL.gr.gz"
     "https://www.diag.uniroma1.it/~challenge9/data/USA-road-t/USA-road-t.FLA.gr.gz"
     "https://www.diag.uniroma1.it/~challenge9/data/USA-road-t/USA-road-t.NW.gr.gz"
     "https://www.diag.uniroma1.it/~challenge9/data/USA-road-t/USA-road-t.NE.gr.gz"
     "https://www.diag.uniroma1.it/~challenge9/data/USA-road-t/USA-road-t.CAL.gr.gz"
     "https://www.diag.uniroma1.it/~challenge9/data/USA-road-t/USA-road-t.LKS.gr.gz"
     "https://www.diag.uniroma1.it/~challenge9/data/USA-road-t/USA-road-t.E.gr.gz"
     "https://www.diag.uniroma1.it/~challenge9/data/USA-road-t/USA-road-t.W.gr.gz"
     "https://www.diag.uniroma1.it/~challenge9/data/USA-road-t/USA-road-t.CTR.gr.gz"
    
)

echo "Downloading road graphs into: $ROAD_DIR"
echo "----------------------------------------"

for URL in "${URLS[@]}"; do
    BASENAME=$(basename "$URL")
    FINAL_FILE="${BASENAME%.gz}"
    FINAL_PATH="$ROAD_DIR/$FINAL_FILE"
    GZ_PATH="$ROAD_DIR/$BASENAME"

    if [[ -f "$FINAL_PATH" ]]; then
        echo "✔ $FINAL_FILE already exists, skipping."
        continue
    fi

    echo "⬇ Downloading $BASENAME"
    wget -c -O "$GZ_PATH" "$URL"

    echo "📦 Extracting $BASENAME"
    gunzip -f "$GZ_PATH"

    echo "✔ Ready: $FINAL_PATH"
    echo "----------------------------------------"
done

echo "✅ All road graphs are available in $ROAD_DIR"
