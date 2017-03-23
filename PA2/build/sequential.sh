#!/bin/bash
#SBATCH -n 1
#SBATCh -N 1
#SBATCH --mem=2048MB
#SBATCH --time=00:05:00
#SBATCH --mail-user=amontano495@gmail.com
#SBATCH --mail-type=ALL
#SBATCH --output=../bin/seq.log
srun mpi_seq_brot
