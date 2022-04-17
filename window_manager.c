#include "window_manager.h"
#include <X11/Xutil.h>
#define BORDER 20
#define MODMASK Mod4Mask
FILE* main_log;

bool wm_detected_;
bool pointer_initialized = FALSE;
void tile_windows(window_manager* wm);
const char* terminal = "lxterminal\0";

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
	static unsigned int BORDER_WIDTH = 0;
	static unsigned long BORDER_COLOR = 0x000000;
	static unsigned long BG_COLOR = 0x000000;
	XWindowAttributes x_window_attrs;
	XGetWindowAttributes(wm->display_, w, &x_window_attrs);
	window_layout* l = add_window(wm->workspace, w);
	log_msg(wm->log, "test");
	if(x_window_attrs.override_redirect == True){
		log_msg(wm->log, "override called");
		return;
	}

	if(is_before_wm_created){
		if(x_window_attrs.override_redirect || x_window_attrs.map_state != IsViewable){
			return;
		}
	}
	Window frame = XCreateSimpleWindow(
			wm->display_,
			wm->root_,
			l->x + BORDER,
			l->y + BORDER,
			l->width - BORDER*2,
			l->height - BORDER*2,
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

	XResizeWindow(wm->display_, w, l->width - BORDER*2, l->height - BORDER*2);

	XMapRaised(wm->display_, frame);

	wm->clients_[w&4095] = frame;

/*	XGrabButton(wm->display_,
			Button1,
			MODMASK,
			frame,
			FALSE,
			ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
			GrabModeAsync,
			GrabModeAsync,
			0,
			0
		);*/
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, XK_J),
		MODMASK,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, XK_Tab),
		Mod1Mask,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, XK_F),
		MODMASK,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, XK_H),
		MODMASK,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, XK_G),
		MODMASK,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, XK_V),
		MODMASK,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, XK_Left),
		MODMASK | ShiftMask,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, XK_Right),
		MODMASK | ShiftMask,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, XK_Up),
		MODMASK | ShiftMask,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, XK_Down),
		MODMASK | ShiftMask,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);

	char* temp = malloc(100);
	memset(temp, 0, 100);
	sprintf(temp, "New Frame: %d", frame);
	log_msg(wm->log, temp);
	free(temp);
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
	XSetInputFocus(wm->display_, wm->focus, RevertToNone, CurrentTime);
}
Window get_lowest_subwindow(window_manager* wm, Window parent){
	Window tw, win;
	int t, n;
	XQueryPointer(wm->display_, parent, &tw, &win, &t, &t, &t, &t, &t);
	if(win == 0 || win == parent){
		return parent;
	} else return get_lowest_subwindow(wm, win);
}

