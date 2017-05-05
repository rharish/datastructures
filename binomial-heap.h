/****************************************************************************
 *     Copyright (C) [2012 - ] Harish Raghuveer - All Rights Reserved       *
 *                                                                          *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY   *
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE      *
 * IMPLIED WARRANTIES OF MECHANTABILITY AND/OR FITNESS FOR A PARTICULAR     *
 * PURPOSE.                                                                 *
 * **************************************************************************/

/**
 * @file    binomial-heap.h
 *
 * @author  Harish Raghuveer
 *
 * @version 1.0
 *
 * @date    (Created On) 2013-10-02
 *
 * @brief   This file contains the API binomial heap for priorityq queue
 * ADT.Dependencies:
 * - stdlib.h
 * - stdbool.h
 * - stdint.h
 */

#ifndef BINOMIAL_HEAP_H_
#define BINOMIAL_HEAP_H_

typedef struct binomial_heap_      binomial_heap_t;
typedef struct binomial_heap_node_ binomial_heap_node_t;

struct binomial_heap_
{
    binomial_heap_node_t* bh_head;
    uint32_t              bh_count;
};

#define bh_node_isroot(bn)  ((bn)->bn_parent == NULL)

struct binomial_heap_node_
{
    binomial_heap_node_t* bn_parent;
    binomial_heap_node_t* bn_lchild;
    binomial_heap_node_t* bn_rsibling;
    void*                 bn_key;
    uint32_t              bn_degree;   /* number of children */
};

/****************************************************************************
 *                            BINOMIAL QUEUE API                            *
 ****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

extern void 
       binomial_heap_new          (binomial_heap_t** pbh);

extern void 
       binomial_heap_destroy      (binomial_heap_t** pbh, 
                                   bool              destroy_all);

extern binomial_heap_node_t* 
       binomial_heap_min          (binomial_heap_t*  bh, 
                                   int               (*compare) (void*, 
                                                                 void*) );
extern binomial_heap_node_t*
       binomial_heap_insert       (binomial_heap_t** pbh, 
                                   void*             key, 
                                   int               (*compare) (void*, 
                                                                 void*) );
extern binomial_heap_node_t*
       binomial_heap_extract_min  (binomial_heap_t** pbh, 
                                   int               (*compare) (void*, 
                                                                 void*) );
extern binomial_heap_node_t*
       binomial_heap_decrease_key (binomial_heap_t*      bh,
                                   binomial_heap_node_t* node,
                                   void*                 newkey,
                                   int                   (*compare) (void*, 
                                                                     void*) );
extern binomial_heap_node_t*
       binomial_heap_delete       (binomial_heap_t**     pbh, 
                                   binomial_heap_node_t* node,
                                   int                   (*compare) (void*, 
                                                                     void*) );
#ifdef __cplusplus
}
#endif

#endif /* BINOMIAL_HEAP_H_ */

