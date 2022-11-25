#include "quadtree.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "logger.h"

bool qt_isfull(quadtree *tree){
	for(int i = 0; i < 4; i++){
		if(tree->chldrn[i] == NULL){ return 0; }
	}
	return 1;
}
bool qt_isempty(quadtree *tree){
	for(int i = 0; i < 4; i++){
		if(tree->chldrn[i] != NULL){ return 0; }
	}
	return 1;
}
void set_coords(quadtree *tree, int x, int y){
	tree->x = x;
	tree->y = y;
}
int count_children(quadtree *tree){
	int num = 0;
	for(int i = 0; i < 4; i++){
		if(tree->chldrn[i] != NULL){ 
			num++;
			num += count_children(tree->chldrn[i]);
		}
	}
	return num;
}
int count_direct_chldrn(quadtree *tree){
	int num = 0;
	for(int i = 0; i < 4; i++){
		if(tree->chldrn[i] != NULL){ 
			num++;
		}
	}
	return num;
}

quadtree *get_spacial_tree(quadtree *tree, dir_t d, quadtree *origin){
	quadtree *new_tree = tree->chldrn[d.x+d.y*2];
	if(new_tree == NULL || new_tree == origin){
		new_tree = tree->chldrn[d.x+(!d.y)*2];
	}
	if(new_tree == NULL || new_tree == origin){
		new_tree = tree->chldrn[(!d.x)+(!d.y)*2];
	}
	if(new_tree == NULL || new_tree == origin){
		new_tree = tree->chldrn[(!d.x)+d.y*2];
	}
	return new_tree;
}

void augment_down(quadtree *node){
	if(node->prnt != NULL){
		node->pos = node->prnt->pos;
		if(node->prnt->chldrn[node->y*2 + (!node->x)] != NULL || node->prnt->chldrn[(!node->y)*2 + (!node->x)] != NULL){
			node->pos.w /= 2;
			node->pos.x += node->pos.w * node->x;
		}
		if(node->prnt->chldrn[(!node->y)*2 + node->x] != NULL){
			node->pos.h /= 2;
			node->pos.y += node->pos.h * node->y;
		}
	}
	for(int i = 0; i < 4; i++){
		if(node->chldrn[i] != NULL){
			augment_down(node->chldrn[i]);
		}
	}
}

void set_branch(quadtree *prnt, quadtree *brnch, int x, int y){
	set_coords(brnch, x, y);
	if(brnch != NULL && prnt != NULL){
		brnch->prnt = prnt;
		prnt->chldrn[x+y*2] = brnch;
		augment_down(prnt);
	}
}


quadtree *init_root(int root, int w, int h, int headx, int heady){
	quadtree *new_tree = (quadtree*)malloc(sizeof(quadtree));
	memset(new_tree, 0, sizeof(quadtree)); // intitialize all branches to NULL
	new_tree->w = root;
	new_tree->pos = (window_pos){headx,heady,w,h,root};
	return new_tree;
}
void reset_root(quadtree *root, int w, int h, int headx, int heady){
	root->pos.x = headx;
	root->pos.y = heady;
	root->pos.w = w;
	root->pos.h = h;
	augment_down(root);
}
void add_branch(quadtree *prnt, quadtree *branch){
	remove_branch(branch);
	if(qt_isempty(prnt) && prnt->prnt != NULL){
		quadtree *new_branch = (quadtree*)malloc(sizeof(quadtree));
		memset(new_branch, 0, sizeof(quadtree));
		set_branch(prnt->prnt, new_branch, prnt->x, prnt->y);
		set_branch(new_branch, prnt, 0, 0);
		set_branch(new_branch, branch, 1, 0); // kind of hard-code-y, not prefered
		return;
	}
	for(int i = 0; i < 4; i++){
		if(prnt->chldrn[i] == NULL){
			set_branch(prnt, branch, i%2, i/2);
			return;
		}
	}
	// We have no room, pass it on to the child with the least amount of children.
	int min_chld = 255; // TODO: get actual number that is useful
	quadtree *best = prnt->chldrn[0];
	for(int i = 0; i < 4; i++){
		int chld_num = count_children(prnt->chldrn[i]);
		if(chld_num < min_chld){
			min_chld = chld_num;
			best = prnt->chldrn[i];
		}
	}
	add_branch(best, branch);
}
quadtree *qt_add_window(quadtree *prnt, int w){
	quadtree *new_branch = malloc(sizeof(quadtree));
	memset(new_branch, 0, sizeof(quadtree)); // intitialize all branches to NULL
	new_branch->w = w;
	add_branch(prnt, new_branch);
	return new_branch;
}
void swap_branch(quadtree *arg1, quadtree *arg2){
	if(arg1 != NULL && arg2 != NULL){
		quadtree *prnt1 = arg1->prnt, *prnt2 = arg2->prnt;
		int x1 = arg1->x, y1 = arg1->y, x2 = arg2->x, y2 = arg2->y;
		if(prnt1 != NULL && prnt2 != NULL){
			set_branch(prnt1, arg2, x1, y1);
			set_branch(prnt2, arg1, x2, y2);
		}
	} else {
		stat_log_msg("Null found in swap branch");
	}
}

