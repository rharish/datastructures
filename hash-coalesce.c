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

#include "hash-coalesce.h"
#include <assert.h>
#include <stdlib.h>

/**
 * @brief Initialize a coalesced hash table. Please ensure that the APIs to
 *        set the hash function pointer and number of buckets (bins) in the
 *        hash table are properly initialized.
 *
 *        Ensure that the hash table is initialized only once.
 *
 * @param clht Pointer to hash table
 *
 * @return ROK on success; RFAIL on failure.
 */
rc_t
clh_table_init (clhash_table_t* clht)
{
    clhash_bin_t* bucket;
    int           i;

    /* Already initialized. */

    if (clht->clh_initialized == true)
        RETVAL (RFAIL);

    assert (clht->clh_hash_func != NULL && 
            clht->clh_bincnt != 0);

    clht->clh_bins = calloc (clht->clh_bincnt, sizeof(clhash_bin_t));
    if (clht->clh_bins != NULL)
    {
        /* create 'n' circular doubly linked lists */
        bucket = &clht->clh_bins [0];
        for (i = 0; i < clht->clh_bincnt; ++i)
        {
            bucket->hb_prev = i;
            bucket->hb_next = i;
            ++bucket;
        }
        clht->clh_initialized = true;
        RETVAL (ROK);
    }
    RETVAL (RFAIL);
}

/**
 * @brief       Create a new coalesced hash table.
 *
 * @param pclht Pointer to the address of the hash table.
 *
 * @return      ROK on success; RFAIL otherwise.
 */
rc_t
clh_table_new (clhash_table_t** pclht)
{
    clhash_table_t* clht;

    assert (pclht);

    clht                  = calloc (1, sizeof(*clht));
    clht->clh_initialized = false;
    *pclht                = clht; 

    RETVAL ((clht == NULL) ?  RFAIL : ROK);
}

/**
 * @brief       Destroy a new coalesced hash table. If a destructor for 
 *              cleaning up the memory associated with the stored item type 
 *              is provided, free them too.
 *
 * @param pclht Pointer to the address of the hash table.
 *
 * @return      ROK on success; RFAIL otherwise.
 */
rc_t
clh_table_delete (clhash_table_t**   pclht, 
                  void             (*data_dtor) (void*) )
{
    clhash_table_t* clht;
    int i;

    if (pclht == NULL)
        RETVAL (RFAIL);

    clht  = *pclht;
    *pclht = NULL;
    for (i = 0; i < clht->clh_bincnt; ++i) 
    {
        if (clht->clh_bins [i].hb_item) 
        {
            if (data_dtor)
                (*data_dtor) (clht->clh_bins [i].hb_item);
            clht->clh_bins [i].hb_item = NULL;
        }
    }
    free (clht->clh_bins);
    free (clht);
    RETVAL (ROK);
}

/**
 * @brief      Insert a data item into a coalesced hash table.
 *
 * @param clht Pointer to hash table
 * @param item Pointer to the item
 *
 * @return     An opaque handle on success; NULL otherwise.
 */
void*
clh_table_insert (clhash_table_t*   clht, 
                  void*             item, 
                  int             (*compare) (void*, void*) )
{
    register uint32_t hval      = (*clht->clh_hash_func) (item);
    register uint32_t i         = hval;
    register uint32_t previndex = hval;
             void*    binitem;

    /* we found an existing hash chain, which is part of same/another 
     * hash value 
     */
    do
    {
        if (clht->clh_bins [i].hb_item != NULL)
        {
            /* We have a comparator to enter unique item. */
            if (compare != NULL)
            {
                /* Already existing -- success case. */
                binitem = clht->clh_bins [i].hb_item;
                if ( (*compare) (item, binitem) == 0)
                    RETVAL (&clht->clh_bins [i]);
            }

            /* Goto next slot */
            i = (i + 1) % clht->clh_bincnt;

            /* No empty slots -- failure case */
            if (i == hval)
                break;
        }
        else
        {
            /* Found an empty slot 
             * Insert this node as the next bin in the hash ring.
             * NOTE: 'previndex' is the last bin in ihe ring rooted at 'hval'.
             */
            previndex                         = clht->clh_bins [hval].hb_prev;
            clht->clh_bins [previndex].hb_next = i;
            clht->clh_bins [i].hb_prev         = previndex;
            clht->clh_bins [hval].hb_prev      = i;
            clht->clh_bins [i].hb_next         = hval;
            clht->clh_bins [i].hb_item        = item;
            RETVAL (&clht->clh_bins [i]);
        }
    }
    while (1);
    RETVAL (NULL);
}

/**
 * @brief           Remove an item from the coalesced hash table. Call the
 *                  destructor routine if supplied. The comparator is
 *                  necessary to ensure that we're removing the correct item
 *                  (considering that multiple datum can return the same
 *                  hash vlue).
 *
 * @param clht      Pointer to coalesced hash table.
 * @param key       Key value.
 * @param compare   Comparator callback
 * @param data_dtor Destructor callback.
 *
 * @return          The stored item if the destructor is not provided and
 *                  the item is found successfully.
 *                  NULL if item not found or item is not found.
 */
