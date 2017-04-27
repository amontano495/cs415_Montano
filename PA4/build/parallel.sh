#!/bin/bash
#SBATCH -n 9
#SBATCH -N 2
#SBATCH --output=../bin/par_matrix_runtime.log
#SBATCH --time=00:05:00
srun par_matrix matA matB
