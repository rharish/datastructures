/***************************************************************************
 *     Copyright (C) [2012 - ] Harish Raghuveer - All Rights Reserved      *
 *                                                                         *
 *   THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF    *
 *   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO   *
 *   THE IMPLIED WARRANTIES OF MECHANTABILITY AND/OR FITNESS FOR A         *
 *   PARTICULAR PURPOSE.                                                   *
 ***************************************************************************
 ***************************************************************************
 * @file    rbtree.c                                                       *
 *                                                                         *
 * @author  Harish Raghuveer                                               *
 *                                                                         *
 * @version 1.0                                                            *
 *                                                                         *
 * @date    2016-07-03                                                     *
 ***************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "rbtree.h"

/**************************************************************************
 *                             Internal Functions                         *
 **************************************************************************/ 

static /* const */ rb_node_t
rb_nil_node =
{
    (uintptr_t) NULL,
    (rb_node_t*) &rb_nil_node,
    (rb_node_t*) &rb_nil_node,
    (rb_node_t*) &rb_nil_node,
    RB_BLACK
};
static rb_node_t *RB_NIL = (rb_node_t*)&rb_nil_node;

static inline void
rb_transplant (rb_node_t **root,
               rb_node_t  *u,
               rb_node_t  *v)
{
    if (u->rbn_paren == RB_NIL)
        *root = v;
    else if (u == u->rbn_paren->rbn_left)
        u->rbn_paren->rbn_left = v;
    else
        u->rbn_paren->rbn_right = v;
    if (v != RB_NIL)
        v->rbn_paren = u->rbn_paren;
}


static inline void
rbn_new (rb_node_t **nodep, 
         uintptr_t   data)
{
    rb_node_t *node;

    node = calloc (1, sizeof(*node));
    assert (node);
    if (node)
    {
        node->rbn_left  = RB_NIL;
        node->rbn_right = RB_NIL;
        node->rbn_paren = RB_NIL;
        node->rbn_data  = data;
        node->rbn_color = RB_RED;
    }
    *nodep = node;
}

static inline void
rbn_delete (void       *dtor,
            rb_node_t  *node)
{
    rb_dtor_t func = dtor;

    if (!node)
        return;

    if (dtor)
    {
        func = dtor;
        (*func) (node->rbn_data);
    }

    free (node);
    return;
}

static inline void
rb_int_init (rb_node_t **rootp)
{
    *rootp = RB_NIL;
}

static inline void
rb_rotate_l (rb_node_t **root, 
             rb_node_t  *x)
{
    rb_node_t  *y;

    y            = x->rbn_right;       /* set y */
    x->rbn_right = y->rbn_left;        /* turn y's left subtree into x's 
                                        * right subtree */
    if (y->rbn_left != RB_NIL)
    {
        y->rbn_left->rbn_paren = x;
    }

    y->rbn_paren = x->rbn_paren;       /* link x's parent to y's */

    if (x->rbn_paren == RB_NIL)
        *root = y;
    else if (x == x->rbn_paren->rbn_left)
        x->rbn_paren->rbn_left = y;
    else
        x->rbn_paren->rbn_right = y;

    y->rbn_left  = x;
    x->rbn_paren = y;
}

static inline void
rb_rotate_r (rb_node_t **root, 
             rb_node_t  *y)
{
    rb_node_t  *x;

    x           = y->rbn_left;
    y->rbn_left = x->rbn_right;

    if (x->rbn_right != RB_NIL)
    {
        x->rbn_right->rbn_paren = y;
    }

    x->rbn_paren = y->rbn_paren;       

    if (y->rbn_paren == RB_NIL)
        *root = x;
    else if (y == y->rbn_paren->rbn_right)
        y->rbn_paren->rbn_right = x;
    else
        y->rbn_paren->rbn_left = x;

    x->rbn_right = y;
    y->rbn_paren = x;
}

