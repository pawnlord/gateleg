#ifndef WMAP
#define WMAP
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>

typedef struct {
	unsigned char n; // number of key-val pair
	Window* keys;
	Window* vals;
} window_bucket;

typedef struct {
	window_bucket* buckets;
} window_map;

int hashw(Window w);

window_map* init_wmap();

void wmap_set(window_map* wm, Window key, Window val);

// Returns 0 on failure
Window wmap_get(window_map* wm, Window key);

#endif
