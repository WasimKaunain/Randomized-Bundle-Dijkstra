import pandas as pd
import matplotlib.pyplot as plt

# Load CSV
df = pd.read_csv("Profile Data.csv")

# Keep only correct runs
df = df[(df["ref_correctness"] == 1) & (df["fib_correctness"] == 1)]

# Compute density factor m/n
df["density"] = df["#Edges"] / df["#Node"]

# Average over seeds (same graph)
grouped = df.groupby(["#Node", "#Edges", "density"]).agg({
    "Ref_dijkstra_ms": "mean",
    "fib_dijksra_ms": "mean",
    "Total_bundle_dijkstra": "mean"
}).reset_index()

# Sort for plotting
grouped = grouped.sort_values("#Node")

# Plot for each density
plt.figure(figsize=(9, 6))

for density in sorted(grouped["density"].unique()):
    subset = grouped[grouped["density"] == density]

    N = subset["#Node"]

    plt.loglog(
        N,
        subset["Ref_dijkstra_ms"],
        marker='o',
        linestyle='--',
        label=f"Dijkstra PQ (m={int(density)}n)"
    )

    plt.loglog(
        N,
        subset["fib_dijksra_ms"],
        marker='s',
        linestyle=':',
        label=f"Dijkstra Fib (m={int(density)}n)"
    )

    plt.loglog(
        N,
        subset["Total_bundle_dijkstra"],
        marker='^',
        linestyle='-',
        label=f"Bundle Dijkstra (m={int(density)}n)"
    )

plt.xlabel("Number of nodes (log scale)")
plt.ylabel("Execution time (ms, log scale)")
plt.title("Average Execution Time vs Graph Size for Different Densities")
plt.legend()
plt.grid(True, which="both", linestyle="--", linewidth=0.5)

plt.tight_layout()
plt.savefig("execution_time_loglog_by_density.png", dpi=300)
plt.show()
