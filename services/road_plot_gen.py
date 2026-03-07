import os
import pandas as pd
import matplotlib.pyplot as plt

# -----------------------------
# Setup Paths 
# -----------------------------

base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

csv_path = os.path.join(base_dir, "Final_Output", "final_road_results.csv")
plot_dir = os.path.join(base_dir, "Plots", "road")

os.makedirs(plot_dir, exist_ok=True)

# -----------------------------
# Load Data
# -----------------------------

df = pd.read_csv(csv_path)

df = df.sort_values(by="N")

# -----------------------------
# Plot 1: Runtime Comparison
# -----------------------------

plt.figure()

plt.plot(df["N"], df["dijkstra_ref"], marker='o', label="Binary Heap")
plt.plot(df["N"], df["dijkstra_fib"], marker='o', label="Fibonacci Heap")
plt.plot(df["N"], df["Total_bundle_dijks"], marker='o', label="Bundle SSSP")

plt.xlabel("Number of Vertices (N)")
plt.ylabel("Time (ms)")
plt.title("Road Graphs: Runtime Comparison")
plt.legend()
plt.xscale("log")

plt.savefig(os.path.join(plot_dir, "road_runtime_comparison.png"), dpi=300)
plt.close()

# -----------------------------
# Plot 2: Fibonacci / Binary Ratio
# -----------------------------

ratio = df["dijkstra_fib"] / df["dijkstra_ref"]

plt.figure()

plt.plot(df["N"], ratio, marker='o')

plt.xlabel("Number of Vertices (N)")
plt.ylabel("Fib / Binary Time Ratio")
plt.title("Road Graphs: Fibonacci vs Binary Ratio")
plt.xscale("log")

plt.savefig(os.path.join(plot_dir, "road_fib_binary_ratio.png"), dpi=300)
plt.close()

# -----------------------------
# Plot 3: Bundle Time Breakdown
# -----------------------------

plt.figure()

plt.plot(df["N"], df["bundle_construct"], marker='o', label="Bundle Construct")
plt.plot(df["N"], df["bundle_dijkstra"], marker='o', label="Bundle Dijkstra")

if "transform" in df.columns:
    plt.plot(df["N"], df["transform"], marker='o', label="Transform")

plt.xlabel("Number of Vertices (N)")
plt.ylabel("Time (ms)")
plt.title("Road Graphs: Bundle Time Breakdown")
plt.legend()
plt.xscale("log")

plt.savefig(os.path.join(plot_dir, "road_bundle_breakdown.png"), dpi=300)
plt.close()

print("Road plots generated successfully.")