#include "window_manager.h"


int main(){
	window_manager wm;
	if(create_window_manager(&wm)){
		printf("failed to create window manager\n");
		return 1;
	}
	run_wm(&wm);
	return 0;

}