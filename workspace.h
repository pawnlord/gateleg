#include <stdlib.h>
#define HORIZONTAL 1
#define VERTICAL 2

// bounds, res, specifics
typedef struct {
	int max_width;
	int max_height;
} ws_info;


typedef struct {
	unsigned long int xid;
	int quad;
	int expand_flag;
	int x, y, width, height;
} window_layout;

// full layout manager
typedef struct {
	ws_info info;
	window_layout* layouts;
	int window_count;
} ws_layout;

ws_layout* init_ws(ws_info info);
window_layout* add_window(ws_layout* ws, unsigned long int xid); // WARNING, will likely change window_layout* pointer
void move_horiz(ws_layout* ws, unsigned long int xid);
void move_vert(ws_layout* ws, unsigned long int xid);
void expand_horiz(ws_layout* ws, unsigned long int xid);
void expand_vert(ws_layout* ws, unsigned long int xid);
void reset_expansion(ws_layout* ws, unsigned long int xid);
unsigned long int get_next(ws_layout* ws, unsigned long int xid);

void reset_positions(ws_layout* ws);
