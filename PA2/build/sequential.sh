#!/bin/bash
#SBATCH -n 1
#SBATCH --mem=2048MB
#SBATCH --time=00:10:00
#SBATCH --mail-user=amontano495@gmail.com
#SBATCH --mail-type=ALL
#SBATCH --output=../bin/seq_40000.log
srun mpi_seq_brot
