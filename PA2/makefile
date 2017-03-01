CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

main : main.o PIMFuncs.o
	$(CC) $(LFLAGS) main.o PIMFuncs.o -o main

main.o : main.cpp
	$(CC) $(CFLAGS) main.cpp

PIMFuncs.o : PIMFuncs.cpp PIMFuncs.h
	$(CC) $(CFLAGS) PIMFuncs.cpp

clean:
	\rm *.o main
