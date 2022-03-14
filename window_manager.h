#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
	Display* display_;
	Window root_;
} window_manager;

int create_window_manager(window_manager* wm);
void init_window_manager(window_manager* wm, Display* display);
void destroy_window_manager(window_manager* wm);
void run_wm(window_manager* wm);