typedef struct{
	dir_t init_d, cnst_coord; // cnst_coord must be true of the found replacement
	int depth; // Matches size of descent order
	dir_t *dscnt_order;
	int dscnt_size;
} find_info;

void add_dscnt_point(find_info *inf, int x, int y){
	if(inf->dscnt_order == NULL){
		inf->dscnt_size = 5;
		inf->dscnt_order = (dir_t*)malloc(sizeof(dir_t)*inf->dscnt_size);
	}
	else if(inf->depth >= inf->dscnt_size){
		inf->dscnt_size*=2;
		inf->dscnt_order = realloc(inf->dscnt_order, sizeof(dir_t)*inf->dscnt_size);
	}
	inf->dscnt_order[inf->depth].x = x;
	inf->dscnt_order[inf->depth].y = y;
	inf->depth+=1;
}
quadtree *find_brnch_rev(quadtree *tree, find_info *inf){
	while(inf->depth > 0){
		dir_t d = inf->dscnt_order[--inf->depth];
		quadtree *new_tree = get_spacial_tree(tree, d, NULL);
		if(new_tree == NULL){
			return tree;
		}
		tree = new_tree;
	}
	return tree;
}


// tree: current tree we are searching for a neigbor of
// d: direction we are trying to move (x,y)
// depth: the current depth we need to traverse to find the neighbor
quadtree *find_brnch_dpth(quadtree *tree, find_info *inf){
	if(tree->prnt == NULL){
		return NULL;
	}
	dir_t d = inf->init_d;
	int x = tree->x + d.x, y = tree->y + d.y;
	quadtree *new_tree = get_spacial_tree(tree->prnt, (dir_t){x,y}, tree);
	if(tree->x + d.x > 1 || tree->x + d.x < 0 || tree->y + d.y > 1 || tree->y + d.y < 0 || new_tree == NULL){
		int x = (d.x)?!tree->x:tree->x;
		int y = (d.y)?!tree->y:tree->y;
		add_dscnt_point(inf, x, y);
		return find_brnch_dpth(tree->prnt, inf);
	}
	if(inf->depth == 0){
		return new_tree;
	}
	return find_brnch_rev(new_tree, inf);
}
quadtree *find_branch(quadtree *tree, dir_t d){
	find_info inf = {d,{-1,-1}, 0, NULL, 0};
	if(d.x == 0){
		inf.cnst_coord.x = tree->x;
	}
	if(d.y == 0){
		inf.cnst_coord.y = tree->y;
	}
	quadtree *found_tree = find_brnch_dpth(tree, &inf);
	if(inf.dscnt_order) {free(inf.dscnt_order);}
	return found_tree;
}
void remove_branch(quadtree *branch){
	if(branch->prnt != NULL){
		branch->prnt->chldrn[branch->x+(branch->y*2)] = NULL;
		branch->prnt = NULL;
	}
}
void delete_tree(quadtree *tree){
	tree->prnt->chldrn[tree->x+tree->y*2] = NULL;
	// free any children
	for(int i = 0; i < 4; i++){
		if(tree->chldrn[i] != NULL){
			delete_tree(tree->chldrn[i]);
		}
	}
	free(tree);
}
void print_down(quadtree *tree){
	if(tree != NULL && !qt_isempty(tree)){
		printf("%d\n|\n|\n", tree->w);
		for(int i = 0; i < 4; i++){
			printf("|%d", (tree->chldrn[i])?tree->chldrn[i]->w:-1);
		}
		printf("\n\n");
		print_down(tree->chldrn[0]);
		print_down(tree->chldrn[1]);
		print_down(tree->chldrn[2]);
		print_down(tree->chldrn[3]);
	}
}
void print_tree(quadtree *tree){
	printf("------UP------\n");
	quadtree *root = tree;
	while(root->prnt != NULL){
		root = root->prnt;
		printf("%d\n|\n|\n", root->w);
		for(int i = 0; i < 4; i++){
			printf("|%d", (root->chldrn[i])?root->chldrn[i]->w:-1);
		}
		printf("\n\n");
	}

	printf("-----DOWN-----\n");
	print_down(tree);
}


