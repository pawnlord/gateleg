#include "wmap.h"

int hashw(Window w){
	return w&4095;
}

window_map* init_wmap(){
	window_map* wm = malloc(sizeof(window_map));
	wm->buckets = malloc(sizeof(window_bucket)*4096);
	for(int i = 0; i < 4096; i++){
		wm->buckets[i].keys = 0;
		wm->buckets[i].vals = 0;
		wm->buckets[i].n = 0;
	}
	return wm;
}

void wmap_set(window_map* wm, Window key, Window val){
	int idx = hashw(key);
	if(wm->buckets[idx].n == 0){
		wm->buckets[idx].n = 1;
		wm->buckets[idx].keys = malloc(sizeof(Window));
		wm->buckets[idx].keys[0] = key;

		wm->buckets[idx].vals = malloc(sizeof(Window));
		wm->buckets[idx].vals[0] = val;
	} else{
		for(int i = 0; i < wm->buckets[idx].n; i++){
			if(wm->buckets[idx].keys[i] == key){
				wm->buckets[idx].vals[i] = val;
				return;
			}
		}
		wm->buckets[idx].n += 1;
		int n = wm->buckets[idx].n;
		wm->buckets[idx].keys = realloc(wm->buckets[idx].keys, sizeof(Window)*n);
		wm->buckets[idx].vals = realloc(wm->buckets[idx].vals, sizeof(Window)*n);
		wm->buckets[idx].keys[n-1] = key;
		wm->buckets[idx].vals[n-1] = val;
	}
}

// Returns -1 on failure
Window wmap_get(window_map* wm, Window key){
	int idx = hashw(key);
	if(wm->buckets[idx].n == 0){
		return -1;
	} else{
		for(int i = 0; i < wm->buckets[idx].n; i++){
			if(wm->buckets[idx].keys[i] == key){
				return wm->buckets[idx].vals[i];
			}
		}
	}
	return -1;
}

