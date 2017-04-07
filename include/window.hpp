
#ifndef __window_hpp__
#define __window_hpp__ 

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <iostream>

#include "main.hpp"

void* windowThread(void* id);
void CreateWindow(const RaytraceOptions &options, RaytraceBitmap &bitmap);
void UpdateImage(const RaytraceOptions &options);


#endif

