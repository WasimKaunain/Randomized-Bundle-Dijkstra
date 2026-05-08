import os
import pandas as pd

# Input and output directories
INPUT_DIR = "./Output"
OUTPUT_DIR = "./Final_Output"

os.makedirs(OUTPUT_DIR, exist_ok=True)


def process_file(filepath):
    print(f"Processing: {filepath}")

    # Read CSV with header (main.cpp writes header on first run)
    df = pd.read_csv(filepath)
    df.columns = df.columns.str.strip()
    print(df.columns)
    
    # Convert all non-Graph columns to numeric
    numeric_cols = df.columns.drop("Graph")
    df[numeric_cols] = df[numeric_cols].apply(pd.to_numeric, errors='coerce')

    # Drop rows with missing data
    df = df.dropna(subset=["#Nodes"])

    # Group by graph name and average across seeds
    averaged = df.groupby("Graph", as_index=False).mean(numeric_only=True).round(1)
    print(averaged.columns)

    
    # Compute total bundle times
    if "Transform_ms" in averaged.columns:
        averaged["Total_Bundle_Set_ms"] = (averaged["Bundle_construct_ms"] + averaged["Bundle_Set_ms"] + averaged["Transform_ms"])
        averaged["Total_Bundle_Fib_ms"] = (averaged["Bundle_construct_ms"] + averaged["Bundle_Fib_ms"] + averaged["Transform_ms"])
        averaged["Total_Bundle_PQ_ms"] = (averaged["Bundle_construct_ms"] + averaged["Bundle_PQ_ms"] + averaged["Transform_ms"])

    else:
        averaged["Total_Bundle_Set_ms"] = (averaged["Bundle_construct_ms"] + averaged["Bundle_Set_ms"])
        averaged["Total_Bundle_Fib_ms"] = (averaged["Bundle_construct_ms"] + averaged["Bundle_Fib_ms"])
        averaged["Total_Bundle_PQ_ms"] = (averaged["Bundle_construct_ms"] + averaged["Bundle_PQ_ms"])

    # Reorder columns so Total_* columns come after Bundle_construct_ms
    cols = list(averaged.columns)

    bundle_idx = cols.index("Bundle_construct_ms")

    # Remove total columns temporarily
    for col in ["Total_Bundle_Set_ms","Total_Bundle_Fib_ms","Total_Bundle_PQ_ms"]:
        cols.remove(col)

    # Insert them after Bundle_construct_ms
    new_cols = (cols[:bundle_idx + 1] + ["Total_Bundle_Set_ms","Total_Bundle_Fib_ms","Total_Bundle_PQ_ms"] + cols[bundle_idx + 1:])

    averaged = averaged[new_cols]

    # Round all numeric columns to 1 decimal place
    averaged = averaged.round(1)

    # Sort by graph size
    averaged = averaged.sort_values(by=["#Nodes", "#Edges"])

    return averaged


def main():
    for file in os.listdir(INPUT_DIR):
        if file.endswith(".csv"):
            input_path = os.path.join(INPUT_DIR, file)

            averaged_df = process_file(input_path)

            output_filename = f"final_{file}"
            output_path = os.path.join(OUTPUT_DIR, output_filename)

            averaged_df.to_csv(output_path, index=False)

            print(f"Saved: {output_path}")
            print(averaged_df.to_string(index=False))
            print()


if __name__ == "__main__":
    main()