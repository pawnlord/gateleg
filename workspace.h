#include <stdlib.h>

// bounds, res, specifics
typedef struct {
	int max_width;
	int max_height;
} ws_info;


typedef struct {
	int xid;
	int quad;
	int x, y, width, height;
} window_layout;

// full layout manager
typedef struct {
	ws_info info;
	window_layout* layouts;
	int window_count;
} ws_layout;

ws_layout* init_ws(ws_info info);
window_layout* add_window(ws_layout* ws, int xid); // WARNING, will likely change window_layout* pointer

