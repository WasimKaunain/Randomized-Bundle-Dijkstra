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

    # Drop any duplicate header rows that may have been appended
    df = df[df["Graph"] != "Graph"]

    # Convert all columns except Graph to numeric
    for col in df.columns:
        if col != "Graph":
            df[col] = pd.to_numeric(df[col], errors='coerce')

    # Drop rows with missing data
    df = df.dropna(subset=["#Nodes"])

    # Group by graph name and average across seeds
    averaged = df.groupby("Graph", as_index=False).mean(numeric_only=True)

    # Compute total bundle times (construction + main algo) for both versions
    averaged["Total_Bundle_Set_ms"] = (
        averaged["Bundle_construct_ms"] + averaged["Bundle_Set_ms"]
    )
    averaged["Total_Bundle_Fib_ms"] = (
        averaged["Bundle_construct_ms"] + averaged["Bundle_Fib_ms"]
    )

    # Sort by graph name
    averaged = averaged.sort_values("Graph")

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