#!/bin/bash
#SBATCH -n 1
#SBATCH -N 1
#SBATCH --mem=2048MB
#SBATCH --output=../bin/seq_bucket_runtime.log
#SBATCH --time=00:05:00
srun seq_bucket input
