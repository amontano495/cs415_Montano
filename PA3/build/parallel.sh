#!/bin/bash
#SBATCH -n 8
#SBATCH -N 1
#SBATCH --output=../bin/par_bucket_runtime.log
#SBATCH --time=00:05:00
srun par_bucket 8000 0
