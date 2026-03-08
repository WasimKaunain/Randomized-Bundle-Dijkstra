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
csv_path = os.path.join(base_dir, "Final_Output", "final_road_results.csv")
plot_dir = os.path.join(base_dir, "Plots", "road")
os.makedirs(plot_dir, exist_ok=True)

# Load and prepare data
df = pd.read_csv(csv_path)
df = df.sort_values("#Nodes")

N = df["#Nodes"]

# -------------------------------------------------------
# Plot 1: Total runtime log-log
# -------------------------------------------------------
fig, ax = plt.subplots(figsize=(7, 5))

ax.plot(N, df["Ref_ms"],              'o-',  color='#1565C0', lw=1.6, ms=5, label='Classical Dijkstra')
ax.plot(N, df["Total_Bundle_Set_ms"], 's-',  color='#C62828', lw=1.6, ms=5, label='Bundle Dijkstra (Set)')
ax.plot(N, df["Total_Bundle_Fib_ms"], '^--', color='#2E7D32', lw=1.6, ms=5, label='Bundle Dijkstra (Fib)')

ax.set_xscale('log'); ax.set_yscale('log')
ax.set_xlabel("Number of Vertices", fontsize=11)
ax.set_ylabel("Total Execution Time (ms)", fontsize=11)
ax.set_title("DIMACS Road Networks — Runtime Comparison", fontsize=12)
ax.legend(fontsize=9)
ax.grid(True, which='major', ls='-', alpha=0.25)
ax.grid(True, which='minor', ls=':', alpha=0.15)

fig.tight_layout()
fig.savefig(os.path.join(plot_dir, "road_runtime_loglog.png"), dpi=300)
plt.close(fig)

# -------------------------------------------------------
# Plot 2: Dijkstra-phase only (Set vs Fib)
# -------------------------------------------------------
fig, ax = plt.subplots(figsize=(7, 5))

ax.plot(N, df["Bundle_Set_ms"], 's-',  color='#C62828', lw=1.6, ms=5, label='Bundle Dijkstra Phase (Set)')
ax.plot(N, df["Bundle_Fib_ms"], '^--', color='#2E7D32', lw=1.6, ms=5, label='Bundle Dijkstra Phase (Fib)')

ax.set_xscale('log'); ax.set_yscale('log')
ax.set_xlabel("Number of Vertices", fontsize=11)
ax.set_ylabel("Dijkstra Phase Only (ms)", fontsize=11)
ax.set_title("Road Networks — Set vs Fibonacci (Dijkstra Phase)", fontsize=12)
ax.legend(fontsize=9)
ax.grid(True, which='major', ls='-', alpha=0.25)
ax.grid(True, which='minor', ls=':', alpha=0.15)

fig.tight_layout()
fig.savefig(os.path.join(plot_dir, "road_dijkstra_phase.png"), dpi=300)
plt.close(fig)

print("Road plots generated: road_runtime_loglog.png, road_dijkstra_phase.png")