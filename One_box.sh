#!/bin/bash
#SBATCH -n 2
#SBATCH --mem=2048MB
#SBATCH --ntasks-per-node=2
#SBATCH --output=../log/one_box.log
srun -n2 PA01
