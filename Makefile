CC = g++
CFLAGS = -std=c++11 -O3 -Wall -fopenmp

vs : main.o grid.o
	$(CC) $(CFLAGS) -o $@ main.o grid.o

main.o : main.cpp main.h
	$(CC) $(CFLAGS) -c main.cpp -o $@

grid.o : grid.cpp grid.h
	$(CC) $(CFLAGS) -c grid.cpp -o $@

clean:
	rm *.o
	rm vs
