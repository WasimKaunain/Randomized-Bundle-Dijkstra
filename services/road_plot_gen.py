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

# Colors (clean & consistent)
COLORS = {
    "dijkstra": "#1f77b4",
    "pq": "#ff7f0e",
    "fib": "#2ca02c",
    "set": "#d62728",
    "heap": "#6a3d9a",
    "graph": "#17becf"
}

base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
csv_path = os.path.join(base_dir, "Final_Output", "final_road_results.csv")
plot_dir = os.path.join(base_dir, "Plots", "road")
os.makedirs(plot_dir, exist_ok=True)

df = pd.read_csv(csv_path).sort_values("#Nodes")

# Average across densities
df = df.groupby("#Nodes", as_index=False).mean(numeric_only=True)
df = df[df["Ref_ms"] > 0]

N = df["#Nodes"]

# -------------------------------------------------------
# Plot 1: Runtime (ONLY Dijkstra vs PQ)
# -------------------------------------------------------
plt.figure(figsize=(7,5))

plt.plot(N, df["Ref_ms"], 'o-', lw=2.5, ms=6,
         color=COLORS["dijkstra"], label="Dijkstra (Binary Heap)")

plt.plot(N, df["Total_Bundle_PQ_ms"], 's-', lw=2.5, ms=6,
         color=COLORS["pq"], label="Bundle Dijkstra (PQ)")

# Highlight gap
plt.fill_between(N, df["Ref_ms"], df["Total_Bundle_PQ_ms"],
                 color=COLORS["pq"], alpha=0.08)

plt.xscale('log'); plt.yscale('log')
plt.xlabel("Number of Nodes", fontsize=11)
plt.ylabel("Execution Time (ms)", fontsize=11)
plt.title("Road Graphs — Runtime Comparison", fontsize=12)

# Annotation (VERY IMPORTANT)
plt.annotate("Bundle overhead dominates",
             xy=(N.iloc[-1], df["Total_Bundle_PQ_ms"].iloc[-1]),
             xytext=(N.iloc[-2], df["Total_Bundle_PQ_ms"].iloc[-1]*2),
             arrowprops=dict(arrowstyle="->", lw=1.2),
             fontsize=9)

plt.legend(fontsize=9)
plt.grid(True, which='major', alpha=0.3)

plt.tight_layout()
plt.savefig(os.path.join(plot_dir, "runtime.png"), dpi=300)
plt.close()

# -------------------------------------------------------
# Plot 2: Bundle Dijkstra Phase (PQ vs Set vs Fib)
# -------------------------------------------------------
plt.figure(figsize=(7,5))

plt.plot(N, df["Bundle_PQ_ms"], 'o-', lw=2.2, ms=5,
         color=COLORS["pq"], label="PQ")

plt.plot(N, df["Bundle_Fib_ms"], '^-', lw=2.2, ms=5,
         color=COLORS["fib"], label="Fibonacci Heap")

plt.plot(N, df["Bundle_Set_ms"], 's-', lw=2.2, ms=5,
         color=COLORS["set"], label="Set")

plt.xscale('log'); plt.yscale('log')
plt.xlabel("Number of Nodes", fontsize=11)
plt.ylabel("Dijkstra Phase Time (ms)", fontsize=11)
plt.title("Road Graphs — Bundle Phase Comparison", fontsize=12)

# Annotation
plt.annotate("Binary heap performs best",
             xy=(N.iloc[-1], df["Bundle_PQ_ms"].iloc[-1]),
             xytext=(N.iloc[-2], df["Bundle_PQ_ms"].iloc[-1]*1.8),
             arrowprops=dict(arrowstyle="->", lw=1.2),
             fontsize=9)

plt.legend(fontsize=9)
plt.grid(True, which='major', alpha=0.3)

plt.tight_layout()
plt.savefig(os.path.join(plot_dir, "phase.png"), dpi=300)
plt.close()

# -------------------------------------------------------
# Plot 3: Speedup
# -------------------------------------------------------
df["Speedup_PQ"] = df["Ref_ms"] / df["Total_Bundle_PQ_ms"]
df["Speedup_Set"] = df["Ref_ms"] / df["Total_Bundle_Set_ms"]
df["Speedup_Fib"] = df["Ref_ms"] / df["Total_Bundle_Fib_ms"]

plt.figure(figsize=(7,5))

plt.plot(N, df["Speedup_PQ"], lw=2.2, label="PQ", color=COLORS["pq"])
plt.plot(N, df["Speedup_Fib"], lw=2.2, label="Fib", color=COLORS["fib"])
plt.plot(N, df["Speedup_Set"], lw=2.2, label="Set", color=COLORS["set"])

plt.axhline(1, linestyle='--', color='black', lw=1)

plt.xscale('log')
plt.xlabel("Number of Nodes", fontsize=11)
plt.ylabel("Speedup (Dijkstra / Bundle)", fontsize=11)
plt.title("Road Graphs — Speedup Analysis", fontsize=12)

# Annotation
plt.annotate("Always < 1 → Bundle slower",
             xy=(N.iloc[-1], df["Speedup_PQ"].iloc[-1]),
             xytext=(N.iloc[-2], 0.5),
             arrowprops=dict(arrowstyle="->", lw=1.2),
             fontsize=9)

plt.legend(fontsize=9)
plt.grid(True, which='major', alpha=0.3)

plt.tight_layout()
plt.savefig(os.path.join(plot_dir, "speedup.png"), dpi=300)
plt.close()

# -------------------------------------------------------
# Plot 4: Heap vs Graph Operations
# -------------------------------------------------------
df["Heap"] = df["pq_extract"] + df["pq_dk"]
df["Graph"] = df["pq_edge_relax"] + df["pq_ball_access"]

plt.figure(figsize=(7,5))

plt.plot(N, df["Heap"], lw=2.2, label="Heap Operations", color=COLORS["heap"])
plt.plot(N, df["Graph"], lw=2.2, label="Graph Operations", color=COLORS["graph"])

plt.xscale('log'); plt.yscale('log')
plt.xlabel("Number of Nodes", fontsize=11)
plt.ylabel("Operation Count", fontsize=11)
plt.title("Road Graphs — Operation Cost Breakdown", fontsize=12)

# Annotation
plt.annotate("Graph ops dominate",
             xy=(N.iloc[-1], df["Graph"].iloc[-1]),
             xytext=(N.iloc[-2], df["Graph"].iloc[-1]*0.3),
             arrowprops=dict(arrowstyle="->", lw=1.2),
             fontsize=9)

plt.legend(fontsize=9)
plt.grid(True, which='major', alpha=0.3)

plt.tight_layout()
plt.savefig(os.path.join(plot_dir, "ops.png"), dpi=300)
plt.close()

print("Enhanced Road plots generated")