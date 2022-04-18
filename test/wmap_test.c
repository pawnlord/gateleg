#include <wmap.h>

int main(){
	window_map* wmap = init_wmap();
	printf("%d\n", wmap_get(wmap, 0));
	wmap_set(wmap, 0, 420);
	printf("%d\n", wmap_get(wmap, 0));
	wmap_set(wmap, 4095, 42);
	printf("%d\n", wmap_get(wmap, 4095));
}
