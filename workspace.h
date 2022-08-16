#include <stdlib.h>
#include "quadtree.h"
#define HORIZONTAL 1
#define VERTICAL 2

// bounds, res, specifics
typedef struct {
	int max_width;
	int max_height;
} ws_info;


typedef struct {
	unsigned long int xid;
	int expand_flag;
	quadtree *node;
	char lock;
	char is_moveable;
} window_layout;

// full layout manager
typedef struct {
	ws_info info;
	window_layout* layouts;
	quadtree *root;
	int window_count;
	int sz;
	int x, y;
} ws_layout;

ws_layout* init_ws(ws_info info);
window_pos* add_window(ws_layout* ws, unsigned long int xid); // WARNING, will likely change window_layout* pointer
void remove_window(ws_layout* ws, unsigned long int xid);
char get_lock(ws_layout* ws, unsigned long int xid);
void move_horiz(ws_layout* ws, unsigned long int xid, dir_t dir);
void move_vert(ws_layout* ws, unsigned long int xid, dir_t dir);
void expand_horiz(ws_layout* ws, unsigned long int xid);
void expand_vert(ws_layout* ws, unsigned long int xid);
void reset_expansion(ws_layout* ws, unsigned long int xid);
void remove_win_from_struct(ws_layout *ws, unsigned long int xid);
void move_resize_lo(ws_layout* ws, unsigned long int xid, int x, int y, int width, int height);
unsigned long int get_next(ws_layout* ws, unsigned long int xid);
window_pos *get_position(ws_layout *ws, unsigned long int xid);
void toggle_moveability(ws_layout *ws, unsigned long int xid);

void reset_positions(ws_layout* ws);
