import os
import pandas as pd
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
plt.rcParams.update({
    'font.family': 'serif',
    'axes.spines.top': False,
    'axes.spines.right': False,
})

# Setup
base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
er_csv = os.path.join(base_dir, "Final_Output", "final_er_results.csv")
sparse_csv = os.path.join(base_dir, "Final_Output", "final_sparse_results.csv")
road_csv = os.path.join(base_dir, "Final_Output", "final_road_results.csv")
plot_dir = os.path.join(base_dir, "Plots", "combined_bundle")
os.makedirs(plot_dir, exist_ok=True)

# Load data
er = pd.read_csv(er_csv).sort_values("#Nodes")
sparse = pd.read_csv(sparse_csv).sort_values("#Nodes")
road = pd.read_csv(road_csv).sort_values("#Nodes")

# For ER: average across densities per N
er = er.groupby("#Nodes", as_index=False).mean(numeric_only=True)

# Filter out zero-time rows
er = er[er["Total_Bundle_Set_ms"] > 0]
sparse = sparse[sparse["Total_Bundle_Set_ms"] > 0]

# --- Combined: Bundle (Set) only, across all families ---
fig, ax = plt.subplots(figsize=(8, 5.5))

ax.plot(er["#Nodes"],     er["Total_Bundle_Set_ms"],
        'o-',  color='#C62828', lw=1.6, ms=5, label='ER Graphs')
ax.plot(sparse["#Nodes"], sparse["Total_Bundle_Set_ms"],
        's-',  color='#1565C0', lw=1.6, ms=5, label='Sparse Random Graphs')
ax.plot(road["#Nodes"],   road["Total_Bundle_Set_ms"],
        '^-',  color='#2E7D32', lw=1.6, ms=5, label='DIMACS Road Networks')

ax.set_xscale('log'); ax.set_yscale('log')
ax.set_xlabel("Number of Vertices", fontsize=11)
ax.set_ylabel("Total Bundle Dijkstra (Set) Time (ms)", fontsize=11)
ax.set_title("Randomized Bundle Dijkstra — Cross-Family Comparison", fontsize=12)
ax.legend(fontsize=9)
ax.grid(True, which='major', ls='-', alpha=0.25)
ax.grid(True, which='minor', ls=':', alpha=0.15)

fig.tight_layout()
fig.savefig(os.path.join(plot_dir, "bundle_runtime_comparison.png"), dpi=300)
plt.close(fig)

print("Combined plot generated: bundle_runtime_comparison.png")