static inline void
rb_min (rb_node_t  *root, 
        rb_node_t **nodep)
{
    rb_node_t* node = root;              

    while (node->rbn_left != RB_NIL)         
        node = node->rbn_left;                   

    *nodep = node;
    return;
}

static inline void
rb_max (rb_node_t  *root, 
        rb_node_t **nodep)
{
    rb_node_t* node = root;              

    while (node->rbn_right != RB_NIL)         
        node = node->rbn_right;                   

    *nodep = node;
    return;
}


static void
rb_insert_fixup (rb_node_t **rootp, 
                 rb_node_t  *z,
                 rb_cmp_t    cmp)
{
    rb_node_t *paren;
    rb_node_t *uncle;
    rb_node_t *grand_paren;

    while (/* (z->rbn_paren != RB_NIL) && */
           (z->rbn_paren->rbn_color == RB_RED))
    {
        paren       = z->rbn_paren;
        grand_paren = paren->rbn_paren;

        if (paren == grand_paren->rbn_left)
        {
            uncle = grand_paren->rbn_right;
            if (uncle->rbn_color == RB_RED)
            {
                paren->rbn_color       = RB_BLACK;
                uncle->rbn_color       = RB_BLACK;
                grand_paren->rbn_color = RB_RED;
                z                      = grand_paren;
            }
            else
            {
                if (z == paren->rbn_right)
                {
                    z = paren;
                    rb_rotate_l (rootp, z);
                }
                paren->rbn_color       = RB_BLACK;
                grand_paren->rbn_color = RB_RED;
                rb_rotate_r (rootp, grand_paren);
            }
        }
        else
        {
            uncle = grand_paren->rbn_left;
            if (uncle->rbn_color == RB_RED)
            {
                paren->rbn_color       = RB_BLACK;
                uncle->rbn_color       = RB_BLACK;
                grand_paren->rbn_color = RB_RED;
                z                      = grand_paren;
            }
            else
            {
                if (z == paren->rbn_left)
                {
                    z = paren;
                    rb_rotate_r (rootp, z);
                }
                paren->rbn_color       = RB_BLACK;
                grand_paren->rbn_color = RB_RED;
                rb_rotate_l (rootp, grand_paren);
            }
        } 
    }
    if (*rootp != RB_NIL)
        (*rootp)->rbn_color = RB_BLACK;
}

static rc_t
rb_int_insert (rb_node_t **root,
               void       *data,
               rb_cmp_t    cmp,
               rb_node_t **node)
{
    rb_node_t *x;
    rb_node_t *y;
    rb_node_t *z;
    int        rc;

    rbn_new (&z, (uintptr_t) data);
    if (NULL == z) 
    {
        *node = RB_NIL;
        return (RFAIL);
    }

    y = RB_NIL;
    x = *root;

    while (x != RB_NIL)
    {
        y = x;
        rc = (*cmp)(z->rbn_data, 
                    x->rbn_data);
        if (rc < 0)
            x = x->rbn_left;
        else
            x = x->rbn_right;
    }
    z->rbn_paren = y;
    rc = (*cmp)(z->rbn_data, 
                y->rbn_data);
    if (y == RB_NIL)
        *root = z;
    else if (rc < 0)
        y->rbn_left = z;
    else
        y->rbn_right = z;

    rb_insert_fixup (root, 
                     z,
                     cmp);
    *node = z;
    return (ROK);
}

