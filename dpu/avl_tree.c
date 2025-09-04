#include "avl_tree.h"
#include "alloc.h"
#include "tm.h"

MRAM avl_tree_t *
avl_tree_alloc()
{
    MRAM avl_tree_t *tree;

    tree = (MRAM avl_tree_t *)mram_malloc(sizeof(avl_tree_t));

    if (tree)
    {
        tree->root = NULL;
    }

    return tree;
}

MRAM avl_node_t *
avl_tree_node_alloc(uint32_t key, MRAM void *data, MRAM avl_node_t *parent)
{
    MRAM avl_node_t *node;

    node = (MRAM avl_node_t *)mram_malloc(sizeof(avl_node_t));

    if (node)
    {
        node->key     = key;
        node->data    = data;
        node->parent  = parent;
        node->left    = NULL;
        node->right   = NULL;
        node->balance = 0;
    }

    return node;
}

MRAM void *
avl_tree_find(MRAM avl_tree_t *tree, uint32_t key)
{
    MRAM avl_node_t *node;
    uint32_t tmp_key;
    MRAM void *tmp_data;

    node = (MRAM avl_node_t *)TM_LOAD_F(&tree->root);
    while (node != NULL)
    {
        tmp_key = TM_LOAD_F(&node->key);
        if (tmp_key == key)
        {
            tmp_data = (MRAM void *)TM_LOAD_F(&node->data);

            return tmp_data;
        }

        if (tmp_key > key)
        {
            node = (MRAM avl_node_t *)TM_LOAD_F(&node->left);
        }
        else
        {
            node = (MRAM avl_node_t *)TM_LOAD_F(&node->right);
        }
    }

    return NULL;
}

void
avl_tree_rotate_right(MRAM avl_node_t *x, MRAM avl_node_t *z)
{
    MRAM avl_node_t *tmp_node;
    // MRAM avl_node_t *tmp_node_2;

    TM_STORE_F(&x->left, (uintptr_t)z->right);

    tmp_node = (MRAM avl_node_t *)TM_LOAD_F(&x->left);
    if (tmp_node != NULL)
    {
        TM_STORE_F(&x->left->parent, (uintptr_t)x);
    }

    TM_STORE_F(&z->parent, (uintptr_t)x->parent);

    tmp_node = (MRAM avl_node_t *)TM_LOAD_F(&x->parent);
    if (tmp_node != NULL)
    {
        // tmp_node   = (MRAM avl_node_t *)TM_LOAD_F(&x);
        tmp_node = (MRAM avl_node_t *)TM_LOAD_F(&x->parent->left);
        if (x == tmp_node)
        {
            TM_STORE_F(&x->parent->left, (uintptr_t)z);
        }
        else
        {
            TM_STORE_F(&x->parent->right, (uintptr_t)z);
        }
    }

    TM_STORE_F(&z->right, (uintptr_t)x);
    TM_STORE_F(&x->parent, (uintptr_t)z);
}

void
avl_tree_rotate_left(MRAM avl_node_t *x, MRAM avl_node_t *z)
{
    MRAM avl_node_t *tmp_node;
    // MRAM avl_node_t *tmp_node_2;

    TM_STORE_F(&x->right, (uintptr_t)z->left);

    tmp_node = (MRAM avl_node_t *)TM_LOAD_F(&x->right);
    if (tmp_node != NULL)
    {
        TM_STORE_F(&x->right->parent, (uintptr_t)x);
    }

    TM_STORE_F(&z->parent, (uintptr_t)x->parent);

    tmp_node = (MRAM avl_node_t *)TM_LOAD_F(&x->parent);
    if (tmp_node != NULL)
    {
        // tmp_node   = (MRAM avl_node_t *)TM_LOAD_F(&x);
        tmp_node = (MRAM avl_node_t *)TM_LOAD_F(&x->parent->left);
        if (x == tmp_node)
        {
            TM_STORE_F(&x->parent->left, (uintptr_t)z);
        }
        else
        {
            TM_STORE_F(&x->parent->right, (uintptr_t)z);
        }
    }

    TM_STORE_F(&z->left, (uintptr_t)x);
    TM_STORE_F(&x->parent, (uintptr_t)z);
}

void
avl_tree_fix_left_imbalance(MRAM avl_tree_t *tree, MRAM avl_node_t *x, MRAM avl_node_t *z)
{
    int32_t balance, z_balance;
    // MRAM avl_node_t *node;
    MRAM avl_node_t *root;

#ifdef DEBUG
    assert(x != NULL);
    assert(z != NULL);
#endif

    balance   = (int32_t)TM_LOAD_F(&x->balance);
    z_balance = (int32_t)TM_LOAD_F(&z->balance);

    if (balance == z_balance)
    {
        avl_tree_rotate_right(x, z);
        AFTER_TM_FUNCTION_F();

        TM_STORE_F(&x->balance, BALANCED);
        TM_STORE_F(&z->balance, BALANCED);
    }
    else
    {
        balance = (int32_t)TM_LOAD_F(&z->right->balance);

        avl_tree_rotate_left(z, z->right);
        AFTER_TM_FUNCTION_F();

        avl_tree_rotate_right(x, x->left);
        AFTER_TM_FUNCTION_F();

        TM_STORE_F(&x->parent->balance, BALANCED);

        if (balance == LEFTHEAVY)
        {
            TM_STORE_F(&z->balance, BALANCED);
            TM_STORE_F(&x->balance, RIGHTHEAVY);
        }
        else if (balance == RIGHTHEAVY)
        {
            TM_STORE_F(&z->balance, LEFTHEAVY);
            TM_STORE_F(&x->balance, BALANCED);
        }
        else if (balance == BALANCED)
        {
            TM_STORE_F(&z->balance, BALANCED);
            TM_STORE_F(&x->balance, BALANCED);
        }
    }

    // node = (MRAM avl_node_t *)TM_LOAD_F(&x);
    root = (MRAM avl_node_t *)TM_LOAD_F(&tree->root);
    if (x == root)
    {
        TM_STORE_F(&tree->root, (uintptr_t)x->parent);
    }
}