Window get_pointer_window(window_manager* wm, bool recursive){
	if(pointer_initialized){
		Window tw, win;
		int t, n;
		do{
			XQueryPointer(wm->display_, wm->root_, &tw, &win, &t, &t, &t, &t, &t);
		} while(win<=0);
		Window* wins;
		XWindowAttributes watts;
		XQueryTree(wm->display_, win, &tw, &tw, &wins, &n);
		XGetWindowAttributes(wm->display_, wm->root_, &watts);
		t = 0;//watts.width * watts.height;
		int full_n = n;
		while(--n >= 0){
			XGetWindowAttributes(wm->display_, wins[n], &watts);
			if(watts.width * watts.height > t){
				win = wins[n];
				t = watts.width * watts.height;
			}
		}
		XFree(wins);
		return win;
	}
	return wm->root_;
}
Window get_pointer_subwindow(window_manager* wm){
	if(pointer_initialized){
		Window win;/*
		int x, y, n, t;
		do{
			XQueryPointer(wm->display_, wm->root_, &tw, &win, &x, &y, &x, &y, &t);
		} while(win<=0);*/
		win = get_lowest_subwindow(wm, wm->root_);
		return win;
	}
	return wm->root_;
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
			MODMASK | ShiftMask,
			wm->root_,
			FALSE,
			GrabModeAsync,
			GrabModeAsync
		);
	// Reset windows to last display
	XGrabKey(wm->display_,
			XKeysymToKeycode(wm->display_, XK_R),
			MODMASK,
			wm->root_,
			FALSE,
			GrabModeAsync,
			GrabModeAsync
		);
	XGrabKey(wm->display_,
			XKeysymToKeycode(wm->display_, XK_N),
			MODMASK,
			wm->root_,
			FALSE,
			GrabModeAsync,
			GrabModeAsync
		);

	log_msg(wm->log, "Entering main loop");
	/*XGrabPointer(wm->display_,
			wm->root_,
			FALSE,
			ButtonReleaseMask | ButtonPressMask | PointerMotionMask,
 			GrabModeAsync, GrabModeAsync,
			None, None, CurrentTime);
	*/// Main Event Loop
	for(;;){
		XEvent main_event;
		XNextEvent(wm->display_, &main_event);

		switch(main_event.type){
			case CreateNotify:{

				log_msg(wm->log, "Create notify");
				XCreateWindowEvent* e = &(main_event.xcreatewindow);
				char* temp = malloc(100);
				memset(temp, 0, 100);
				sprintf(temp, "New Window: %d", e->window);
				log_msg(wm->log, temp);
				free(temp);
			}
			break;
			case DestroyNotify:{
				XDestroyWindowEvent *e = &(main_event.xdestroywindow);
				remove_window(wm->workspace, e->window);
			}
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
				pointer_initialized = TRUE;
				XMapRequestEvent* e = &(main_event.xmaprequest);
				frame(wm, e->window, FALSE);
				XMapWindow(wm->display_, e->window);
				char* temp = malloc(100);
				memset(temp, 0, 100);
				sprintf(temp, "New Window: %d", e->window);
				log_msg(wm->log, temp);
				free(temp);
				XRaiseWindow(wm->display_, e->window);
				XSetInputFocus(wm->display_, e->window, RevertToNone, CurrentTime);
				XGrabButton(wm->display_,
					Button1,
					None,
					wm->root_,
					FALSE,
					ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
					GrabModeAsync,
					GrabModeAsync,
					None,
					None
				);
			}
			break;
			case KeyPress:{
				handle_key_press(wm, &(main_event.xkey));
			}
			break;
			case FocusOut:
			case FocusIn:{
				log_msg(wm->log, "Focus event");
				XFocusChangeEvent* e = &(main_event.xfocus);
				if(!e->send_event){
					XSendEvent(wm->display_, InputFocus, TRUE, None, (XEvent*)e);
				}
			}
			break;
			case MotionNotify: {
				XMotionEvent* e = &(main_event.xmotion);
				if(!e->send_event){
					Window focus = get_pointer_window(wm, FALSE);
					e->window = focus;
					XWindowAttributes watts;
					XGetWindowAttributes(wm->display_, wm->clients_[focus&4095], &watts);
					e->x -= watts.x + watts.border_width;
					e->y -= watts.y + watts.border_width;
					XSendEvent(wm->display_, focus, TRUE, None, (XEvent*)e);
				}
			}
			break;
			case LeaveNotify:
			case EnterNotify: {
				XCrossingEvent* e = &(main_event.xcrossing);
				char* temp = malloc(100);
				memset(temp, 0, 100);
				sprintf(temp, "enter/leave: %d, %d", wm->root_, e->window);
				log_msg(wm->log, temp);
				free(temp);
				if(!e->send_event){
					Window focus = get_pointer_window(wm, FALSE);
					XWindowAttributes watts;
					if(wm->clients_[focus&4095] != 0){
						XGetWindowAttributes(wm->display_, wm->clients_[focus&4095], &watts);
						e->x -= watts.x + watts.border_width;
						e->y -= watts.y + watts.border_width;
					}
					XSendEvent(wm->display_, InputFocus, TRUE, None, (XEvent*)e);
				}
			}
			break;

			case ButtonPress:
			case ButtonRelease:{
				handle_button_press(wm, &(main_event.xbutton));
			}
			break;
			case KeymapNotify:
				log_msg(wm->log, "Keymap notify");
				XKeymapEvent* e = &(main_event.xkeymap);
				if(!e->send_event){
					XSendEvent(wm->display_, InputFocus, TRUE, None, (XEvent*)e);
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
	if((e->state & (MODMASK | ShiftMask)) && (e->keycode == XKeysymToKeycode(wm->display_, XK_E))){
		close_window_manager(wm);
		log_msg(wm->log, "Exit hotkey pressed");
		exit(0);
	}
	if((e->state & MODMASK) && (e->keycode == XKeysymToKeycode(wm->display_, XK_J))){
		Window w = e->window;
		printf("Window %d accessed\n", w);
		XKillClient(wm->display_, e->window);
	}
	if((e->state & MODMASK) && (e->keycode == XKeysymToKeycode(wm->display_, XK_F))){
		Window w = e->window;
		XResizeWindow(wm->display_, w, wm->workspace->info.max_width, wm->workspace->info.max_height);
		XMoveWindow(wm->display_, wm->clients_[w&4095], 0, 0);
		XResizeWindow(wm->display_, wm->clients_[w&4095], wm->workspace->info.max_width, wm->workspace->info.max_height);
	}
	if((e->state & MODMASK) && (e->keycode == XKeysymToKeycode(wm->display_, XK_R))){
		tile_windows(wm);
	}
	if(((e->state & (MODMASK | ShiftMask)) && (e->keycode == XKeysymToKeycode(wm->display_, XK_Left)))
	   || ((e->state & (MODMASK | ShiftMask)) && (e->keycode == XKeysymToKeycode(wm->display_, XK_Right)))){
		move_horiz(wm->workspace, e->window);
		tile_windows(wm);
	}
	if((e->state & MODMASK) && (e->keycode == XKeysymToKeycode(wm->display_, XK_H))){
		expand_horiz(wm->workspace, e->window);
		tile_windows(wm);
	}
	if((e->state & MODMASK) && (e->keycode == XKeysymToKeycode(wm->display_, XK_G))){
		reset_expansion(wm->workspace, e->window);
		tile_windows(wm);
	}
	if((e->state & MODMASK) && (e->keycode == XKeysymToKeycode(wm->display_, XK_V))){
		expand_vert(wm->workspace, e->window);
		tile_windows(wm);
	}
	if((e->state & MODMASK) && (e->keycode == XKeysymToKeycode(wm->display_, XK_N))){
		if(fork() == 0){
			system(terminal);
			exit(0);
		}
	}
	if(((e->state & (MODMASK | ShiftMask)) && (e->keycode == XKeysymToKeycode(wm->display_, XK_Up)))
	   || ((e->state & (MODMASK | ShiftMask)) && (e->keycode == XKeysymToKeycode(wm->display_, XK_Down)))){
		move_vert(wm->workspace, e->window);
		tile_windows(wm);
	}
	XRaiseWindow(wm->display_, e->window);
	XSetInputFocus(wm->display_, e->window, RevertToNone, CurrentTime);


	if((e->state & Mod1Mask) && (e->keycode == XKeysymToKeycode(wm->display_, XK_Tab))){
		Window w = get_next(wm->workspace, e->window);
		XRaiseWindow(wm->display_, w);
		XSetInputFocus(wm->display_, w, RevertToNone, CurrentTime);
		char* temp = malloc(100);
		memset(temp, 0, 100);
		sprintf(temp, "Next Window %d", w);
		log_msg(wm->log, temp);
		free(temp);
	}

	XGrabButton(wm->display_,
		Button1,
		None,
		wm->root_,
		FALSE,
		ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
		GrabModeAsync,
		GrabModeAsync,
		None,
		None
	);
	log_msg(wm->log, "handling key press\n");
}


int handle_button_press(window_manager* wm, XButtonEvent* e){
	static Window last_focus = 0;
	Window focus = get_pointer_window(wm, FALSE);
	if(e->button == Button1 && e->type == ButtonPress){
		/*if(focus != last_focus){
			XRaiseWindow(wm->display_, focus);
			XSetInputFocus(wm->display_, focus, RevertToNone, CurrentTime);
		}*/
		XSetInputFocus(wm->display_, wm->root_, RevertToNone, CurrentTime);
		XUngrabButton(wm->display_,
			Button1,
			None,
			wm->root_
		);
	}
	if(!e->send_event){
		Window subwindow = get_pointer_subwindow(wm);
		e->window = focus;
		e->subwindow = 0;
		if(subwindow != focus){
			e->subwindow = subwindow;
		}
		XWindowAttributes watts;
		XGetWindowAttributes(wm->display_, wm->clients_[focus&4095], &watts);
		e->x -= watts.x + watts.border_width;
		e->y -= watts.y + watts.border_width;
		log_msg(wm->log, "sending event");
		XSendEvent(wm->display_, InputFocus, TRUE, None, (XEvent*)e);
	} else {
		log_msg(wm->log, "received sent event");
	}
	log_msg(wm->log, "handling button press");
	last_focus = focus;
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
			XResizeWindow(wm->display_, w, lo->width - BORDER*2, lo->height - BORDER*2);
			XResizeWindow(wm->display_, wm->clients_[w&4095], lo->width - BORDER*2, lo->height - BORDER*2);
			XMoveWindow(wm->display_, wm->clients_[w&4095], lo->x + BORDER, lo->y + BORDER);
		}
	}
}