static void
rb_remove_fixup (rb_node_t **rootp,
                 rb_node_t  *x,
                 rb_cmp_t    cmp)
{
    rb_node_t *sibling;

    while ((x            != *rootp) &&
           (x->rbn_color == RB_BLACK))
    {
        if (x == x->rbn_paren->rbn_left)
        {
            sibling = x->rbn_paren->rbn_right;

            if (sibling->rbn_color == RB_RED)
            {
                sibling->rbn_color      = RB_BLACK;
                x->rbn_paren->rbn_color = RB_RED;
                rb_rotate_l (rootp, 
                             x->rbn_paren);
                sibling                 = x->rbn_paren->rbn_right;
            }

            if ((sibling->rbn_left->rbn_color  == RB_BLACK) &&
                (sibling->rbn_right->rbn_color == RB_BLACK))
            {
                sibling->rbn_color = RB_RED;
                x                  = x->rbn_paren;
            }
            else
            {
                if (sibling->rbn_right->rbn_color == RB_BLACK)
                {
                    sibling->rbn_left->rbn_color = RB_BLACK;
                    sibling->rbn_color           = RB_RED;
                    rb_rotate_r (rootp, 
                                 sibling);
                    sibling                      = x->rbn_paren->rbn_right;
                }

                sibling->rbn_color            = x->rbn_paren->rbn_color;
                x->rbn_paren->rbn_color       = RB_BLACK;
                sibling->rbn_right->rbn_color = RB_BLACK;
                rb_rotate_l (rootp, 
                             x->rbn_paren);
                x                             = *rootp;
            }
        }
        else
        {
            sibling = x->rbn_paren->rbn_left;

            if (sibling->rbn_color == RB_RED)
            {
                sibling->rbn_color      = RB_BLACK;
                x->rbn_paren->rbn_color = RB_RED;
                rb_rotate_r (rootp, 
                             x->rbn_paren);
                sibling                 = x->rbn_paren->rbn_left;
            }

            if ((sibling->rbn_left->rbn_color  == RB_BLACK) &&
                (sibling->rbn_right->rbn_color == RB_BLACK))
            {
                sibling->rbn_color = RB_RED;
                x                  = x->rbn_paren;
            }
            else
            {
                if (sibling->rbn_left->rbn_color == RB_BLACK)
                {
                    sibling->rbn_right->rbn_color = RB_BLACK;
                    sibling->rbn_color            = RB_RED;
                    rb_rotate_l (rootp, 
                                 sibling);
                    sibling                       = x->rbn_paren->rbn_left;
                }

                sibling->rbn_color           = x->rbn_paren->rbn_color;
                x->rbn_paren->rbn_color      = RB_BLACK;
                sibling->rbn_left->rbn_color = RB_BLACK;
                rb_rotate_l (rootp, 
                             x->rbn_paren);
                x                            = *rootp;
            }
        }
    }
    x->rbn_color = RB_BLACK;
}

static rc_t
rb_int_remove (rb_node_t **rootp,
               rb_node_t  *z,
               rb_cmp_t    cmp)
{
    rb_node_t *x;
    rb_node_t *y;
    rb_color_t color;

    y     = z;
    color = y->rbn_color;

    if (z->rbn_left == RB_NIL)
    {
        x = z->rbn_right;
        rb_transplant (rootp, 
                       z, 
                       z->rbn_right);
    }
    else if (z->rbn_right == RB_NIL)
    {
        x = z->rbn_left;
        rb_transplant (rootp, 
                       z, 
                       z->rbn_left);
    }
    else
    {
        rb_min (z->rbn_right, &y);
        color = y->rbn_color;
        x     = y->rbn_right;

        if (y->rbn_paren == z)
            x->rbn_paren = z;
        else
        {
            rb_transplant (rootp, 
                           y, 
                           y->rbn_right);
            y->rbn_right            = z->rbn_right;
            y->rbn_right->rbn_paren = y;
        }
        rb_transplant (rootp, 
                       z, 
                       y);
        y->rbn_left            = z->rbn_left;
        y->rbn_left->rbn_paren = y;
        y->rbn_color           = z->rbn_color;
    }
    if (color == RB_BLACK)
        rb_remove_fixup (rootp, 
                         x, 
                         cmp);
    if (z == *rootp)
        *rootp = RB_NIL;
    rbn_delete (NULL, z);
    return (ROK);
}

