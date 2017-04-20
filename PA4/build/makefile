# set flags and compiler, as well as file var
CC = mpic++
CLFAGS = -Wall
LIBS = -lpmi
#cpp files
FILE1 = ../src/seq_matrix.cpp
FILE2 = ../src/par_matrix.cpp
#executable files
OUTPUT1 = seq_matrix
OUTPUT2 = par_matrix

all: $(OUTPUT1) $(OUTPUT2)

$(OUTPUT1): $(FILE1)
	$(CC) $(CFLAGS) -o $(OUTPUT1) $(FILE1) $(LIBS)

$(OUTPUT2): $(FILE2)
	$(CC) $(CFLAGS) -o $(OUTPUT2) $(FILE2) $(LIBS)


clean:
	\rm $(OUTPUT1) $(OUTPUT2)