quadtree *qt_get_win(quadtree *root, int w){
	for(int i = 0; i < 4; i++){
		if(root->chldrn[i] != NULL){
			if(root->chldrn[i]->w == w){
				return root->chldrn[i];
			}
			if(root->chldrn[i]->w == 0){ // more to search under here
				quadtree *tr = qt_get_win(root->chldrn[i], w);
				if(tr != NULL){ return tr; }
			}
		}
	}
	return NULL;
}

void qt_remove_win(quadtree *root, int w){
	quadtree *tr = qt_get_win(root,w);
	if(tr == NULL){ // window was not part of our tree structure
		return;
	}
	quadtree *prnt = tr->prnt;
	delete_tree(tr);
	int chld_num = count_direct_chldrn(prnt);
	if(chld_num == 1){
		for(int i = 0; i < 4; i++){
			if(prnt->chldrn[i] != NULL && prnt->prnt != NULL){
				int x = prnt->x, y = prnt->y;
				set_branch(prnt->prnt, prnt->chldrn[i], x, y);
				augment_down(prnt->prnt);
				free(prnt);
				return;
			}
		}
	} else if(chld_num == 0 && prnt->w == 0 && prnt->prnt != NULL){
		delete_tree(prnt);
	}
	augment_down(prnt);
}
window_pos *add_pos(pos_list_t *pos_list, window_pos pos){
	pos_list->len+=1;
	if(pos_list->len >= pos_list->size){
		pos_list->size *= 2; // maybe change factor if too much memory?
		pos_list->lst = realloc(pos_list->lst, pos_list->size*sizeof(window_pos));
	}
	pos_list->lst[pos_list->len-1] = pos;
	return (window_pos*)(pos_list->lst+(pos_list->len-1));
}


void _gather_poses(quadtree *root, pos_list_t *pos_list, window_pos **head){
	for(int i = 0; i < 4; i++){
		if(root->chldrn[i] != NULL){
			if(root->chldrn[i]->w != 0){
				window_pos *new_head = add_pos(pos_list, **head); // add a copy of the current head that we will switch to later
				(*head)->x += (i%2)*((*head)->w/2);
				(*head)->y += (i/2)*((*head)->h/2);
				(*head)->w /= 2;
				(*head)->h /= 2;
				(*head)->win = root->chldrn[i]->w;
				root->chldrn[i]->pos = (**head);
				*head = new_head;
			} else {
				window_pos curr_pos = **head;
				(*head)->x += (i%2)*((*head)->w/2);
				(*head)->y += (i/2)*((*head)->h/2);
				(*head)->w /= 2;
				(*head)->h /= 2;
				root->chldrn[i]->pos = (**head);
				_gather_poses(root->chldrn[i], pos_list, head);
				**head = curr_pos; // reset where ever the sub function left off to the current layers dimensions
			}
		}
	}
}

void gather_positions(quadtree *root, pos_list_t *pos_list, int max_width, int max_height){
	window_pos **head = malloc(sizeof(window_pos*));
	window_pos tmp = {0, 0, max_width, max_height};
	window_pos *tmp2 = add_pos(pos_list, tmp);
	(*head) = tmp2;
	_gather_poses(root, pos_list, head);
	pos_list->len--;
}

pos_list_t  *display_wins(quadtree *root, int max_width, int max_height){
	pos_list_t *pos_list = malloc(sizeof(pos_list));
	pos_list->len = 0;
	pos_list->size = 10;
	pos_list->lst = malloc(pos_list->size*sizeof(window_pos));
	gather_positions(root, pos_list, max_width, max_height);
	return pos_list;
}

void display_positions(quadtree *root){
	for(int i = 0; i < 4; i++){
		if(root->chldrn[i] != NULL && root->chldrn[i]->w != 0){
			window_pos p = root->chldrn[i]->pos;
			printf("[%d]@(%d,%d): %dx%d\n", root->chldrn[i]->w, p.x, p.y, p.w, p.h);
		} else if(root->chldrn[i] != NULL){
			display_positions(root->chldrn[i]);
		}
	}
}
