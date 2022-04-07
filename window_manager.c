#include "window_manager.h"
#include <X11/Xutil.h>

FILE* main_log;

bool wm_detected_;
void tile_windows(window_manager* wm);

int create_window_manager(window_manager* wm){
	Display* display = XOpenDisplay(NULL);
	if(display == NULL){
		perror("");
		return 1;
	}

	init_window_manager(wm, display);
	return 0;
}
int close_window_manager(window_manager* wm){
	log_msg(wm->log, "Closing Window Manager\n");
	XCloseDisplay(wm->display_);
	return 0;
}

void init_window_manager(window_manager* wm, Display* display){
	wm->display_ = display;
	wm->root_ = DefaultRootWindow(wm->display_);
	wm->clients_ = malloc(sizeof(Window)*4096);
	for(int i = 0; i < 4096; i++){
		wm->clients_[i] = 0;
	}
	wm->log = open_log("log.log");
	ws_info inf;
	int snum = XDefaultScreen(wm->display_);
	inf.max_width = XDisplayWidth(wm->display_, snum);
	inf.max_height = XDisplayHeight(wm->display_, snum);
	wm->workspace = init_ws(inf);
	wm->focus = wm->root_;
	main_log = wm->log;
}


void destroy_window_manager(window_manager* wm){
	XCloseDisplay(wm->display_);
}

void frame(window_manager* wm, Window w, bool is_before_wm_created){
	static unsigned int BORDER_WIDTH = 3;
	static unsigned long BORDER_COLOR = 0xFF0000;
	static unsigned long BG_COLOR = 0x0000FF;
	XWindowAttributes x_window_attrs;
	XGetWindowAttributes(wm->display_, w, &x_window_attrs);
	window_layout* l = add_window(wm->workspace, w);


	if(is_before_wm_created){
		if(x_window_attrs.override_redirect || x_window_attrs.map_state != IsViewable){
			return;
		}
	}

	Window frame = XCreateSimpleWindow(
			wm->display_,
			wm->root_,
			l->x,
			l->y,
			l->width,
			l->height,
			BORDER_WIDTH,
			BORDER_COLOR,
			BG_COLOR
		);
	XSelectInput(
			wm->display_,
			frame,
			SubstructureRedirectMask | SubstructureNotifyMask
		);
	XAddToSaveSet(wm->display_, w);
	XReparentWindow(wm->display_, w, frame, 0, 0);
	XMapWindow(wm->display_, frame);
	wm->clients_[w&4095] = frame;
	XGrabButton(wm->display_,
			Button1,
			Mod1Mask,
			w,
			FALSE,
			ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
			GrabModeAsync,
			GrabModeAsync,
			0,
			0
		);
	XGrabButton(wm->display_,
			Button1,
			0,
			w,
			FALSE,
			ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
			GrabModeAsync,
			GrabModeAsync,
			0,
			0
		);
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, XK_J),
		Mod1Mask,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, XK_K),
		Mod1Mask,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, XK_F),
		Mod1Mask,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, XK_Left),
		Mod1Mask | ShiftMask,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, XK_Right),
		Mod1Mask | ShiftMask,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);
	XSetInputFocus(wm->display_, w, RevertToPointerRoot, CurrentTime);
	wm->focus = w;
}
void unframe(window_manager* wm, Window w){
	Window frame = wm->clients_[w&4095];
	XUnmapWindow(wm->display_, frame);
	XReparentWindow(wm->display_, w, wm->root_, 0, 0);
	XRemoveFromSaveSet(wm->display_, w);
	XDestroyWindow(wm->display_, frame);
	wm->clients_[w&4095] = 0;
	wm->focus = wm->root_;
	XSetInputFocus(wm->display_, wm->focus, RevertToPointerRoot, CurrentTime);
}

