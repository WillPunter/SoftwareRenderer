/*  Window_x11.cpp */

#include "Window_x11.hpp"
#include <stdexcept>
#include <iostream>

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

    /*  Select events to handle. TODO - set window to handle all events and
        pass the event handling logic to a higher level of abstraction. */
    XSelectInput(this->display, this->window, ExposureMask | ButtonPressMask);

    /*  Show window. */
    XMapWindow(this->display, this->window);

    /*  Set window title. */
    XStoreName(this->display, this->window, this->title.c_str());

    /*  A note about X11: the window manager is responsible for providing the
        window titlebar and buttons (minimise, maximise, close, etc.). The
        window manager is itself an X client, therefore this application cannot
        directly intercept the close button press as an event - this is done by
        the window manager. Instead, we need to use properties (a server-side
        resource, reference by ids called Atoms, and used to facillitate
        communication between multiple clients connected to the same X server).
        Specifically, for checking if the window manager closed the window, we
        need to check for the window manager delete window message
        WM_DELETE_WINDOW.
        
        XInternAtom is called by a client to obtain the atom (id) associated
        with a property (a string referring to some data stored by the server).
        Note that according to the docs, the reason both an atom and a string
        id are used is for efficiency - string ids are easy for humans to
        remember, but atom ids are simpler to manipulate digitally. */
    this->window_manager_destroy_window_id =
        XInternAtom(this->display, "WM_DELETE_WINDOW", False);
    
    /*  Bind the obtained atom to the WM_PROTOCOLS property on this window.
        This actually takes a lit of atoms to bind to WM_PROTOCOL but since we
        only use the atom obtained just above we give a list length of 1. */
    XSetWMProtocols(this->display, this->window,
        &this->window_manager_destroy_window_id, 1);

    /*  Flush output buffer (called event queue on other platforms - not to be
        confused with the render buffer for outputting pixels) to get the
        window to display immediately. */
    XFlush(this->display);
}

/*  Handle events - the Window interface provides specific information about
    state and events to the user, with the idea of abstracting away all event
    handling details behind an interface for accessing things like key state,
    mouse coordinates, etc. directly.
    
    The return value is true if execution should continue and false if it
    should not go ahead (e.g. handling an event which may throw off delta time,
    receiving a close event, etc.). */
bool Window_x11::handle_events() {
    XEvent event;

    bool frame_continue = true;
    
    while (XPending(this->display)) {
        XNextEvent(this->display, &event);

        frame_continue = frame_continue && this->multiplex_event(&event);
    }

    return frame_continue;
}

void Window_x11::draw_pixel(int x, int y, uint8_t red, uint8_t green,
    uint8_t blue) {
    // TODO.
}

void Window_x11::close_window() {
    this->open = false;
    XDestroyWindow(this->display, this->window);
}

bool Window_x11::is_closed() const {
    return !this->open;
}

/*  Multiplex on a given event - check event type and execute the correct
    handling logic. */
bool Window_x11::multiplex_event(XEvent* event) {
    bool continue_frame = true;

    switch (event->type) {
        case Expose: {
            std::cout << "Expose event." << std::endl;
            break;
        }

        /*  Received message from other X client. */
        case ClientMessage: {
            std::cout << "Received a client message!!!" << std::endl;

            /*  The window manager is a client and will send a message to this
                application, typically, when the close (X) button is clicked.
                We check for this button press by checking if it's id matches
                the atom of the WM_DESTROY_WINDOW property. */
            if (static_cast<Atom>(event->xclient.data.l[0]) ==
                this->window_manager_destroy_window_id) {
                continue_frame = false;
                this->close_window();
            };

            break;
        }
    }

    return true;
}

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