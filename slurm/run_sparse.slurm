#!/bin/bash
#SBATCH --job-name=dijkstra_sparse
#SBATCH --partition=gpu2
#SBATCH --gres=gpu:1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=64G
#SBATCH --time=5:00:00
#SBATCH --output=slurm_%j.out
#SBATCH --error=slurm_%j.err

set -ex pipefail

cd $SLURM_SUBMIT_DIR

# Load compiler module (needed for libasan)
module load gcc-13.1.0

# Optional: clean and rebuild inside the compute node
make clean
make -j16

# Run experiment
bash scripts/run_sparse.sh