void run_wm(window_manager* wm){
	wm_detected_ = FALSE;
	XSetErrorHandler(&OnWMDetected);
	XSelectInput(
			wm->display_,
			wm->root_,
			SubstructureRedirectMask | SubstructureNotifyMask
		);
	XSync(wm->display_, FALSE);
	if(wm_detected_){
		log_msg(wm->log, "WM detected: Detected another Window Manager\n");
		return;
	}
	XSetErrorHandler(OnXError);

	// Remap already mapped windows
	XGrabServer(wm->display_);
	Window returned_root, returned_parent;
	Window* top_level_windows;
	unsigned int num_top_level_windows;
	XQueryTree(
			wm->display_, wm->root_,
			&returned_root, &returned_parent,
			&top_level_windows, &num_top_level_windows
		);

	for(unsigned int i = 0; i < num_top_level_windows; i++){
		frame(wm, top_level_windows[i], TRUE);
	}
	XFree(top_level_windows);
	XUngrabServer(wm->display_);
	// Close button
	XGrabKey(wm->display_,
			XKeysymToKeycode(wm->display_, XK_E),
			Mod1Mask | ShiftMask,
			wm->root_,
			FALSE,
			GrabModeAsync,
			GrabModeAsync
		);
	// Reset windows to last display
	XGrabKey(wm->display_,
			XKeysymToKeycode(wm->display_, XK_R),
			Mod1Mask,
			wm->root_,
			FALSE,
			GrabModeAsync,
			GrabModeAsync
		);
	log_msg(wm->log, "Entering main loop");
	// Main Event Loop
	for(;;){
		XEvent main_event;
		XNextEvent(wm->display_, &main_event);
		XSetInputFocus(wm->display_, wm->focus, RevertToPointerRoot, CurrentTime);

		switch(main_event.type){
			case CreateNotify:{
				XCreateWindowEvent* e = &(main_event.xcreatewindow);
			}
			break;
			case DestroyNotify:
			break;
			case ReparentNotify:
			break;
			case ConfigureRequest:{
				XConfigureRequestEvent* e = &(main_event.xconfigurerequest);
				XWindowChanges frame_changes;
				frame_changes.x = e->x;
				frame_changes.y = e->y;
				frame_changes.width = e->width;
				frame_changes.height = e->height;
				frame_changes.border_width = e->border_width;
				frame_changes.sibling = e->above;
				frame_changes.stack_mode = e->detail;

				XWindowChanges window_changes = frame_changes;
				window_changes.x = 0;
				window_changes.y = 0;

				if(wm->clients_[e->window&4095] != 0){
					Window frame = wm->clients_[e->window&4095];
					XConfigureWindow(wm->display_, frame, e->value_mask, &frame_changes);
					XConfigureWindow(wm->display_, e->window, e->value_mask, &window_changes);
				}
			}
			break;
			case UnmapNotify:{
				XUnmapEvent *e = &(main_event.xunmap);
				if(wm->clients_[e->window&4095] == 0 || e->event == wm->root_){
					break;
				}
				unframe(wm, e->window);
			}
			break;
			case MapRequest:{
				XMapRequestEvent* e = &(main_event.xmaprequest);
				frame(wm, e->window, FALSE);
				XMapWindow(wm->display_, e->window);
				char* temp = malloc(100);
				memset(temp, 0, 100);
				sprintf(temp, "New Window: %d", e->window);
				log_msg(wm->log, temp);
				free(temp);
			}
			break;
			case KeyPress:{
				handle_key_press(wm, &(main_event.xkey));
			}
			break;
			case ButtonPress:{
				handle_button_press(wm, &(main_event.xbutton));
			}
			break;
			default:
			//	printf("Ignoring Event (type: %d)...\n", main_event.type);
			break;
		}
	}
}
int OnXError(Display* display, XErrorEvent* e){
	char* temp = malloc(100);
	memset(temp, 0, 100);
	XGetErrorText(display, e->error_code, temp, 99);
	log_msg(main_log, temp);
	free(temp);
	return 0;
}
int OnWMDetected(Display* display, XErrorEvent* e){
	if(!((int)(e->error_code) == BadAccess)){
		printf("[gataleg] Something unknown went wrong\n");
	}
	wm_detected_ = TRUE;
	return 0;
}

int handle_key_press(window_manager* wm, XKeyEvent* e){
	if((e->state & (Mod1Mask | ShiftMask)) && (e->keycode == XKeysymToKeycode(wm->display_, XK_E))){
		close_window_manager(wm);
		log_msg(wm->log, "Exit hotkey pressed");
		exit(0);
	}
	if((e->state & Mod1Mask) && (e->keycode == XKeysymToKeycode(wm->display_, XK_J))){
		Window w = e->window;
		printf("Window %d accessed\n", w);
		XKillClient(wm->display_, e->window);
	}
	if((e->state & Mod1Mask) && (e->keycode == XKeysymToKeycode(wm->display_, XK_F))){
		Window w = e->window;
		XResizeWindow(wm->display_, w, wm->workspace->info.max_width, wm->workspace->info.max_height);
		XMoveWindow(wm->display_, wm->clients_[w&4095], 0, 0);
		XResizeWindow(wm->display_, wm->clients_[w&4095], wm->workspace->info.max_width, wm->workspace->info.max_height);
	}
	if((e->state & Mod1Mask) && (e->keycode == XKeysymToKeycode(wm->display_, XK_R))){
		tile_windows(wm);
	}
	if(((e->state & (Mod1Mask | ShiftMask)) && (e->keycode == XKeysymToKeycode(wm->display_, XK_Left)))
	   || ((e->state & (Mod1Mask | ShiftMask)) && (e->keycode == XKeysymToKeycode(wm->display_, XK_Right)))){
		move_horiz(wm->workspace, e->window);
		tile_windows(wm);
	}
	if((e->state & Mod1Mask) && (e->keycode == XKeysymToKeycode(wm->display_, XK_K))){
		int i = e->window;
		while(wm->clients_[(++i)%4096] == 0){
			i%=4096;
		}
		i%=4096;
		XRaiseWindow(wm->display_, wm->clients_[i]);
		XSetInputFocus(wm->display_, i, RevertToPointerRoot, CurrentTime);
		wm->focus = i;
	}
	log_msg(wm->log, "handling key press\n");
}
int handle_button_press(window_manager* wm, XButtonEvent* e){
	if(e->state == 0 && e->button == Button1){
		Window w, dummy_w;
		int *dummy;
		XQueryPointer(wm->display_, wm->root_, &w, &dummy_w, dummy, dummy, dummy, dummy, dummy);
		if(w != wm->focus){
			wm->focus = w;
		}
		XRaiseWindow(wm->display_, wm->clients_[w&4095]);
		XSetInputFocus(wm->display_, w, RevertToPointerRoot, CurrentTime);
	}
	log_msg(wm->log, "handling button press\n");
}

void tile_windows(window_manager* wm){
	ws_layout* workspace = wm->workspace;
	for(int i = 0; i < workspace->window_count; i++){
		Window w = workspace->layouts[i].xid;
		window_layout* lo = workspace->layouts+i;
		if(wm->clients_[w&4095] != 0){
			char* temp = malloc(100);
			memset(temp, 0, 100);
			sprintf(temp, "Resizing x: %d y: %d w: %d h: %d", lo->x, lo->y, lo->width, lo->height);
			log_msg(wm->log, temp);
			free(temp);
			XResizeWindow(wm->display_, w, lo->width, lo->height);
			XResizeWindow(wm->display_, wm->clients_[w&4095], lo->width, lo->height);
			XMoveWindow(wm->display_, wm->clients_[w&4095], lo->x, lo->y);
		}
	}
}
