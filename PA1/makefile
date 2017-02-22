# set flags and compiler, as well as file var
CC=mpicc
CXXFLAGS=-Wall
LIBS=-lpmi
# feel free to either add files here (for more compilcated src's), or create more vars, or just manually type them in. Up to you.
FILE1=./src/PA01.c
FILE2=./src/PA01-2.c
# uncomment below if/when you have a folder called include that holds potential header files
#INCLUDES=-I../include
OUTPUT1=PA01
OUTPUT2=PA01-2

all: $(OUTPUT1) $(OUTPUT2)

$(OUTPUT1): $(FILE1)
	$(CC) $(CXXFLAGS) -o $(OUTPUT1) $(FILE1) $(LIBS)

$(OUTPUT2): $(FILE2)
	$(CC) $(CXXFLAGS) -o $(OUTPUT2) $(FILE2) $(LIBS)
# use this to run
single: $(OUTPUT)
	sbatch One_box.sh

double: $(OUTPUT)
	sbatch Two_box.sh

timing: $(OUTPUT2)
	sbatch Timing.sh

clean:
	-@if rm $(OUTPUT1) $(OUTPUT2) 2>/dev/null || true; then echo "Cleaned"; else echo "Nothing to clean"; fi
