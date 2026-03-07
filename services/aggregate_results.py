import os
import pandas as pd

# Input and output directories
INPUT_DIR = "./Output"
OUTPUT_DIR = "./Final_Output"

os.makedirs(OUTPUT_DIR, exist_ok=True)

# Column names (based on your C++ output order)
columns = [
    "filename", "N", "mcount",
    "Nt", "Mt", "k", "seed",
    "dijkstra_ref",
    "dijkstra_fib",
    "transform",
    "bundle_construct",
    "bundle_dijkstra",
    "Total_bundle_dijks",
    "R_size",
    "sum_ball_sizes",
    "extracts",
    "decrease_key",
    "ref_ok",
    "fib_ok"
]

def process_file(filepath):
    print(f"Processing: {filepath}")

    df = pd.read_csv(filepath, header=None)
    df.columns = columns

    # Convert all columns except filename to numeric
    for col in columns[1:]:
        df[col] = pd.to_numeric(df[col], errors='coerce')

    # Drop possible bad rows (like header rows accidentally read as data)
    df = df.dropna(subset=["N"])

    # Group by filename and average
    averaged = df.groupby("filename", as_index=False).mean()

    # Sort by filename
    averaged = averaged.sort_values("filename")

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


if __name__ == "__main__":
    main()