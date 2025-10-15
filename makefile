SRC_PATH := ./src
SYSTEM_PATH := $(SRC_PATH)/System
LINUXX11_PATH := $(SYSTEM_PATH)/LinuxX11
MATHS_PATH := $(SRC_PATH)/Maths
GRAPHICS_PATH := $(SRC_PATH)/Graphics
RESOURCES_PATH := $(SRC_PATH)/Resources

BUILD_PATH := ./build
EXAMPLES_PATH := ./examples

CC := g++

CFLAGS := -c
LFLAGS := -lX11

# System module.
$(BUILD_PATH)/X11Window.o: $(LINUXX11_PATH)/X11Window.cpp $(LINUXX11_PATH)/X11Window.hpp
	$(CC) $(CFLAGS) $(LINUXX11_PATH)/X11Window.cpp -o $(BUILD_PATH)/X11Window.o

$(BUILD_PATH)/X11RGBARenderWindow.o: $(BUILD_PATH)/X11Window.o $(LINUXX11_PATH)/X11RGBARenderWindow.cpp $(LINUXX11_PATH)/X11RGBARenderWindow.hpp
	$(CC) $(CFLAGS) $(LINUXX11_PATH)/X11RGBARenderWindow.cpp -o $(BUILD_PATH)/X11RGBARenderWindow.o

$(BUILD_PATH)/LinuxX11.o: $(BUILD_PATH)/X11RGBARenderWindow.o $(LINUXX11_PATH)/LinuxX11.cpp
	$(CC) $(CFLAGS) $(LINUXX11_PATH)/LinuxX11.cpp -o $(BUILD_PATH)/LinuxX11.o

Systems_Linux: $(BUILD_PATH)/LinuxX11.o

# Matematics module.
$(BUILD_PATH)/Transform.o: $(MATHS_PATH)/Transform.cpp $(MATHS_PATH)/Transform.hpp $(MATHS_PATH)/Vector.hpp $(MATHS_PATH)/Matrix.hpp
	$(CC) $(CFLAGS) $(MATHS_PATH)/Transform.cpp -o $(BUILD_PATH)/Transform.o

Maths: $(BUILD_PATH)/Transform.o

# Graphics module.
$(BUILD_PATH)/Rasteriser.o: $(GRAPHICS_PATH)/Rasteriser.cpp $(GRAPHICS_PATH)/Rasteriser.hpp
	$(CC) $(CFLAGS) $(GRAPHICS_PATH)/Rasteriser.cpp -o $(BUILD_PATH)/Rasteriser.o

$(BUILD_PATH)/Model.o: $(GRAPHICS_PATH)/Model.cpp $(GRAPHICS_PATH)/Model.hpp
	$(CC) $(CFLAGS) $(GRAPHICS_PATH)/Model.cpp -o $(BUILD_PATH)/Model.o

$(BUILD_PATH)/Renderer.o: $(GRAPHICS_PATH)/Renderer.cpp $(GRAPHICS_PATH)/Renderer.hpp
	$(CC) $(CFLAGS) $(GRAPHICS_PATH)/Renderer.cpp -o $(BUILD_PATH)/Renderer.o

Graphics: $(BUILD_PATH)/Rasteriser.o $(BUILD_PATH)/Model.o $(BUILD_PATH)/Renderer.o

# Resources module.
$(BUILD_PATH)/load_mesh.o: $(RESOURCES_PATH)/load_mesh.cpp $(RESOURCES_PATH)/load_mesh.hpp
	$(CC) $(CFLAGS) $(RESOURCES_PATH)/load_mesh.cpp -o $(BUILD_PATH)/load_mesh.o

Resources: $(BUILD_PATH)/load_mesh.o

# All - compile all modules (do not link into library though).
all: Systems_Linux Maths Graphics Resources

# Examples
pixels: all
	$(CC) $(BUILD_PATH)/X11Window.o $(BUILD_PATH)/X11RGBARenderWindow.o $(BUILD_PATH)/LinuxX11.o $(EXAMPLES_PATH)/pixels/main.cpp $(LFLAGS) -o $(BUILD_PATH)/pixels
	cd build && ./pixels

lines: all
	$(CC) $(BUILD_PATH)/X11Window.o $(BUILD_PATH)/X11RGBARenderWindow.o $(BUILD_PATH)/LinuxX11.o $(BUILD_PATH)/Rasteriser.o $(EXAMPLES_PATH)/lines/main.cpp $(LFLAGS) -o $(BUILD_PATH)/lines
	cd build && ./lines

models: all
	$(CC) $(BUILD_PATH)/X11Window.o $(BUILD_PATH)/X11RGBARenderWindow.o $(BUILD_PATH)/LinuxX11.o $(BUILD_PATH)/Transform.o $(BUILD_PATH)/Rasteriser.o $(BUILD_PATH)/Model.o $(BUILD_PATH)/Renderer.o $(BUILD_PATH)/load_mesh.o $(EXAMPLES_PATH)/models/main.cpp $(LFLAGS) -o $(BUILD_PATH)/models
	cd build && ./models

# Clean
clean:
	rm -f $(BUILD_PATH)/*.o