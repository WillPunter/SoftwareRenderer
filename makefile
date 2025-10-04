./build/main.o: ./src/main.cpp ./src/Maths/Vector.hpp ./src/Maths/Matrix.hpp
	g++ -c ./src/main.cpp -o ./build/main.o

#./build/Vector.o: ./src/mathematics/Vector.cpp ./src/mathematics/Vector.hpp
#	g++ -c ./src/mathematics/Vector.cpp -o ./build/Vector.o

all: ./build/main.o
	g++ ./build/main.o -o ./build/app

run: all
	cd ./build && ./app

clean:
	rm -f ./build/*.o ./build/app

./build/Window_x11.o: ./src/System/Linux_x11/Window_x11.cpp ./src/System/Linux_x11/Window_x11.hpp
	g++ -c ./src/System/Linux_x11/Window_x11.cpp -o ./build/Window_x11.o

test: ./build/Window_x11.o
	g++ ./tests/main.cpp ./build/Window_x11.o -lX11 -o ./tests/app && ./tests/app