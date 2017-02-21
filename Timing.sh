#!/bin/bash
#SBATCH -n 2
#SBATCH --mem=2048MB
#SBATCH --ntasks-per-node=1
#SBATCH --output=../log/timing.log
srun -n2 PA01-2
