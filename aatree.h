/***************************************************************************
 *     Copyright (C) [2012 - ] Harish Raghuveer - All Rights Reserved      *
 *                                                                         *
 *   THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF    *
 *   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO   *
 *   THE IMPLIED WARRANTIES OF MECHANTABILITY AND/OR FITNESS FOR A         *
 *   PARTICULAR PURPOSE.                                                   *
 ***************************************************************************/
/* *************************************************************************
 * @file    aatree.h
 *
 * @brief
 *
 * From Arne Andersson's paper "Balanced Search Trees Made Simple"
 *
 * Basic rules:
 *  1) Every path contains the same number of pseudo-nodes.
 *  2) A left child may not have the same level as its parent.
 *  3) Two right children with the same level as the parent are not allowed.
 * 
 *
 * @author  Harish Raghuveer
 *
 * @version 1.0
 *
 * @date    2016-07-07
 ***************************************************************************/

#ifndef AA_TREE_H_
#define AA_TREE_H_

#define LEFT    0
#define RIGHT   1

#define LLINK(n)    (n)->aa_link[LEFT]
#define RLINK(n)    (n)->aa_link[RIGHT]

typedef enum
{
    ROK,
    RFAIL
} rc_t;


typedef int             (*aa_cmp_t) (void *, void *);

typedef struct aa_node_ aa_node_t;

struct aa_node_
{
    aa_node_t *aa_link [2];
    uintptr_t  aa_data;
    uint8_t    aa_level;
};


#endif /* AA_TREE_H_ */
