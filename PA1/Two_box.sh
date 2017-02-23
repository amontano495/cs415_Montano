#!/bin/bash
#SBATCH -n 2
#SBATCH --mem=2048MB
#SBATCH --ntasks-per-node=1
#SBATCH --output=./log/two_box.log
#SBATCH --time=00:10:00
srun -n2 PA01
