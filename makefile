./build/main.o: ./src/main.cpp ./src/mathematics/Vector.hpp
	g++ -c ./src/main.cpp -o ./build/main.o

#./build/Vector.o: ./src/mathematics/Vector.cpp ./src/mathematics/Vector.hpp
#	g++ -c ./src/mathematics/Vector.cpp -o ./build/Vector.o

all: ./build/main.o
	g++ ./build/main.o -o ./build/app

run: all
	cd ./build && ./app

clean:
	rm -f ./build/*.o ./build/app