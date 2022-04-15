#include "workspace.h"
#include <stdio.h>

ws_layout* init_ws(ws_info info){
	ws_layout* ws = malloc(sizeof(ws_layout));
	ws->info = info;
	ws->window_count = 0;
	ws->layouts = malloc(50*sizeof(window_layout));
	return ws;
}
window_layout* add_window(ws_layout* ws, unsigned long int xid){
	int i = ws->window_count;
	int quad = i%4;
	int is_right = quad%2;
	int is_down = quad>1;
	ws->layouts[i].xid = xid;
	ws->layouts[i].quad = i%4;
	ws->layouts[i].x = is_right * (ws->info.max_width/2);
	ws->layouts[i].y = is_down * (ws->info.max_height/2);
	ws->layouts[i].width = ws->info.max_width/2;
	ws->layouts[i].height = ws->info.max_height/2;
	ws->window_count += 1;
	return (ws->layouts)+i;
}

void move_horiz(ws_layout* ws, unsigned long int xid){
	window_layout* w;
	for(int i = 0; i < ws->window_count; i++){
		if(ws->layouts[i].xid == xid){
			w = ws->layouts+i;
			break;
		}
	}
	int starting_quad = w->quad;
	int is_right = w->quad%2;


	if(is_right){
		w->quad -= 1;
	} else {
		w->quad += 1;
	}

	w->width = ws->info.max_width/2;

	for(int i = 0; i < ws->window_count; i++){
		int temp_quad = ws->layouts[i].quad;
		if(temp_quad == w->quad && ws->layouts[i].xid != xid){
			ws->layouts[i].quad = starting_quad;
		}
	}
	reset_positions(ws);
}

void move_vert(ws_layout* ws, unsigned long int xid){
	window_layout* w;
	for(int i = 0; i < ws->window_count; i++){
		if(ws->layouts[i].xid == xid){
			w = ws->layouts+i;
			break;
		}
	}
	int starting_quad = w->quad;
	int is_up = w->quad>1;


	if(is_up){
		w->quad -= 2;
	} else{
		w->quad += 2;
	}

	w->height = ws->info.max_height/2;


	for(int i = 0; i < ws->window_count; i++){
		int temp_quad = ws->layouts[i].quad;
		if(temp_quad == w->quad && ws->layouts[i].xid != xid){
			ws->layouts[i].quad = starting_quad;
		}
	}
	reset_positions(ws);
}

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
		if(temp_is_up == is_up && ws->layouts[i].xid != xid){
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
		int temp_is_right = ws->layouts[i].quad > 1;
		if(temp_is_right == is_right && ws->layouts[i].xid != xid){
			if(is_right){
				ws->layouts[i].quad -= 1;
			} else{
				ws->layouts[i].quad += 1;
			}
		}
	}
	reset_positions(ws);
}

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

unsigned long int get_next(ws_layout* ws, unsigned long int xid){
	window_layout* w;
	int i;
	for(i = 0; i < ws->window_count; i++){
		if(ws->layouts[i].xid == xid){
			w = ws->layouts+i;
			break;
		}
	}
	// get next to the left
	int quad = w->quad + 1; // quadrant we're aiming for
	int quads[4] = {-1};
	int start = i;
	// go backwards in the list, looping
	i = (i-1+ws->window_count)%ws->window_count;
	for(;i != start; i = (i-1+ws->window_count)%ws->window_count){
		printf("%d\n", i);
		window_layout* temp_w = ws->layouts+i;
		int temp_q = (temp_w->quad - quad + 4) % 4;
		quads[temp_q] = i;
	}

	for(int i = 0; i < 4; i++){
		if(quads[i] != -1){
			return ws->layouts[quads[i]].xid;
		}
	}
	return 0;
}

void reset_positions(ws_layout* ws){
	for(int i = 0; i < ws->window_count; i++){
		window_layout* w = ws->layouts+i;
		int is_right = w->quad%2;
		int is_down = w->quad>1;
		ws->layouts[i].x = is_right * (ws->info.max_width/2);
		ws->layouts[i].y = is_down * (ws->info.max_height/2);
	}
}
