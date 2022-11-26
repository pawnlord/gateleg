#include "window_manager.h"
#include <X11/Xutil.h>
#include <stdint.h>
#define BORDER 40
#define MODMASK Mod4Mask
FILE* main_log;

bool wm_detected_;
bool pointer_initialized = FALSE;
void tile_windows(window_manager* wm);
void switch_spaces(window_manager* wm, int src_wsnum, int dst_wsnum);
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
	wm->clients_ = init_wmap();
	wm->log = open_log("log.log");
	ws_info inf;
	int snum = XDefaultScreen(wm->display_);
	inf.max_width = XDisplayWidth(wm->display_, snum);
	inf.max_height = XDisplayHeight(wm->display_, snum);
	wm->workspace = malloc(sizeof(ws_layout*)*10);
	memset(wm->workspace, 0, 10*sizeof(ws_layout*));
	wm->workspace[0] = init_ws(inf);
	wm->wsnum = 0;
	wm->focus = wm->root_;
	wm->is_ws_switch = 0;
	main_log = wm->log;
}


void destroy_window_manager(window_manager* wm){
	XCloseDisplay(wm->display_);
}

void grab_key(window_manager *wm, int key, int mask, Window w){
	XGrabKey(wm->display_,
		XKeysymToKeycode(wm->display_, key),
		mask,
		w,
		FALSE,
		GrabModeAsync,
		GrabModeAsync
	);
}
void frame(window_manager* wm, Window w, bool is_before_wm_created);
void unframe(window_manager* wm, Window w);

