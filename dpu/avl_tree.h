#pragma once

#include <stdint.h>

#include "util.h"

#define LEFTHEAVY  -1
#define BALANCED   0
#define RIGHTHEAVY 1

typedef struct avl_node
{
    uint32_t key;
    MRAM void *data;
    MRAM struct avl_node *parent;
    MRAM struct avl_node *left;
    MRAM struct avl_node *right;
    int32_t balance;
} avl_node_t;

typedef struct avl_tree
{
    MRAM struct avl_node *root;
} avl_tree_t;

MRAM avl_tree_t *avl_tree_alloc();
MRAM avl_node_t *avl_tree_node_alloc(uint32_t key, MRAM void *data, MRAM avl_node_t *parent);

MRAM void *avl_tree_find(MRAM avl_tree_t *tree, uint32_t key);

void avl_tree_rotate_right(MRAM avl_node_t *x, MRAM avl_node_t *z);
void avl_tree_rotate_left(MRAM avl_node_t *x, MRAM avl_node_t *z);
void avl_tree_fix_left_imbalance(MRAM avl_tree_t *tree, MRAM avl_node_t *x, MRAM avl_node_t *z);
void avl_tree_fix_right_imbalance(MRAM avl_tree_t *tree, MRAM avl_node_t *x, MRAM avl_node_t *z);
void avl_tree_insert(MRAM avl_tree_t *tree, uint32_t key, MRAM void *data);

// void avl_tree_print(MRAM avl_node_t *node, int depth);