void* 
clh_table_remove (clhash_table_t* clht, 
                  void*           key,
                  int           (*compare)   (void*, void*),
                  void          (*data_dtor) (void*) )
{
    void*     item      = NULL;
    uint32_t  hashval   = (*clht->clh_hash_func) (key);
    uint32_t  i         = hashval;
    uint32_t  previndex = 0;
    uint32_t  nextindex = 0;

    /* We only need to look in the current hash ring */
    do 
    {
        if (clht->clh_bins [i].hb_item == NULL)
            break;

        if ((*compare) (key, clht->clh_bins [i].hb_item) == 0)
        {
            item                           = clht->clh_bins [i].hb_item;
            clht->clh_bins [i].hb_item     = NULL;

            /* Remove the bin from the ring */
            previndex                          = clht->clh_bins [i].hb_prev;
            nextindex                          = clht->clh_bins [i].hb_next;
            clht->clh_bins [previndex].hb_next = nextindex;
            clht->clh_bins [nextindex].hb_prev = previndex;
            clht->clh_bins [i].hb_prev         = clht->clh_bins [i].hb_next = i;

            /* We are expected to called the destructor. */
            if (data_dtor)
            {
                (*data_dtor) (item);
                item = NULL; 
            }
            break;
        }
        i = clht->clh_bins [i].hb_next;
    } 
    while (i != hashval); 
    RETVAL (item);
}

/**
 * @brief           Remove an item from the coalesced hash table using the 
 *                  handle returned to the application when the item was 
 *                  inserted.
 *
 * @param clht      Pointer to the hash table
 * @param handle    Handle obtained when the item was inserted into the
 *                  table.
 * @param data_dtor Destructor to the object, if any.
 *
 * @return A valid pointer to the object if destructor is not passsed as an
 *         argument and handle contained the item; NULL otherwise
 */
void* 
clh_remove_handle (clhash_table_t*  clht, 
                   void*            handle,
                   void           (*data_dtor) (void*) )
{
    clhash_bin_t* bucket    = handle;
    void*         item      = NULL;
    uint32_t      i         = bucket - &clht->clh_bins [0];
    uint32_t      previndex = 0;
    uint32_t      nextindex = 0;

    assert (i < clht->clh_bincnt);

    item            = bucket->hb_item;
    bucket->hb_item = NULL;
    previndex       = clht->clh_bins [i].hb_prev;
    nextindex       = clht->clh_bins [i].hb_next;

    /* Remove from hash ring */
    clht->clh_bins [previndex].hb_next = nextindex;
    clht->clh_bins [nextindex].hb_prev = previndex;
    clht->clh_bins [i].hb_prev         = i;
    clht->clh_bins [i].hb_next         = i;

    if (data_dtor) 
    {
        (*data_dtor) (item);
        item = NULL;
    }

    RETVAL (item);
}

/**
 * @brief      Find an existing object (if any) in the coalesced hash table 
 *             using key value.
 *
 * @param clht Pointer to the hash table
 * @param key  Key value
 * @param      compare Pointer to the comparator function (< 0
 *
 * @return     A valid object pointer if success; NULL otherwise
 */
void* 
clh_find (clhash_table_t* clht, 
          void*           key,
          int             (*compare) (void*, void*) )
{
    clhash_bin_t* bucket;
    void*         item    = NULL;
    uint32_t      hashval = (*clht->clh_hash_func) (key);
    uint32_t      i       = hashval;

    do
    {
        bucket = &clht->clh_bins [i];

        /* this item will be part of the chain, in which bucket index
         * 'hashval' is also part of. look at the insert logic */
        if (bucket->hb_item == NULL)
            break;

        if ( (*compare) (key, bucket->hb_item) == 0)
        {
            item = bucket->hb_item;
            break;
        }
        i = bucket->hb_next;
    } while (i != hashval);

    RETVAL (item);
}

/**
 * @brief      Visit and perform some action on all items existing a colesced 
 *             hash table, using the walk routine passed as the argument.
 *
 * @param clht Pointer to hash table
 * @param walk Walk routine
 */
void 
hash_coalesce_walk (clhash_table_t* clht, 
                    void            (*walk) (void*) )
{
    void* object;
    int   i;

    for (i = 0; i < clht->clh_bincnt; ++i) 
    {
        object = clht->clh_bins [i].hb_item;
        if (object)
            (*walk) (object);
    }
    return;
}

/**
 * @brief       Visit and perform some action on all items existing a colesced 
 *              hash table, using the walk routine passed as the argument.
 *
 * @param clht  Pointer to hash table
 * @param walk2 Walk routine that take 2 arguments
 * @param arg   First argument to walk routine
 */
void 
hash_coalesce_walk2 (clhash_table_t*   clht, 
                     void            (*walk2) (void*, void*),
                     void*             arg)
{
    void* object;
    int   i;

    for (i = 0; i < clht->clh_bincnt; ++i)
    {
        object = clht->clh_bins [i].hb_item;
        if (object)
            (*walk2) (arg, object);
    }
    return;
}

