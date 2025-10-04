/*  Window_x11.cpp */

#include "Window_x11.hpp"
#include <stdexcept>

namespace System_Linux {

/*  Constructor - this initialises an X11 window. */
Window_x11::Window_x11(std::string title, int width, int height) :
    title{title}, width{width}, height{height} {
    /*  Establish a connection with the X server - X calls this connection a
        "display" (well it's a bit more nuanced - see the X11 docs for more
        information). If it is not possible to establish a connection, throw
        an error that is not meant to be caught - this failure is unrecoverable
        for the purposes of this program. */
    this->display = XOpenDisplay(NULL);

    if (!this->display) {
        throw std::runtime_error("Error - could not establish connection"
            " to X server.");
    }

    /*  Get system default screen id - this is the screen we will create the
        window on. */
    unsigned int screen_id = DefaultScreen(display);

    /*  Create window. */
    this->window = XCreateSimpleWindow(
        display,    /* Display (X server connection). */
        DefaultRootWindow(display), /* Parent - root window of display. */
        0, 0,   /* X and Y coordinates - 0,0 is default. */
        width, height,  /* Window width and height. */
        2, WhitePixel(display, screen_id),  /* Border width and colour. */
        BlackPixel(display, screen_id)   /* Background colour. */
    );

    /*  Show window. */
    XMapWindow(this->display, this->window);
}

void Window_x11::draw_pixel(int x, int y, uint8_t red, uint8_t green,
    uint8_t blue) {
    // TODO.
};


}

namespace System {

/*  The Window_x11 (and all system-specific types) are not meant to be part of
the graphics engine API - we instead impleent the platform-independent
Window interface and construct a concrete implementation through
make_window. */

std::unique_ptr<Window> make_window(
    std::string title,
    int width,
    int height
) {
    /*  Annoyingly we cannot use make unique just here because this is a friend
        function - make_unique is not, and friendship is not nested or
        transitive. Hence, the slightly unwieldly syntax below. */
    return std::unique_ptr<System_Linux::Window_x11>(
        new System_Linux::Window_x11(title, width, height));
}

}