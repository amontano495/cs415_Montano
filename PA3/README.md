# PA3: "Bucketsort"
by Adam Montano

# Dependencies, Building, and Running

## Dependency Instructions
These projects depend on the MPI Library and SLIURM Running environment. 

## Building and Running
Use the makefile to make the files executable. Like so

## Building and Compiling
```bash
cd build
make

# to clean
make clean
```

## Running
This project is meant to be run with SBATCH for queuing and courtesy to the other users.
`run.sh` may be found in this project's root directory.
The contents of this file includes specification requirements for this job in the queue.

This will be run by using:
```bash
cd build

sbatch sequential.sh
sbatch parallel.sh
```
This will perform each of the programs and an image will be saved with a corresponding program.
Since this is run by another program, `sbatch`, do not change the execution mode using `chmod` at all with them.
Also, inside the sbatch files are command line arguments that can be modified.
For sequential, you can enter the amount of numbers to sort, number of desired buckets, and the last is either a 1 or 0 for verbose mode.
For parallel, you can enter the amount of numbers to sort and also a 1 or 0 for a verbose mode.

## Results
The data of the run time is saved in the `bin` folder.
