#!/bin/bash
#SBATCH -n 2
#SBATCH --mem=2048MB
#SBATCH --ntasks-per-node=1
#SBATCH --output=../log/two_box.log
srun -n2 PA01
