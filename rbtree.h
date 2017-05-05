/***************************************************************************
 *     Copyright (C) [2012 - ] Harish Raghuveer - All Rights Reserved      *
 *                                                                         *
 *   THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF    *
 *   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO   *
 *   THE IMPLIED WARRANTIES OF MECHANTABILITY AND/OR FITNESS FOR A         *
 *   PARTICULAR PURPOSE.                                                   *
 ***************************************************************************
 ***************************************************************************
 * @file    rbtree.h                                                       *
 *                                                                         *
 * @author  Harish Raghuveer                                               *
 *                                                                         *
 * @version 1.0                                                            *
 *                                                                         *
 * @date    2016-07-03                                                     *
 ***************************************************************************/

#ifndef RB_TREE_H_
#define RB_TREE_H_

// C_DECL_BEGIN_

typedef enum
{
    ROK,
    RFAIL
} rc_t;

typedef enum
{
    RB_TRAV_INORDER = 0,
    RB_TRAV_DFLT    = RB_TRAV_INORDER,
    RB_TRAV_PREORDER,
    RB_TRAV_PSTORDER,
    RB_TRAV_LVLORDER
} rb_trav_order_t;

typedef enum
{
    RB_RED,
    RB_BLACK
}   rb_color_t;

typedef struct rb_node_ rb_node_t;
typedef int             (*rb_cmp_t)    (uintptr_t, uintptr_t);
typedef void            (*rb_dtor_t)   (uintptr_t);
typedef void            (*rb_visit_t)  (void*, rb_node_t*);

struct        rb_node_
{
    uintptr_t  rbn_data;
    rb_node_t *rbn_left;
    rb_node_t *rbn_right;
    rb_node_t *rbn_paren;
    rb_color_t rbn_color;
};

typedef struct rb_tree_
{
    rb_node_t *rb_root;
    uint32_t   rb_cnt;
} rb_tree_t;

// C_DECL_END_

#endif /* RB_TREE_H_ */
