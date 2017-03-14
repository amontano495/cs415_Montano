# set flags and compiler, as well as file var
CC = mpic++
CLFAGS = -Wall
LIBS = -lpmi
#cpp files
FILE1 = ../src/dyn_par_brot.cpp
FILE2 = ../src/stat_par_brot.cpp
FILE3 = ../src/seq_brot.cpp
#executable files
OUTPUT1 = mpi_dynamic_brot
OUTPUT2 = mpi_static_brot
OUTPUT3 = mpi_seq_brot


all: $(OUTPUT1) $(OUTPUT2) $(OUTPUT3)

$(OUTPUT1): $(FILE1)
	$(CC) $(CFLAGS) -o $(OUTPUT1) $(FILE1) $(LIBS)

$(OUTPUT2): $(FILE2)
	$(CC) $(CFLAGS) -o $(OUTPUT2) $(FILE2) $(LIBS)

$(OUTPUT3): $(FILE3)
	$(CC) $(CLFAGS) -o $(OUTPUT3) $(FILE3) $(LIBS)

clean:
	\rm $(OUTPUT1) $(OUTPUT2) $(OUTPUT3)
