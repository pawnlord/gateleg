#include "quadtree.h"

int main(){
	quadtree *root = init_root(1);
	printf("1: %d\n", root);
	for(int i = 0; i < 6; i++){
		printf("%d\n", i+2);
		qt_add_window(root, i+2);
	}
	qt_add_window(root->chldrn[0], 8);
	dir_t t = {1,0};
	swap_branch(root->chldrn[0]->chldrn[1], find_branch(root->chldrn[0]->chldrn[1], t));
	print_tree(root);
	quadtree *w = qt_get_win(root, 6);
	print_tree(w);
	qt_remove_win(root, 6);
	print_tree(root);
	qt_add_window(root, 9);
	print_tree(root);
}