rc_t
rb_int_find (rb_node_t  *root,
             void       *data,
             rb_cmp_t    cmp,
             rb_node_t **nodep)
{
    rb_node_t *node;
    int        rc;

    node = root;
    while (1)
    {                                               
        if (node == RB_NIL)                     
            break;                                  
        rc = (*cmp)((uintptr_t) data, 
                    (uintptr_t) node->rbn_data); 
        if (rc == 0)
            break;
        node = ((rc < 0)          ? 
                    node->rbn_left : 
                    node->rbn_right);   
    }                                               
    *nodep = node;
    return ((node != RB_NIL) ? ROK : RFAIL); 

}

static rb_node_t*                                      
rb_next (rb_node_t* rb_node)
{                                                       
    rb_node_t* node;

    if (rb_node->rbn_right != RB_NIL)        
    {
        rb_min(rb_node->rbn_right, 
               &node);
        return (node);
    } 

    node = rb_node->rbn_paren;                  
    while ((node != RB_NIL) &&                      
           (rb_node == node->rbn_right))         
    {                                                   
        rb_node = node;                             
        node    = node->rbn_paren;              
    }                                                   
    return (node);                                      
}

int
rb_int_height (rb_node_t  *root)
{
    int hl, hr;

    if (root != RB_NIL)
    {
        hl = rb_int_height (root->rbn_left);
        hr = rb_int_height (root->rbn_right);
        if (hl >= hr)
            return hl + 1;
        else 
            return hr + 1;

    }
    return 0;
}

void
rb_int_inorder (rb_node_t   *root, 
                void        *cb, 
                rb_visit_t   visit2)
{
    rb_node_t *node = root;

    rb_min (root, &node);
    for (;node != RB_NIL; node = rb_next (node))
        (*visit2) (cb, node);
}

void
rb_int_preorder (rb_node_t   *root, 
                 void        *cb, 
                 rb_visit_t   visit2)
{
    if (root != RB_NIL)
    {
        (*visit2)(cb, 
                  root);
        rb_int_preorder (root->rbn_left, 
                          cb, 
                          visit2);
        rb_int_preorder (root->rbn_right, 
                          cb, 
                          visit2);
    }
}

void
rb_int_pstorder  (rb_node_t   *root, 
                  void        *cb, 
                  rb_visit_t   visit2)
{
    if (root != RB_NIL)
    {
        rb_int_pstorder (root->rbn_left, 
                          cb, 
                          visit2);
        rb_int_pstorder (root->rbn_right, 
                          cb, 
                          visit2);
        (*visit2) (cb, root);
    }
}

/**************************************************************************
 *                                   API                                  *
 **************************************************************************/ 

/**
 * @brief 
 *
 * @param treep
 */
void
rb_init (rb_tree_t *tree)
{
    rb_int_init (&tree->rb_root);
    tree->rb_cnt = 0;
}

/**
 * @brief 
 *
 * @param tree
 */
void
rb_new (rb_tree_t **treep)
{
    rb_tree_t *tree;

    tree = calloc(1, sizeof(*tree));
    if (tree != NULL)
        rb_init (tree);
    *treep = tree;
}

void
rb_walk  (rb_tree_t       *tree, 
          rb_trav_order_t  order,
          void            *cb, 
          rb_visit_t       visit)
{
    switch (order)
    {
    default:
        break;

    case RB_TRAV_INORDER:
        rb_int_inorder (tree->rb_root, cb, visit);
        break;

    case RB_TRAV_PREORDER:
        rb_int_preorder (tree->rb_root, cb, visit);
        break;

    case RB_TRAV_PSTORDER:
        rb_int_pstorder (tree->rb_root, cb, visit);
        break;

#if 0
    case RB_TRAV_LVLORDER:
        rb_int_lvlorder2 (tree->rb_root, cb, visit);
        break;
#endif
    }
}

/**
 * @brief 
 *
 * @param tree
 * @param dtor
 */
