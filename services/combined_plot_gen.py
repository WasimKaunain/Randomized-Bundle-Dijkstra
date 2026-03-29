import os
import pandas as pd
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

plt.rcParams.update({
    'font.family': 'serif',
    'axes.spines.top': False,
    'axes.spines.right': False,
})

# Colors
COLORS = {
    "er": "#1f77b4",
    "sparse": "#ff7f0e",
    "road": "#2ca02c"
}

base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

er = pd.read_csv(os.path.join(base_dir, "Final_Output", "final_er_results.csv"))
sparse = pd.read_csv(os.path.join(base_dir, "Final_Output", "final_sparse_results.csv"))
road = pd.read_csv(os.path.join(base_dir, "Final_Output", "final_road_results.csv"))

# ER: average across densities
er = er.groupby("#Nodes", as_index=False).mean(numeric_only=True)

# Sort properly
er = er.sort_values("#Nodes")
sparse = sparse.sort_values("#Nodes")
road = road.sort_values("#Nodes")

# OPTIONAL: smooth road (removes jaggedness)
road["smoothed"] = road["Total_Bundle_PQ_ms"].rolling(window=2, min_periods=1).mean()

plt.figure(figsize=(8,5.5))

# ER (bold)
plt.plot(er["#Nodes"], er["Total_Bundle_PQ_ms"],
         lw=2.8, color=COLORS["er"], label="ER Graphs")

# Sparse (medium)
plt.plot(sparse["#Nodes"], sparse["Total_Bundle_PQ_ms"],
         lw=2.4, color=COLORS["sparse"], label="Sparse Random")

# Road (dashed + smoothed)
plt.plot(road["#Nodes"], road["smoothed"],
         lw=2.2, linestyle='--', color=COLORS["road"], label="Road Networks")

plt.xscale('log'); plt.yscale('log')
plt.xlabel("Number of Nodes", fontsize=11)
plt.ylabel("Execution Time (ms)", fontsize=11)
plt.title("Bundle Dijkstra (PQ) — Cross Graph Comparison", fontsize=12)

# Annotation (VERY IMPORTANT)
plt.annotate("Sparse graphs are easiest",
             xy=(sparse["#Nodes"].iloc[-1], sparse["Total_Bundle_PQ_ms"].iloc[-1]),
             xytext=(sparse["#Nodes"].iloc[-2], sparse["Total_Bundle_PQ_ms"].iloc[-1]*2),
             arrowprops=dict(arrowstyle="->", lw=1.2),
             fontsize=9)

plt.annotate("ER graphs are hardest",
             xy=(er["#Nodes"].iloc[-1], er["Total_Bundle_PQ_ms"].iloc[-1]),
             xytext=(er["#Nodes"].iloc[-2], er["Total_Bundle_PQ_ms"].iloc[-1]*1.8),
             arrowprops=dict(arrowstyle="->", lw=1.2),
             fontsize=9)

plt.legend(fontsize=9)

# Clean grid
plt.grid(True, which='major', alpha=0.3)

plot_dir = os.path.join(base_dir, "Plots", "combined")
os.makedirs(plot_dir, exist_ok=True)

plt.tight_layout()
plt.savefig(os.path.join(plot_dir, "combined_pq.png"), dpi=300)
plt.close()

print("Enhanced combined plot generated")