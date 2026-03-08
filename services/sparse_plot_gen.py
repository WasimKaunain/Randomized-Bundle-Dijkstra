import os
import pandas as pd
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter
plt.rcParams.update({
    'font.family': 'serif',
    'axes.spines.top': False,
    'axes.spines.right': False,
})

# Setup
base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
csv_path = os.path.join(base_dir, "Final_Output", "final_sparse_results.csv")
plot_dir = os.path.join(base_dir, "Plots", "sparse")
os.makedirs(plot_dir, exist_ok=True)

# Load and prepare data
df = pd.read_csv(csv_path)
df = df.sort_values("#Nodes")

# Filter out rows where times are 0 (too small to measure)
mask = (df["Ref_ms"] > 0) & (df["Total_Bundle_Set_ms"] > 0)
df = df[mask].reset_index(drop=True)
N = df["#Nodes"]

# X-axis: 2^x labels for sparse graph vertex counts
def pow2_formatter(x, pos):
    exp = int(round(np.log2(x)))
    return r'$2^{%d}$' % exp

# Tick positions: only at the exact 2^x values present in data
tick_positions = sorted(N.unique())

# -------------------------------------------------------
# Plot 1: Total runtime log-log with 2^x X-axis
# -------------------------------------------------------
fig, ax = plt.subplots(figsize=(8, 5))

ax.plot(N, df["Ref_ms"],              'o-',  color='#1565C0', lw=1.6, ms=5, label='Classical Dijkstra')
ax.plot(N, df["Total_Bundle_Set_ms"], 's-',  color='#C62828', lw=1.6, ms=5, label='Bundle Dijkstra (Set)')
ax.plot(N, df["Total_Bundle_Fib_ms"], '^--', color='#2E7D32', lw=1.6, ms=5, label='Bundle Dijkstra (Fib)')

ax.set_xscale('log', base=2)
ax.set_yscale('log')
ax.set_xticks(tick_positions)
ax.xaxis.set_major_formatter(FuncFormatter(pow2_formatter))
ax.tick_params(axis='x', rotation=45, labelsize=9)
ax.set_xlabel("Number of Vertices", fontsize=11)
ax.set_ylabel("Total Execution Time (ms)", fontsize=11)
ax.set_title("Sparse Random Graphs — Runtime Comparison", fontsize=12)
ax.legend(fontsize=9)
ax.grid(True, which='major', ls='-', alpha=0.25)

fig.tight_layout()
fig.savefig(os.path.join(plot_dir, "sparse_runtime_loglog.png"), dpi=300)
plt.close(fig)

# -------------------------------------------------------
# Plot 2: Dijkstra-phase only (Set vs Fib)
# -------------------------------------------------------
fig, ax = plt.subplots(figsize=(8, 5))

ax.plot(N, df["Bundle_Set_ms"], 's-',  color='#C62828', lw=1.6, ms=5, label='Bundle Dijkstra Phase (Set)')
ax.plot(N, df["Bundle_Fib_ms"], '^--', color='#2E7D32', lw=1.6, ms=5, label='Bundle Dijkstra Phase (Fib)')

ax.set_xscale('log', base=2)
ax.set_yscale('log')
ax.set_xticks(tick_positions)
ax.xaxis.set_major_formatter(FuncFormatter(pow2_formatter))
ax.tick_params(axis='x', rotation=45, labelsize=9)
ax.set_xlabel("Number of Vertices", fontsize=11)
ax.set_ylabel("Dijkstra Phase Only (ms)", fontsize=11)
ax.set_title("Sparse Graphs — Set vs Fibonacci (Dijkstra Phase)", fontsize=12)
ax.legend(fontsize=9)
ax.grid(True, which='major', ls='-', alpha=0.25)

fig.tight_layout()
fig.savefig(os.path.join(plot_dir, "sparse_dijkstra_phase.png"), dpi=300)
plt.close(fig)

print("Sparse plots generated: sparse_runtime_loglog.png, sparse_dijkstra_phase.png")