void
rb_delete (rb_tree_t **treep, 
           rb_dtor_t   dtor)
{
    rb_tree_t *tree;

    if (!treep || !*treep)
        return;

    tree = *treep;
    rb_walk (tree, RB_TRAV_PSTORDER, dtor, rbn_delete);
    free (tree);
    *treep = NULL;
}

int
rb_height (rb_tree_t  *tree)
{
    return (rb_int_height(tree->rb_root));
}
/**
 * @brief 
 *
 * @param tree
 * @param item
 * @param cmp
 */
void
rb_find (rb_tree_t  *tree, 
         void       *item, 
         rb_cmp_t    cmp,
         rb_node_t **nodep)
{

    rb_int_find (tree->rb_root,
                 item,
                 cmp,
                 nodep);
    if (*nodep == RB_NIL)
        printf ("Item not found!\n");
}
        
/**
 * @brief 
 *
 * @param tree
 * @param item
 * @param cmp
 */
void
rb_insert (rb_tree_t  *tree, 
           void       *item, 
           rb_cmp_t    cmp,
           rb_node_t **nodep)
{
    rc_t       rc;

    rc = rb_int_insert (&tree->rb_root, 
                        item, 
                        cmp,
                        nodep);
    if (rc == ROK)
        ++tree->rb_cnt;
    return; 
}


/**
 * @brief 
 *
 * @param tree
 * @param item
 * @param cmp
 */
void
rb_remove_handle (rb_tree_t  *tree, 
                  rb_node_t  *node, 
                  rb_cmp_t    cmp)
{
    rc_t       rc;

    if (node != RB_NIL)
    {
        rc = rb_int_remove (&tree->rb_root, 
                             node, 
                             cmp);
        if (rc == ROK)
            --tree->rb_cnt;
    }
    return; 
}

/**
 * @brief 
 *
 * @param tree
 * @param item
 * @param cmp
 */
void
rb_remove (rb_tree_t *tree, 
           void      *item, 
           rb_cmp_t   cmp)
{
    rb_node_t *node = RB_NIL;

    rb_find (tree,
             item,
             cmp,
             &node);

    if (node == RB_NIL)
    {
        printf ("Item not found! Not removing..\n");
        return;
    }

    rb_remove_handle (tree,
                      node, 
                      cmp);
    return; 
}

#ifdef ETEST

int
intcmp (uintptr_t a, uintptr_t b)
{
    return ((int) a - (int) b);
}

void
intprint (void *cb __attribute__((unused)), rb_node_t *n)
{
    printf("%d ", (int) n->rbn_data);
}

int
main (int argc, char **argv)
{
    rb_tree_t *tp;
    rb_node_t *n;
    int        nfound = 0;
    int        found  = 0;
    int        i;

    rb_new (&tp);

    for (i = 0; i <  65536; ++i) {
        rb_insert (tp, (void*) i, intcmp, &n);
        if (n == RB_NIL)
            printf ("insert(%d) failed!\n", (i+1));
        // printf("%p : <%d>\n", n, i);
    }

    for (i = 0; i < 65536; ++i)
    {
        rb_find (tp, (void*) i, intcmp, &n);
        if (n == RB_NIL) {
            printf ("%d not foubd!\n", i);
            ++nfound;
        }
        else
            ++found;
    }

    rb_walk (tp, 
             RB_TRAV_INORDER,
             NULL,
             intprint);
    printf("\n");

    printf ("found %d; missed %d\n", found, nfound);
    printf ("root %d\n", tp->rb_root->rbn_data);
    printf ("Height of RB-tree is %d\n",
            rb_height (tp));

    for (i = 0; i < 65536; ++i)
    {
        rb_remove (tp, (void *) i, intcmp);
        // printf("%p : <%d>\n", tp->rb_root, tp->rb_root->rbn_data);
    }
    printf ("Height of RB-tree is %d\n",
            rb_height (tp));

    rb_delete (&tp, NULL);
    return 0;
}
#endif
