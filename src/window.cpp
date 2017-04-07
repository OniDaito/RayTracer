/**
* @brief X11, Windows or OSX style windowing
* @file window.cpp
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 08/01/2016
*
*/

#include "window.hpp"

using namespace std;

// X Display stuff - naughty globals! Mostly here because of the thread
Display *display;
Window window;
int xid;
GC gc;
XImage* render_image;
char* render_image_data; // sadly we need another buffer ><



void* WindowThread(void* id) {
  XEvent e;
  while (true) {
    XNextEvent(display, &e);
    /*if (e.type == Expose) {

    } 
    if (e.type == KeyPress){
    
    }*/
    if (e.type == DestroyNotify) {
      std::cout << "Caught Quit" << std::endl;
      break;
    }
  }
    
  XCloseDisplay(display);
  RaysQuit();
}


void CreateWindow(const RaytraceOptions &options, RaytraceBitmap &bitmap) {

  std::cout << "Creating live view window..." << std::endl;

  // Allocate XImage Buffer

  XInitThreads();

  display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Cannot open display\n");
  }

  xid = DefaultScreen(display);

  window = XCreateSimpleWindow(display, RootWindow(display, xid), 10, 10, options.width, options.height, 1,
                         BlackPixel(display, xid), WhitePixel(display, xid));

  XGCValues values;
  /* which values in 'values' to check when creating the GC. */

  values.cap_style = CapButt;
  values.join_style = JoinBevel;

  unsigned long valuemask = GCCapStyle | GCJoinStyle;
  
  gc = XCreateGC(display,window,valuemask, &values);
  
  int depth = DefaultDepth(display, DefaultScreen(display));

  XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);
  XMapWindow(display, window);

  Visual *visual=DefaultVisual(display, 0);

  //render_image_data = new char[ 4 * options.width * options.height];

  render_image = XCreateImage(display, visual, depth, ZPixmap, 0, &(bitmap.data[0]), options.width,options.height, 32,0);

  pthread_t thread;
  int rc = pthread_create(&thread, NULL, WindowThread, nullptr);
  if(rc){
    std::cout<< "ERROR; return code from pthread_create() is " << rc << std::endl;
  }

}


void UpdateImage(const RaytraceOptions &options) {
  XPutImage(display, window, gc, render_image, 0, 0, 0, 0, options.width, options.height);
}

