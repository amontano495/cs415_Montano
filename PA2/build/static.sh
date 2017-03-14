#!/bin/bash
#SBATCH -n 8
#SBATCH --mem=2048MB
#SBATCH --time=00:10:00
#SBATCH --mail-user=amontano495@gmail.com
#SBATCH --mail-type=ALL
#SBATCH --output=../bin/stat_8proc_40000
srun mpi_static_brot
