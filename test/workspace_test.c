#include "workspace.h"
#include <stdio.h>

int main(){
	ws_info inf = {1000, 500};
	ws_layout* ws = init_ws(inf);
	add_window(ws, 1);
	add_window(ws, 2);
	add_window(ws, 3);
	add_window(ws, 4);
	add_window(ws, 5);
	window_layout*  ls = ws->layouts;
	for(int i = 0; i < 5; i++){
		printf("%d (%d): %d, %d, %d, %d\n", i, ls[i].xid, ls[i].x, ls[i].y, ls[i].width, ls[i].height);
	}
}
