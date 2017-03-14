#!/bin/bash
#SBATCH -n 17
#SBATCH --mem=2048MB
#SBATCH --time=00:10:00
#SBATCH --mail-user=amontano495@gmail.com
#SBATCH --mail-type=ALL
#SBATCH --output=../bin/dynamic_17proc_40000.log
srun mpi_dynamic_brot
