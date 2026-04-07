#!/bin/bash
#SBATCH --job-name=bundle_dijkstra_large
#SBATCH --partition=gpu1
#SBATCH --gres=gpu:1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=64G
#SBATCH --time=72:00:00
#SBATCH --output=logs/slurm_%j.out
#SBATCH --error=logs/slurm_%j.err

set -ex pipefail

cd $SLURM_SUBMIT_DIR

########################################
# Load modules
########################################
module load gcc-13.1.0

########################################
# Build (inside compute node)
########################################
make clean
make -j16

########################################
# Generate large graphs
########################################
echo "===== GENERATING LARGE GRAPHS ====="
bash services/gen_all_graphs.sh


echo "===== DONE ====="