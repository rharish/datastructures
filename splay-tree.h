/*****************************************************************************
 * Copyright (C) [2012 - ] Harish Raghuveer - All Rights Reserved
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MECHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 ****************************************************************************/
/*!
 * \file   splay-tree.h
 *
 * \brief  Splay tree header file
 *
 * \author Harish Raghuveer
 *
 * \date
 *
 * [Detailed description goes here]
 */

#ifndef SPLAY_TREE_H_
#define SPLAY_TREE_H_

#define DEFINE_splay_AUX_DATA

DEFINE_TEMPLATE_BST_COMPARATOR(splay)

DEFIME_TEMPLATE_BST_STRUCT(splay)

#ifdef __cplusplus
extern "C" {
#endif

extern splay_node_t* splay_nilp         (void);

extern void          splay_node_init    (splay_node_t*  spnode, 
                                         void* item);

extern rc_t          splay_node_new     (splay_node_t** splay_nodep, 
                                         void*          data);

extern void          splay_node_delete  (splay_node_t** splay_nodep);

extern splay_node_t* splay_insert       (splay_node_t**     splay_rootp,
                                         splay_node_t*      nilp, 
                                         splay_comparator_t splay_comp,
                                         splay_node_t*      node);

extern splay_node_t* splay_remove       (splay_node_t**     splay_rootp, 
                                         splay_node_t*      nilp, 
                                         splay_comparator_t splay_comp,
                                         splay_node_t*      node);

extern splay_node_t* splay_find_node    (splay_node_t**     splay_rootp, 
                                         splay_node_t*      nilp, 
                                         splay_comparator_t splay_comp,
                                         splay_node_t*      node);

extern splay_node_t* splay_find         (splay_node_t**     splay_rootp, 
                                         splay_node_t*      nilp, 
                                         splay_comparator_t splay_comp,
                                         void*              item);

extern splay_node_t* splay_delete_min   (splay_node_t**     splay_rootp, 
                                         splay_node_t*      nilp, 
                                         splay_comparator_t splay_comp);

extern splay_node_t* splay_decrease_key (splay_node_t**     splay_rootp,
                                         splay_node_t*      nilp,
                                         splay_comparator_t splay_comp,
                                         splay_node_t*      node,
                                         void*              newkey);
#ifdef __cplusplus
}
#endif

#endif /* SPLAY_TREE_H_ */
