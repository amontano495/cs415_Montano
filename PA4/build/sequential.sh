#!/bin/bash
#SBATCH -n 1
#SBATCH -N 1
#SBATCH --output=../bin/seq_matrix_runtime.log
#SBATCH --time=00:05:00
srun seq_matrix matA matB
