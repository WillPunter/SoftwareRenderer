/*  LinuxX11.cpp

    Contains common functionality for code supporting the Linux X11
    platform. */

#include "./../RenderWindow.hpp"
#include "X11RGBARenderWindow.hpp"

namespace System {

RenderWindow* make_render_window(std::string title, int width,
    int height) {
    /*  TODO - identify the details about the video hardware and settings of
        the running device and use it to decide which type of render window
        to construct.
        
        For now, the vast majority of Linux systems running X11 will support
        TrueColor, so use RGBA buffer format and create an
        X11RGBARenderWindow. */
    
    /*  Note that since the X11RGBARenderWindow constructor is private and this
        is a friend function (but std::make_unique is not), we cannot use
        make_unique, hence the slightly odd construction. */
    return new X11RGBARenderWindow(title, width, height);
}

}
