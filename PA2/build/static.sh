#!/bin/bash
#SBATCH -n 8
#SBATCH -N 1
#SBATCH --mem=2048MB
#SBATCH --time=00:10:00
#SBATCH --mail-user=amontano495@gmail.com
#SBATCH --mail-type=ALL
#SBATCH --output=../bin/static.log
srun mpi_static_brot
