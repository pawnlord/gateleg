#include "workspace.h"

ws_layout* init_ws(ws_info info){
	ws_layout* ws = malloc(sizeof(ws_layout));
	ws->info = info;
	ws->window_count = 0;
	ws->layouts = malloc(50*sizeof(window_layout));
	return ws;
}
window_layout* add_window(ws_layout* ws, int xid){
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


