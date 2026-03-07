import os
import pandas as pd
import matplotlib.pyplot as plt

# -----------------------------
# Setup Paths
# -----------------------------

base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

er_csv = os.path.join(base_dir, "Final_Output", "final_er_results.csv")
sparse_csv = os.path.join(base_dir, "Final_Output", "final_sparse_results.csv")
road_csv = os.path.join(base_dir, "Final_Output", "final_road_results.csv")

plot_dir = os.path.join(base_dir, "Plots", "combined_bundle")
os.makedirs(plot_dir, exist_ok=True)

# -----------------------------
# Load Data
# -----------------------------

er = pd.read_csv(er_csv).sort_values(by="N")
sparse = pd.read_csv(sparse_csv).sort_values(by="N")
road = pd.read_csv(road_csv).sort_values(by="N")

# -----------------------------
# Plot 1: Bundle Runtime Comparison
# -----------------------------

plt.figure()

plt.plot(er["N"], er["Total_bundle_dijks"], marker='o', label="ER")
plt.plot(sparse["N"], sparse["Total_bundle_dijks"], marker='o', label="Sparse")
plt.plot(road["N"], road["Total_bundle_dijks"], marker='o', label="Road")

plt.xlabel("Number of Vertices (N)")
plt.ylabel("Bundle Time (ms)")
plt.title("Bundle SSSP Runtime Across Graph Families")
plt.legend()
plt.xscale("log")

plt.savefig(os.path.join(plot_dir, "bundle_runtime_comparison.png"), dpi=300)
plt.close()

# -----------------------------
# Plot 2: Bundle / Binary Ratio
# -----------------------------

plt.figure()

er_ratio = er["Total_bundle_dijks"] / er["dijkstra_ref"]
sparse_ratio = sparse["Total_bundle_dijks"] / sparse["dijkstra_ref"]
road_ratio = road["Total_bundle_dijks"] / road["dijkstra_ref"]

plt.plot(er["N"], er_ratio, marker='o', label="ER")
plt.plot(sparse["N"], sparse_ratio, marker='o', label="Sparse")
plt.plot(road["N"], road_ratio, marker='o', label="Road")

plt.xlabel("Number of Vertices (N)")
plt.ylabel("Bundle / Binary Time Ratio")
plt.title("Bundle vs Binary Across Graph Families")
plt.legend()
plt.xscale("log")

plt.savefig(os.path.join(plot_dir, "bundle_binary_ratio_comparison.png"), dpi=300)
plt.close()

# -----------------------------
# Plot 3: Bundle / Fibonacci Ratio
# -----------------------------

plt.figure()

er_ratio = er["Total_bundle_dijks"] / er["dijkstra_fib"]
sparse_ratio = sparse["Total_bundle_dijks"] / sparse["dijkstra_fib"]
road_ratio = road["Total_bundle_dijks"] / road["dijkstra_fib"]

plt.plot(er["N"], er_ratio, marker='o', label="ER")
plt.plot(sparse["N"], sparse_ratio, marker='o', label="Sparse")
plt.plot(road["N"], road_ratio, marker='o', label="Road")

plt.xlabel("Number of Vertices (N)")
plt.ylabel("Bundle / Fibonacci Time Ratio")
plt.title("Bundle vs Fibonacci Across Graph Families")
plt.legend()
plt.xscale("log")

plt.savefig(os.path.join(plot_dir, "bundle_fibonacci_ratio_comparison.png"), dpi=300)
plt.close()

print("Bundle-focused combined plots generated successfully.")