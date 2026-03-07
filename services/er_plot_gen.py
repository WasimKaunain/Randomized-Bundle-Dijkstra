import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

#BASE DiRECTORY
base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

#CSV PATH
csv_path = os.path.join(base_dir, "Final_Output", "final_er_results.csv")

#PLOT OUTPUT DIRECTORY
plot_dir = os.path.join(base_dir, "Plots", "er")

os.makedirs(plot_dir, exist_ok=True)


# Load ER CSV
df = pd.read_csv(csv_path)

# Sort by N
df = df.sort_values("N")

# Extract columns
N = df["N"]
binary = df["dijkstra_ref"]
fib = df["dijkstra_fib"]
bundle = df["Total_bundle_dijks"]

# -------------------------------
# 1. Log-Log Runtime Plot
# -------------------------------
plt.figure()
plt.loglog(N, binary, marker='o')
plt.loglog(N, fib, marker='o')
plt.loglog(N, bundle, marker='o')
plt.xlabel("Number of vertices (N)")
plt.ylabel("Execution Time (ms)")
plt.title("ER Graphs: Runtime Comparison (Log-Log)")
plt.legend(["Binary Heap", "Fibonacci Heap", "Bundle"])
output_path = os.path.join(plot_dir, "er_runtime_loglog.png")
plt.savefig(output_path, dpi=300)
plt.close()

# -------------------------------
# 2. Ratio Plot
# -------------------------------
ratio_binary = bundle / binary
ratio_fib = bundle / fib

plt.figure()
plt.plot(N, ratio_binary, marker='o')
plt.plot(N, ratio_fib, marker='o')
plt.xscale("log")
plt.xlabel("Number of vertices (N)")
plt.ylabel("Time Ratio")
plt.title("ER Graphs: Bundle / Classical Ratio")
plt.legend(["Bundle / Binary", "Bundle / Fibonacci"])
output_path = os.path.join(plot_dir, "er_ratio_plot.png")
plt.savefig(output_path, dpi=300)
plt.close()


# -------------------------------
# 3. Bundle Time Breakdown
# -------------------------------
transform = df["transform"]
construct = df["bundle_construct"]
bd = df["bundle_dijkstra"]

plt.figure()
plt.stackplot(N, transform, construct, bd)
plt.xscale("log")
plt.xlabel("Number of vertices (N)")
plt.ylabel("Time (ms)")
plt.title("ER Graphs: Bundle Time Breakdown")
plt.legend(["Transform", "Construct", "Bundle Dijkstra"])
output_path = os.path.join(plot_dir, "er_bundle_breakdown.png")
plt.savefig(output_path, dpi=300)
plt.close()

# -------------------------------
# 4. Extract Operations
# -------------------------------
plt.figure()
plt.plot(N, df["extracts"], marker='o')
plt.xscale("log")
plt.xlabel("Number of vertices (N)")
plt.ylabel("Extract Operations")
plt.title("ER Graphs: Extract Operations Growth")
output_path = os.path.join(plot_dir, "er_extracts.png")
plt.savefig(output_path, dpi=300)
plt.close()

# -------------------------------
# 5. Decrease-Key Operations
# -------------------------------
plt.figure()
plt.plot(N, df["decrease_key"], marker='o')
plt.xscale("log")
plt.xlabel("Number of vertices (N)")
plt.ylabel("Decrease-Key Operations")
plt.title("ER Graphs: Decrease-Key Growth")
output_path = os.path.join(plot_dir, "er_decrease_key.png")
plt.savefig(output_path, dpi=300)
plt.close()

print("ER plots generated successfully.")