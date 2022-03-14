#include "window_manager.h"

int create_window_manager(window_manager* wm){
	Display* display = XOpenDisplay(NULL);
	if(display == NULL){
		perror("");
		return 1;
	}

	init_window_manager(wm, display);
	return 0;
}

void init_window_manager(window_manager* wm, Display* display){
	wm->display_ = display;
	wm->root_ = DefaultRootWindow(wm->display_);
}

void destroy_window_manager(window_manager* wm){
	XCloseDisplay(wm->display_);
}

void run_wm(window_manager* wm){}