void frame(window_manager* wm, Window w, bool is_before_wm_created){
	static unsigned int BORDER_WIDTH = 0;
	static unsigned long BORDER_COLOR = 0x000000;
	static unsigned long BG_COLOR = 0x000000;
	XWindowAttributes x_window_attrs;
	XGetWindowAttributes(wm->display_, w, &x_window_attrs);
	window_pos* l = add_window(wm->workspace[wm->wsnum], w);
	if(x_window_attrs.override_redirect == True){
		log_msg(wm->log, "override called");
		return;
	}

	if(is_before_wm_created){
		if(x_window_attrs.override_redirect || x_window_attrs.map_state != IsViewable){
			return;
		}
	}
	if(wmap_get(wm->clients_, w) != -1){
		unframe(wm, w);
	}
	Window frame = XCreateSimpleWindow(
			wm->display_,
			wm->root_,
			l->x + BORDER,
			l->y + BORDER,
			l->w - BORDER*2,
			l->h - BORDER*2,
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

	XMapRaised(wm->display_, frame);

	wmap_set(wm->clients_, w, frame);

	grab_key(wm, XK_J, MODMASK, w);
	grab_key(wm, XK_Tab, Mod1Mask, w);
	grab_key(wm, XK_F, MODMASK, w);
	grab_key(wm, XK_G, MODMASK, w);
	grab_key(wm, XK_V, MODMASK, w);
	grab_key(wm, XK_H, MODMASK, w);
	grab_key(wm, XK_M, MODMASK, w);
	grab_key(wm, XK_Left, MODMASK | ShiftMask, w);
	grab_key(wm, XK_Right, MODMASK | ShiftMask, w);
	grab_key(wm, XK_Up, MODMASK | ShiftMask, w);
	grab_key(wm, XK_Down, MODMASK | ShiftMask, w);

	// Grab all workspace hotkeys
	for(int i = 10; i < 20; i++){
		XGrabKey(wm->display_,
				i,
				MODMASK | ShiftMask,
				w,
				FALSE,
				GrabModeAsync,
				GrabModeAsync
			);
	}

	char* temp = malloc(100);
	memset(temp, 0, 100);
	sprintf(temp, "New Frame: %d", frame);
	log_msg(wm->log, temp);
	free(temp);
	wm->focus = w;
}
void unframe(window_manager* wm, Window w){
	Window frame = wmap_get(wm->clients_, w);
	XUnmapWindow(wm->display_, frame);
	XReparentWindow(wm->display_, w, wm->root_, 0, 0);
	XRemoveFromSaveSet(wm->display_, w);
	XDestroyWindow(wm->display_, frame);
	wmap_set(wm->clients_, w, -1);
	wm->focus = wm->root_;
	XSetInputFocus(wm->display_, wm->focus, RevertToNone, CurrentTime);
	tile_windows(wm);
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
	grab_key(wm, XK_E, MODMASK | ShiftMask, wm->root_);
	// Reset windows to last display
	grab_key(wm, XK_R, MODMASK, wm->root_);
	grab_key(wm, XK_N, MODMASK, wm->root_);
	// Grab all workspace hotkeys
	for(int i = 10; i < 20; i++){
		XGrabKey(wm->display_,
				i,
				MODMASK,
				wm->root_,
				FALSE,
				GrabModeAsync,
				GrabModeAsync
			);
	}
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

				XCreateWindowEvent* e = &(main_event.xcreatewindow);
			}
			break;
			case DestroyNotify:{
				XDestroyWindowEvent *e = &(main_event.xdestroywindow);
				remove_window(wm->workspace[wm->wsnum], e->window);
				if(wmap_get(wm->clients_, e->window) != -1){
					unframe(wm, e->window);
				}
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

				Window frame = wmap_get(wm->clients_, e->window);
				XWindowChanges window_changes = frame_changes;
				if(frame != -1){
					window_changes.x = 0;
					window_changes.y = 0;
				}
				char* temp = malloc(100);
				memset(temp, 0, 100);
				sprintf(temp, "Configure Request: w: %d (f: %d) x: %d y: %d w: %d h: %d", e->window, frame, e->x, e->y, e->width, e->height);
				log_msg(wm->log, temp);
				free(temp);

				if(frame != -1){
					stat_log_msg("asd");
					XConfigureWindow(wm->display_, frame, e->value_mask, &frame_changes);
					XResizeWindow(wm->display_, frame, e->width+BORDER*2, e->height+BORDER*2);
					move_resize_lo(wm->workspace[wm->wsnum], e->window, e->x-BORDER, e->y-BORDER, e->width+BORDER*2, e->height+BORDER*2);
				}
				XConfigureWindow(wm->display_, e->window, e->value_mask, &window_changes);
				tile_windows(wm);
			}
			break;
			case UnmapNotify:{
				XUnmapEvent *e = &(main_event.xunmap);
				if(wmap_get(wm->clients_, e->window) == -1 || e->event == wm->root_){
					break;
				}
				unframe(wm, e->window);
				remove_win_from_struct(wm->workspace[wm->wsnum], e->window);
				tile_windows(wm);
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
				tile_windows(wm);
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
					XGetWindowAttributes(wm->display_, wmap_get(wm->clients_, focus), &watts);
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
					Window frame = wmap_get(wm->clients_, focus);
					if(frame != -1){
						XGetWindowAttributes(wm->display_, frame, &watts);
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
	char* temp = malloc(200);
	memset(temp, 0, 200);
	XGetErrorText(display, e->error_code, temp, 99);
	if(e->resourceid != 0){
		sprintf(temp, "%s (Window %d)", temp, e->resourceid);
	}
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
/*		Window w = e->window;
		printf("Window %d accessed\n", w);
		XKillClient(wm->display_, e->window);
*/
		//hide window
		Window w = e->window;
		XIconifyWindow(wm->display_, wm->root_, DefaultScreen(wm->display_));
	}
	if((e->state & MODMASK) && (e->keycode >= 10 && e->keycode <= 19)){
		int n = e->keycode - 10;
		if(wm->workspace[n] == 0){
			wm->workspace[n] = init_ws(wm->workspace[wm->wsnum]->info);
		}

		if(e->state & (ShiftMask)){
			remove_window(wm->workspace[wm->wsnum], e->window);
			add_window(wm->workspace[n], e->window);
		}

		switch_spaces(wm, wm->wsnum, n);

		wm->wsnum = n;
		tile_windows(wm);

		char* temp = malloc(100); memset(temp, 0, 100);
		sprintf(temp, "workspace number %d", n);
		log_msg(wm->log, temp); free(temp);
	}
	if((e->state & MODMASK) && (e->keycode == XKeysymToKeycode(wm->display_, XK_F))){
		Window w = e->window;
		Window frame = wmap_get(wm->clients_, w);
		XResizeWindow(wm->display_, w, wm->workspace[wm->wsnum]->info.max_width, wm->workspace[wm->wsnum]->info.max_height);
		XMoveWindow(wm->display_, frame, 0, 0);
		XResizeWindow(wm->display_, frame, wm->workspace[wm->wsnum]->info.max_width, wm->workspace[wm->wsnum]->info.max_height);
	}
	if((e->state & MODMASK) && (e->keycode == XKeysymToKeycode(wm->display_, XK_R))){
		tile_windows(wm);
	}

	if((e->state & MODMASK) && (e->keycode == XKeysymToKeycode(wm->display_, XK_M))){
		stat_log_msg("XK_M Pressed");
		Window w = e->window;
		toggle_moveability(wm->workspace[wm->wsnum], w);
	}
	if(((e->state & (MODMASK | ShiftMask)) && (e->keycode == XKeysymToKeycode(wm->display_, XK_Left)))
	   || ((e->state & (MODMASK | ShiftMask)) && (e->keycode == XKeysymToKeycode(wm->display_, XK_Right)))){
		dir_t d = {1,0};
		d.x *= (e->keycode == XKeysymToKeycode(wm->display_, XK_Right))?1:-1;
		stat_log_msg("Move horizontal");
		move_horiz(wm->workspace[wm->wsnum], e->window, d);
		tile_windows(wm);
	}
	// --- WARNING ---
	// expansion is currently not meant to do anything
	/*
	if((e->state & MODMASK) && (e->keycode == XKeysymToKeycode(wm->display_, XK_H))){
		// expand_horiz(wm->workspace[wm->wsnum], e->window);
		tile_windows(wm);
	}
	if((e->state & MODMASK) && (e->keycode == XKeysymToKeycode(wm->display_, XK_G))){
		// reset_expansion(wm->workspace[wm->wsnum], e->window);
		tile_windows(wm);
	}
	if((e->state & MODMASK) && (e->keycode == XKeysymToKeycode(wm->display_, XK_V))){
		// expand_vert(wm->workspace[wm->wsnum], e->window);
		tile_windows(wm);
	}
	*/
	if((e->state & MODMASK) && (e->keycode == XKeysymToKeycode(wm->display_, XK_N))){
		if(fork() == 0){
			system(terminal);
			exit(0);
		}
	}
	if(((e->state & (MODMASK | ShiftMask)) && (e->keycode == XKeysymToKeycode(wm->display_, XK_Up)))
	   || ((e->state & (MODMASK | ShiftMask)) && (e->keycode == XKeysymToKeycode(wm->display_, XK_Down)))){
		dir_t d = {0,1};
		d.y *= (e->keycode == XKeysymToKeycode(wm->display_, XK_Down))?1:-1;
		move_vert(wm->workspace[wm->wsnum], e->window, d);
		tile_windows(wm);
	}
	XRaiseWindow(wm->display_, e->window);
	XSetInputFocus(wm->display_, e->window, RevertToNone, CurrentTime);


	if((e->state & Mod1Mask) && (e->keycode == XKeysymToKeycode(wm->display_, XK_Tab))){
		Window w = get_next(wm->workspace[wm->wsnum], e->window);
		if(w != 0){
			XRaiseWindow(wm->display_, w);
			XSetInputFocus(wm->display_, w, RevertToNone, CurrentTime);
			char* temp = malloc(100);
			memset(temp, 0, 100);
			sprintf(temp, "Next Window %d", w);
			log_msg(wm->log, temp);
			free(temp);
		}
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
		XGetWindowAttributes(wm->display_, wmap_get(wm->clients_, focus), &watts);
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

get_property_t *get_property(Display *display_, Window w, const char *prop_name, int sz){
	get_property_t *prop = malloc(sizeof(get_property_t));
	prop->prop_return = malloc(sz*4);
	XGetWindowProperty(display_, w, XInternAtom(display_, prop_name, True),
				0, sz, False, AnyPropertyType,
				&prop->ret_type, &prop->actual_fmt, &prop->nir, &prop->bar, &prop->prop_return
			);
	return prop;
}


typedef struct{
	uint32_t l,r,t,b;
}strut_t;
typedef struct{
	strut_t st;
	uint32_t lsy,ley,rsy,rey,tsx,tex,bsx,bex;
}strut_partial_t;
int has_strut_info(char *prop_rtn){
	for(int i = 0; i < 12; i++){
		if(prop_rtn[i] != 0){
			return 1;
		}
	}
	return 0;
}

void resize_workspace(ws_layout *workspace, strut_t *st){
	workspace->info.max_width -= st->r;
	workspace->info.max_height -= st->b;
	workspace->x += st->l;
	workspace->y += st->t;
	reset_root(workspace->root, workspace->info.max_width, workspace->info.max_height, workspace->x, workspace->y);
}

int check_layout_properties(window_manager *wm, ws_layout *workspace, Window w, window_layout *lo){
	window_pos *pos = &(lo->node->pos);
	strut_t* st;
	get_property_t *prop  = get_property(wm->display_, w, "_NET_WM_STRUT_PARTIAL", 12);
	if(prop->ret_type == None){
		prop = get_property(wm->display_, w, "_NET_WM_STRUT", 4);
	}
	if(prop->ret_type != None){
		stat_log_msg("dasdas");
		st = (strut_t*) prop->prop_return;

		char *temp = malloc(1000);memset(temp,0,1000);
		sprintf(temp, "current position %d: %d,%d %dx%d",w,pos->x,pos->y,pos->w,pos->h);
		log_msg(wm->log, temp); free(temp);

		remove_win_from_struct(workspace, w);
		move_resize_lo(wm->workspace[wm->wsnum], w, pos->x, pos->y, pos->w, pos->h);

		resize_workspace(workspace, st);
		lo->is_moveable = 1;
	}
	free(prop->prop_return);
	free(prop);
	return prop->ret_type != None;
}

void tile_windows(window_manager* wm){
	ws_layout* workspace = wm->workspace[wm->wsnum];
	log_msg(wm->log, "tiling");
	for(int i = 0; i < workspace->window_count; i++){
		Window w = workspace->layouts[i].xid;

		window_layout *lo_full = workspace->layouts+i;
		window_pos *lo = &(lo_full->node->pos);

		// Get if window currently has set it's position with ewmh
		check_layout_properties(wm, workspace, w, lo_full);

		lo_full->lock = 1;

		Window frame = wmap_get(wm->clients_, w);
		if(frame != -1){
			char* temp = malloc(100);
			memset(temp, 0, 100);
			sprintf(temp, "Resizing w: %d x: %d y: %d w: %d h: %d", w, lo->x, lo->y, lo->w, lo->h);
			log_msg(wm->log, temp);
			free(temp);
			XResizeWindow(wm->display_, w, lo->w - BORDER*2, lo->h - BORDER*2);
			XResizeWindow(wm->display_, frame, lo->w - BORDER*2, lo->h - BORDER*2);
			XMoveWindow(wm->display_, frame, lo->x + BORDER, lo->y + BORDER);
		}
		lo_full->lock = 0;
	}
}

void switch_spaces(window_manager* wm, int src_wsnum, int dst_wsnum){
	ws_layout* src = wm->workspace[src_wsnum];
	ws_layout* dst = wm->workspace[dst_wsnum];
	wm->is_ws_switch = 1;
	for(int i = 0; i < src->window_count; i++){
		Window w = src->layouts[i].xid;
		Window frame = wmap_get(wm->clients_, w);
		XUnmapWindow(wm->display_, frame);
	}
	for(int i = 0; i < dst->window_count; i++){
		Window w = dst->layouts[i].xid;
		Window frame = wmap_get(wm->clients_, w);
		XMapWindow(wm->display_, frame);
	}
	wm->is_ws_switch = 0;
}
