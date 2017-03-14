#!/bin/bash
#SBATCH -n 1
#SBATCh -N 2
#SBATCH --mem=2048MB
#SBATCH --time=00:10:00
#SBATCH --mail-user=amontano495@gmail.com
#SBATCH --mail-type=ALL
#SBATCH --output=../bin/sequential.log
srun mpi_seq_brot