void
avl_tree_fix_right_imbalance(MRAM avl_tree_t *tree, MRAM avl_node_t *x, MRAM avl_node_t *z)
{
    int32_t balance, z_balance;
    // MRAM avl_node_t *node;
    MRAM avl_node_t *root;

#ifdef DEBUG
    assert(x != NULL);
    assert(z != NULL);
#endif

    balance   = (int32_t)TM_LOAD_F(&x->balance);
    z_balance = (int32_t)TM_LOAD_F(&z->balance);

    if (balance == z_balance)
    {
        avl_tree_rotate_left(x, z);
        AFTER_TM_FUNCTION_F();

        TM_STORE_F(&x->balance, BALANCED);
        TM_STORE_F(&z->balance, BALANCED);
    }
    else
    {
        balance = (int32_t)TM_LOAD_F(&z->left->balance);

        avl_tree_rotate_right(z, z->left);
        AFTER_TM_FUNCTION_F();

        avl_tree_rotate_left(x, x->right);
        AFTER_TM_FUNCTION_F();

        TM_STORE_F(&x->parent->balance, BALANCED);

        if (balance == LEFTHEAVY)
        {
            TM_STORE_F(&x->balance, BALANCED);
            TM_STORE_F(&z->balance, RIGHTHEAVY);
        }
        else if (balance == RIGHTHEAVY)
        {
            TM_STORE_F(&x->balance, LEFTHEAVY);
            TM_STORE_F(&z->balance, BALANCED);
        }
        else if (balance == BALANCED)
        {
            TM_STORE_F(&x->balance, BALANCED);
            TM_STORE_F(&z->balance, BALANCED);
        }
    }

    // node = (MRAM avl_node_t *)TM_LOAD_F(&x);
    root = (MRAM avl_node_t *)TM_LOAD_F(&tree->root);
    if (x == root)
    {
        TM_STORE_F(&tree->root, (uintptr_t)x->parent);
    }
}

void
avl_tree_insert(MRAM avl_tree_t *tree, uint32_t key, MRAM void *data)
{
    MRAM avl_node_t *parent;
    MRAM avl_node_t *current;
    MRAM avl_node_t *tmp_child;
    uint32_t tmp_key;
    int32_t balance;

    parent  = NULL;
    current = (MRAM avl_node_t *)TM_LOAD_F(&tree->root);

    if (current == NULL)
    {
        TM_STORE_F(&tree->root, (uintptr_t)avl_tree_node_alloc(key, data, NULL));

        return;
    }

    while (current != NULL)
    {
        tmp_key = TM_LOAD_F(&current->key);
        if (tmp_key == key)
        {
            TM_STORE_F(&current->data, (uintptr_t)data);

            return;
        }

        parent = current;
        if (tmp_key > key)
        {
            current = (MRAM avl_node_t *)TM_LOAD_F(&current->left);
        }
        else
        {
            current = (MRAM avl_node_t *)TM_LOAD_F(&current->right);
        }
    }

    current = avl_tree_node_alloc(key, data, parent);

    tmp_key = TM_LOAD_F(&parent->key);
    if (tmp_key > key)
    {
        TM_STORE_F(&parent->left, (uintptr_t)current);
    }
    else
    {
        TM_STORE_F(&parent->right, (uintptr_t)current);
    }

    while (parent != NULL)
    {
        tmp_child = (MRAM avl_node_t *)TM_LOAD_F(&parent->left);
        balance   = (int32_t)TM_LOAD_F(&parent->balance);

        if (current == tmp_child)
        {
            // New node inserted on the left
            if (balance == RIGHTHEAVY)
            {
                TM_STORE_F(&parent->balance, BALANCED);
                break;
            }
            else if (balance == BALANCED)
            {
                TM_STORE_F(&parent->balance, LEFTHEAVY);
            }
            else if (balance == LEFTHEAVY)
            {
                avl_tree_fix_left_imbalance(tree, parent, current);
                AFTER_TM_FUNCTION_F();
                break;
            }
        }
        else
        {
            // New node inserted on the right
            if (balance == LEFTHEAVY)
            {
                TM_STORE_F(&parent->balance, BALANCED);
                break;
            }
            else if (balance == BALANCED)
            {
                TM_STORE_F(&parent->balance, RIGHTHEAVY);
            }
            else if (balance == RIGHTHEAVY)
            {
                avl_tree_fix_right_imbalance(tree, parent, current);
                AFTER_TM_FUNCTION_F();
                break;
            }
        }

        current = (MRAM avl_node_t *)TM_LOAD_F(&current->parent);
        parent  = (MRAM avl_node_t *)TM_LOAD_F(&current->parent);
    }
}

// void
// avl_tree_print(MRAM avl_node_t *node, int depth)
// {
//     if (!node)
//     {
//         return;
//     }

//     avl_tree_print(node->right, depth + 1);
//     printf("%*s", 8 * depth, "");
//     printf("K=%u\n", node->key);
//     avl_tree_print(node->left, depth + 1);
// }
