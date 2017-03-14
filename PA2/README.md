# PA2: "Mandelbrot"
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

sbatch static.sh
sbatch dynamic.sh
sbatch sequential.sh
```
This will perform each of the programs and an image will be saved with a corresponding program.
Since this is run by another program, `sbatch`, do not change the execution mode using `chmod` at all with them.
