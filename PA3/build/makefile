# set flags and compiler, as well as file var
CC = mpic++
CLFAGS = -Wall
LIBS = -lpmi
#cpp files
FILE1 = ../src/seq_bucket.cpp
#executable files
OUTPUT1 = seq_bucket

all: $(OUTPUT1)

$(OUTPUT1): $(FILE1)
	$(CC) $(CFLAGS) -o $(OUTPUT1) $(FILE1) $(LIBS)

clean:
	\rm $(OUTPUT1)
