#include "client.hpp"

Client::Client(Display* dpy, Window w) : dpy_(dpy), window_(w) {

}

Client::~Client() {

}


void Client::SetBorderWidth(unsigned int width) {
    XSetWindowBorderWidth(dpy_, window_, width);
}

void Client::SetBorderColor(unsigned long color) {
    XSetWindowBorder(dpy_, window_, color);
}

Window Client::window() {
    return window_;
}
