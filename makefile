SRC_PATH := ./src
SYSTEM_PATH := $(SRC_PATH)/System
LINUXX11_PATH := $(SYSTEM_PATH)/LinuxX11
MATHS_PATH := $(SRC_PATH)/Maths
GRAPHICS_PATH := $(SRC_PATH)/Graphics

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

# Matematics module - all code currenty headers.

# Graphics module - TODO.

# All - compile all modules (do not link into library though).
all: Systems_Linux

# Examples
pixels: all
	$(CC) $(BUILD_PATH)/X11Window.o $(BUILD_PATH)/X11RGBARenderWindow.o $(BUILD_PATH)/LinuxX11.o $(EXAMPLES_PATH)/pixels/main.cpp $(LFLAGS) -o $(BUILD_PATH)/pixels
	cd build && ./pixels

# Clean
clean:
	rm -f $(BUILD_PATH)/*.o