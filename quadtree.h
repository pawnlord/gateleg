#ifndef QUADTREE
#define QUADTREE
#include <stdio.h>

struct quadtree_t {
    int w;
    struct quadtree_t *prnt;
    struct quadtree_t *chldrn[4];
    int x, y; 
};
typedef struct quadtree_t quadtree;

typedef struct {
    int x, y; // only one is expected to have a value
} dir_t;

quadtree *init_root(int root);
void add_branch(quadtree *prnt, quadtree *branch);
void qt_add_window(quadtree *prnt, int w);
void swap_branch(quadtree *arg1, quadtree *arg2);
quadtree *find_branch(quadtree *tree, dir_t d); // finds a branch in the specified direction
void remove_branch(quadtree *branch);
void delete_tree(quadtree *tree);
void print_tree(quadtree *tree);
quadtree *qt_get_win(quadtree *root, int w); // searches tree for w
void qt_remove_win(quadtree *root, int w);

#endif
