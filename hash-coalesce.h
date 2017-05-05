/****************************************************************************
 *     Copyright (C) [2012 - ] Harish Raghuveer - All Rights Reserved       *
 *                                                                          *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY   *
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE      *
 * IMPLIED WARRANTIES OF MECHANTABILITY AND/OR FITNESS FOR A PARTICULAR     *
 * PURPOSE.                                                                 *
 ****************************************************************************/

/**
 * @file    hash-coalesce.c
 *
 * @brief   Implement coalesced hashing (combines the advantages of both
 *          open addressing and chaining based hash implementations.
 *
 * @author  Harish Raghuveer
 *
 * @version 1.0
 *
 * @date    2014-01-19
 */

#include "base-types.h"

#ifndef HASH_COALESCE_H_
#define HASH_COALESCE_H_

typedef struct clhash_bin_ clhash_bin_t;

struct clhash_bin_
{
    uint32_t   hb_prev;
    uint32_t   hb_next;
    void*      hb_item;
};

typedef struct      clhash_tbl_
{
    hash_func_t     clh_hash_func;
    bool            clh_initialized;
    uint32_t        clh_bincnt;
    clhash_bin_t*   clh_bins;
}   clhash_table_t;

#ifdef __cplusplus
extern "C" {
#endif

extern void* clh_handle_to_data      (void*            handle);

extern rc_t  clh_table_bincnt_set    (clhash_table_t*   clht);

extern rc_t  clh_table_hash_func_set (clhash_table_t*   clht);

extern rc_t  clh_table_init          (clhash_table_t*  clht);

extern rc_t  clh_table_new           (clhash_table_t** pclht);

extern rc_t  clh_table_delete        (clhash_table_t**  pclht, 
                                      void             (*data_dtor) (void*));

extern void* clh_table_insert        (clhash_table_t*   clht, 
                                      void*             item,
                                      int             (*compare) (void*, 
                                                                  void*));

extern void*  clh_table_remove       (clhash_table_t*   clht, 
                                      void*             item, 
                                      int             (*compare) (void*, void*),
                                      void            (*data_dtor) (void*) );

extern rc_t  clh_table_remove_handle (clhash_table_t*   clht, 
                                      void*             handle,
                                      void            (*data_dtor) (void*) );

extern void* clh_table_find          (clhash_table_t*   clht, 
                                      void*             item);

extern void  clh_table_walk          (clhash_table_t*   clht,
                                      void            (*walk_func) (void*) );

extern void  clh_table_walk2         (clhash_table_t*   clht,
                                      void            (*walk2_func) (void*, 
                                                                     void*),
                                      void*             arg);
#ifdef __cplusplus
}
#endif

#endif /* HASH_COALESCE_H_ */
