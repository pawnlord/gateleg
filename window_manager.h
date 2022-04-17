#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H
#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "logger.h"
#include "workspace.h"
#define TRUE 1
#define FALSE 0

typedef int bool;

typedef struct {
	Display* display_;
	Window root_;
	Window* clients_;
	Window* main_windows;
	ws_layout** workspace;
	Window focus;
	FILE* log;
	int wsnum;
} window_manager;

int create_window_manager(window_manager* wm);
void init_window_manager(window_manager* wm, Display* display);
void destroy_window_manager(window_manager* wm);
void run_wm(window_manager* wm);
int OnXError(Display* display, XErrorEvent* e);
int OnWMDetected(Display* display, XErrorEvent* e);
int handle_key_press(window_manager* wm, XKeyEvent* e);
int handle_button_press(window_manager* wm, XButtonEvent* e);

#endif
