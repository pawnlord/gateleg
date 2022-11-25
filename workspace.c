#include "workspace.h"
#include <stdio.h>
#include <string.h>
#include "logger.h"

ws_layout* init_ws(ws_info info){
	ws_layout* ws = malloc(sizeof(ws_layout));
	ws->info = info;
	ws->window_count = 0;
	ws->layouts = malloc(50*sizeof(window_layout));
	ws->sz = 50;
	ws->root = init_root(1, info.max_width, info.max_height, 0, 0); // root represented by 1, but doesn't need to be
	return ws;
}
window_pos* add_window(ws_layout* ws, unsigned long int xid){

	int i = ws->window_count;
	for(int j = 0; j < ws->window_count; j++){
		if(ws->layouts[i].xid == xid){
			i = j;
			break;
		}
	}
	if(i == ws->window_count) {ws->window_count += 1;}
	if(ws->window_count == ws->sz){
		ws->sz *= 1.5;
		ws->layouts = realloc(ws->layouts, ws->sz*sizeof(window_layout));
	}
	ws->layouts[i].xid = xid;
	ws->layouts[i].node = qt_add_window(ws->root, xid);
	ws->layouts[i].is_moveable = 0; // assume we don't need to move it until requested otherwise
	return &(ws->layouts[i].node->pos);
}

void remove_window(ws_layout* ws, unsigned long int xid){
	int id = -1;
	for(int i = 0; i < ws->window_count; i++){
		if(ws->layouts[i].xid == xid){
			id = i;
			break;
		}
	}
	qt_remove_win(ws->root, xid);
	if(id != -1){
		for(int i = id; i < ws->window_count; i++){
			ws->layouts[i] = ws->layouts[i+1];
		}
		ws->window_count -= 1;
	}
}

char get_lock(ws_layout* ws, unsigned long int xid){
	window_layout* w;
	for(int i = 0; i < ws->window_count; i++){
		if(ws->layouts[i].xid == xid){
			w = ws->layouts+i;
			break;
		}
	}
	return w->lock;
}


void move_horiz(ws_layout* ws, unsigned long int xid, dir_t d){
	quadtree *node = qt_get_win(ws->root, xid);
	if(node != NULL){
		quadtree *new_node = find_branch(node, d);
		swap_branch(node, find_branch(node, d));
	}
}

void move_vert(ws_layout* ws, unsigned long int xid, dir_t d){
	move_horiz(ws, xid, d);
}
/*
void expand_horiz(ws_layout* ws, unsigned long int xid){
	window_layout* w;
	for(int i = 0; i < ws->window_count; i++){
		if(ws->layouts[i].xid == xid){
			w = ws->layouts+i;
			break;
		}
	}
	int is_up = w->quad>1;
	if(is_up) {
		w->quad = 2;
	} else {
		w->quad = 0;
	}
	w->width = ws->info.max_width;

	for(int i = 0; i < ws->window_count; i++){
		int temp_is_up = ws->layouts[i].quad > 1;
		if(temp_is_up == is_up && ws->layouts[i].xid != xid && ws->layouts[i].quad >= 0){
			if(is_up){
				ws->layouts[i].quad -= 2;
			} else{
				ws->layouts[i].quad += 2;
			}
		}
	}
	reset_positions(ws);
}

void expand_vert(ws_layout* ws, unsigned long int xid){
	window_layout* w;
	for(int i = 0; i < ws->window_count; i++){
		if(ws->layouts[i].xid == xid){
			w = ws->layouts+i;
			break;
		}
	}
	int is_right = w->quad%2;
	w->quad = is_right;

	w->height = ws->info.max_height;

	for(int i = 0; i < ws->window_count; i++){
		int temp_is_right = ws->layouts[i].quad%2;
		if(temp_is_right == is_right && ws->layouts[i].xid != xid && ws->layouts[i].xid && ws->layouts[i].quad != 0){
			if(is_right){
				ws->layouts[i].quad -= 1;
			} else{
				ws->layouts[i].quad += 1;
			}
		}
	}
	reset_positions(ws);
}

 // Unimplemented because they do not make sense in a quad tree. To replace this functionality, this is done automatically if we have space.

void reset_expansion(ws_layout* ws, unsigned long int xid){
	window_layout* w;
	for(int i = 0; i < ws->window_count; i++){
		if(ws->layouts[i].xid == xid){
			w = ws->layouts+i;
			break;
		}
	}
	w->height = ws->info.max_height/2;
	w->width = ws->info.max_width/2;
}
*/
void remove_win_from_struct(ws_layout *ws, unsigned long int xid){
	window_layout* win;
	int i;
	for(i = 0; i < ws->window_count; i++){
		if(ws->layouts[i].xid == xid){
			win = ws->layouts+i;
			break;
		}
	}
	if(win != NULL){
		qt_remove_win(ws->root, xid); // remove window from our normal structure
		win->node = malloc(sizeof(quadtree));
		win->node->w=-1;
		window_pos *w = &(win->node->pos);
	}
}
void toggle_moveability(ws_layout *ws, unsigned long int xid){
	for(int i = 0; i < ws->window_count; i++){
		if(ws->layouts[i].xid == xid){
			ws->layouts[i].is_moveable = !ws->layouts[i].is_moveable;
			break;
		}
	}
}
void move_resize_lo(ws_layout* ws, unsigned long int xid, int x, int y, int width, int height){
	window_layout* win = NULL;
	for(int i = 0; i < ws->window_count; i++){
		if(ws->layouts[i].xid == xid){
			win = ws->layouts+i;
			break;
		}
	}
	if(win != NULL){
		if(win->is_moveable){
			window_pos *w = &(win->node->pos);
			w->x = x;
			w->y = y;
			w->w = width;
			w->h = height;
		}
	} else{
		stat_log_msg("No Window Found");
	}
}
window_pos *get_position(ws_layout *ws, unsigned long int xid){
	for(int i = 0; i < ws->window_count; i++){
		if(ws->layouts[i].xid == xid){
			return &(ws->layouts[i].node->pos);
		}
	}
}

unsigned long int get_next(ws_layout* ws, unsigned long int xid){
	if(ws->window_count == 0)
		return 0;
	for(int i = 0; i < ws->window_count-1; i++){
		if(ws->layouts[i].xid == xid){
			for(int j=i+1; j < ws->window_count; j++){
				if(ws->layouts[j].node->w != -1)
					return ws->layouts[j].xid;
			}
			break;
		}
	}
	return ws->layouts[0].xid;
} // TODO: this does not work

/*
void reset_positions(ws_layout* ws){
	for(int i = 0; i < ws->window_count; i++){
		window_layout* w = ws->layouts+i;
		int is_right = w->quad%2;
		int is_down = w->quad>1;
		ws->layouts[i].x = is_right * (ws->info.max_width/2);
		ws->layouts[i].y = is_down * (ws->info.max_height/2);
	}
}
*/
