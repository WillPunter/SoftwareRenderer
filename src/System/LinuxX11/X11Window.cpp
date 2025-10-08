/*  X11Window.cpp */

#include "X11Window.hpp"
#include <stdexcept>
#include <iostream>

namespace System {

X11Window::X11Window(std::string title, int width, int height) :
    width{width}, height{height} {
    /*  Establish connection with X server. NULL indicates default / local
        server should be connected to. */
    this->server_connection = XOpenDisplay(NULL);

    if (!this->server_connection) {
        throw std::runtime_error("X11 Error - could not connect to X server.");
    }

    /*  Get default screen by number. */
    this->screen_id = DefaultScreen(this->server_connection);

    /*  Create window. */
    this->window = XCreateSimpleWindow(
        this->server_connection,
        DefaultRootWindow(this->server_connection),
        0, 0,
        width, height,
        2, WhitePixel(this->server_connection, this->screen_id),
        BlackPixel(this->server_connection, this->screen_id)
    );

    /*  Select events with event mask. */
    unsigned long event_mask = ExposureMask | ButtonPressMask;
    XSelectInput(this->server_connection, this->window, event_mask);

    /*  Set window title. */
    XStoreName(this->server_connection, this->window, title.c_str());

    /*  Note that the close (X) button is not an inherent part of windows in X.
        In fact, the window buttons are actually provided by the window
        manager, which itself is a client of the X server. Therefore, we must
        communicate with the window manager via the server to decide how to
        respond to the close button press, and handle it gracefully.
        
        The X server stores properties which are named collections of data. The
        WM_PROTOCOL property for a window maintains a list of protocols which
        this application and the window manager engage in. We want to set the
        WM_PROTOCOLS property for our application to WM_DELETE_WINDOW atom (an
        atom in X11 is just a named value that does not contain any other data).
        Therefore, we look up the id of the atom for the WM_DELETE_WINDOW
        protocol by it's name and then add it to the WM_PROTOCOLS property.
        
        Agreeing to the WM_DELETE_WINDOW protocol means that when the window
        is closed (e.g. by pressing the close button), then client will be sent
        a ClientMessage event by the window manager via the server. Hence in
        the later event handling code we must check for this event and check if
        it corresponds to the message we expect from the window manager. */
    this->window_manager_delete_window_id =
        XInternAtom(this->server_connection, "WM_DELETE_WINDOW", False);
    
    XSetWMProtocols(this->server_connection, this->window,
        &this->window_manager_delete_window_id, 1);

    /*  Get visual data. */
    this->visual_info = DefaultVisual(this->server_connection,
        this->screen_id);

    /*  Show window. */
    XMapWindow(this->server_connection, this->window);
}

X11Window::~X11Window() {
    XDestroyWindow(this->server_connection, this->window);
    XCloseDisplay(this->server_connection);
}

bool X11Window::handle_events() {
    XEvent event;

    bool frame_continue = true;

    while (XPending(this->server_connection)) {
        XNextEvent(this->server_connection, &event);
        frame_continue = frame_continue && this->multiplex_event(event);
    }

    return frame_continue;
}

bool X11Window::is_open() {
    return !this->should_close;
}

void X11Window::close_window() {
    this->should_close = true;
}

bool X11Window::multiplex_event(XEvent& event) {
    bool frame_should_continue = true;

    switch (event.type) {
        case Expose: {
            std::cout << "Expose event - TODO." << std::endl;
            break;
        }

        case ButtonPress: {
            std::cout << "Button press - TODO." << std::endl;
            break;
        }

        /*  Because we set the WM_PROTOCOLS property in the constructor for our
            window, when the close button (owned by the window manager) is
            pressed, the window manager may send this application a
            ClientMessage event via the X server. To confirm that this is a
            delete window event, we interpret the event as a client message
            type and then check that the first element of it's data field is
            equal to the atom that represents the WM_DELETE_WINDOW
            protocol. */
        case ClientMessage: {
            if (
                static_cast<Atom>(event.xclient.data.l[0]) ==
                this->window_manager_delete_window_id
            ) {
                this->should_close = true;
                frame_should_continue = false;
            }

            break;
        }
    }

    return frame_should_continue;
}

}