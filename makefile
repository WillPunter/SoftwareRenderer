./build/main.o: ./src/main.cpp ./src/mathematics/Vector.hpp
	g++ -c ./src/main.cpp -o ./build/main.o

all: ./build/main.o
	g++ ./build/main.o -o ./build/app

run: all
	cd ./build && ./app

clean:
	rm -f ./build/*.o ./build/app