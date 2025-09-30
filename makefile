main.o: main.cpp
	g++ -c main.cpp -c main.o

all: main.o
	g++ main.o -o app