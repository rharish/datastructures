/****************************************************************************
 *     Copyright (C) [2012 - ] Harish Raghuveer - All Rights Reserved       *
 *                                                                          *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY   *
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE      *
 * IMPLIED WARRANTIES OF MECHANTABILITY AND/OR FITNESS FOR A PARTICULAR     *
 * PURPOSE.                                                                 *
 ****************************************************************************/
/**
 * @file   binomial-heap.c
 *
 * @author  Harish Raghuveer
 *
 * @version 1.0
 *
 * @date    (Created On) 2013-09-28
 *
 * @brief   This files implements binomial heaps.
 */

#include <stdlib.h>  
#include <stdbool.h> 
#include <stdint.h>  
#include <gperftools/profiler.h>  
#include "binomial-heap.h"

/****************************************************************************
 *                      STATIC VARIABLES AND FUNCTIONS                      *
 ****************************************************************************/

static int
binomial_heap_node_compare (binomial_heap_node_t* first,
                            binomial_heap_node_t* second,
                            int                   (*compare) (void*, void*))
{
    void* key1;
    void* key2;
    
    key1 = first->bn_key;
    key2 = second->bn_key;

    if (key1 == NULL)
    {
        /* both are NULL. */
        if (key2 == NULL)
        {
            return 0;
        }
        else 
        {
            /* Second one is not NULL. */
            return -1;
        }
    }
    else if (key2 == NULL)
    {
        /* both are NULL. */
        if (key1 == NULL)
        {
            /* This is not white box testetd. */
            return 0;
        }
        else 
        {
            /* First one is not NULL. */
            return 1;
        }
    }

    return (*compare)(key1, key2);
}

/**
 * @brief binomial_heap_node_link Link the B(k-1) tree rooted at node y to
 * the B(k-1) rooted at node z. Make 'z' the parent of y; making z the root
 * of a B(k) tree.
 *
 * @param[] y
 * @param[] z
 */
static inline void 
binomial_heap_node_link (binomial_heap_node_t* y, binomial_heap_node_t* z)
{
    y->bn_parent   = z;
    y->bn_rsibling = z->bn_lchild;
    z->bn_lchild   = y;
    ++z->bn_degree;
}

/**
 * @brief binomial_heap_node_sibling_sll_reverse__ Reverse right sibling
 * chain. Used during extract-min procedure.
 *
 * @param[] pnode Pointer to binomial heap node.
 */
static inline void
binomial_heap_node_sibling_sll_reverse__ (binomial_heap_node_t** pnode)
{
    binomial_heap_node_t* prev = NULL;
    binomial_heap_node_t* curr = *pnode;
    binomial_heap_node_t* next = NULL;

    while (curr) 
    {
        next = curr->bn_rsibling;
        curr->bn_rsibling = prev;
        prev = curr;
        curr = next;
    }

    *pnode = prev;
    return;
}

/**
 * @brief bn_node_new Create a new binomial node.
 *
 * @param[] pbn
 */
static void
binomial_heap_node_new (binomial_heap_node_t** pbn, void* key)
{
    binomial_heap_node_t* bn;

    if (!pbn) 
    {
        return;
    }
    bn   = calloc (1, sizeof(*bn));
    *pbn = bn;
    if (bn) 
    {
        bn->bn_lchild   = NULL;
        bn->bn_rsibling = NULL;
        bn->bn_key      = NULL;
        bn->bn_degree   = 0;
        bn->bn_key      = key;
    }
}

/**
 * @brief binomial_heap_node_destroy 
 *
 * @param[] bn
 * @param[] nitems
 */
static void
binomial_heap_node_destroy (binomial_heap_node_t* bn, int nitems)
{
    binomial_heap_node_t** stack;
    binomial_heap_node_t*  next;
    binomial_heap_node_t*  child;
    int32_t                top;

    if (!nitems)
    {
        return;
    }
    /* 
     * Allocate max. # of nodes in the stack. This may be a little too much
     * - but let us make sure. 
     */
    stack = calloc (nitems, sizeof(binomial_heap_node_t*));
    if (!stack) 
    {
        /* This is not white box tested. */

        return;
    }

    top = -1;
    do
    {
        if (bn->bn_rsibling) 
        {
            /* Break the right sibling link. */
            next = bn->bn_rsibling;
            bn->bn_rsibling = NULL;

            /* Push() into stack. */
            stack[++top] = bn;

            bn = next;
            continue;
        }

        if (bn->bn_lchild)
        {
            child            = bn->bn_lchild;
            child->bn_parent = NULL;
            bn->bn_lchild    = NULL;
            free (bn);
            --nitems;
            bn = child;
            continue;

        } 
        
        free (bn);
        --nitems;
        
        /* Check if stack is empty. */
        if (top == -1)
        {
            break;
        }
        /* Pop() from stack. */
        bn = stack[top--];

        continue;
    }
    while (1);

    /* Clean up auxilliary structures. */
    free (stack);
}

/**
 * @brief binomial_heap_merge 
 *
 * @param[] h1
 * @param[] h2
 * @param[] result
 */
static void
binomial_heap_merge (binomial_heap_t* h1, 
                     binomial_heap_t* h2, 
                     binomial_heap_t* result)
{
    binomial_heap_node_t* n1;
    binomial_heap_node_t* n2;
    binomial_heap_node_t* next;
    binomial_heap_node_t* lastnode;

    n1        = h1->bh_head;
    n2        = h2->bh_head;
    lastnode = NULL;
    while (n1 && n2)
    {
        if (n1->bn_degree <= n2->bn_degree)
        {
            next = n1->bn_rsibling;
            if (lastnode == NULL)
            {
                result->bh_head = n1;
            }
            else
            {
                lastnode->bn_rsibling = n1;
                n1->bn_rsibling       = NULL;
            }
            lastnode = n1;
            n1       = next;
        }
        else 
        {
            next = n2->bn_rsibling;
            if (lastnode == NULL)
            {
                result->bh_head = n2;
            }
            else
            {
                lastnode->bn_rsibling = n2;
                n2->bn_rsibling       = NULL;
            }
            lastnode = n2;
            n2       = next;
        }
    }

    /* remnant. */
    if (n1)
    {
        if (lastnode == NULL)
        {
            result->bh_head = n1;
        }
        else
        {
            lastnode->bn_rsibling = n1;
        }
    }
    /* remnant. */
    if (n2)
    {
        if (lastnode == NULL)
        {
            result->bh_head = n2;
        }
        else
        {
            lastnode->bn_rsibling = n2;
        }
    }
    return;
}

/**
 * @brief binomial_heap_union Combine two binomial heaps H1 and H2 to
 * produce and return a new binomial heap. Works by repeatedly linking
 * binomial trees whose roots have the same degree.
 *
 * @param[in] h1
 * @param[in] h2
 * @param[in] compare
 *
 * @return 
 */
static binomial_heap_t*
binomial_heap_union (binomial_heap_t* h1, 
                     binomial_heap_t* h2,
                     int              (*compare) (void*, void*))
{
    binomial_heap_t*      result;
    binomial_heap_node_t* curr;
    binomial_heap_node_t* prev;
    binomial_heap_node_t* next;
    int                   rc;

    binomial_heap_new (&result);
    binomial_heap_merge (h1, h2, result);
    if (result->bh_head == NULL) 
    {
        return result;
    }

    prev = NULL;
    curr = result->bh_head;
    next = curr->bn_rsibling;

    while (next != NULL)
    {
        if ( (curr->bn_degree != next->bn_degree) ||
             ( (next->bn_rsibling != NULL) &&
               (next->bn_rsibling->bn_degree == curr->bn_degree) ))
        {
            /* Cases 1 and 2. */
            prev = curr;
            curr = next;
        }
        else
        {
            rc = binomial_heap_node_compare (curr, next, compare); 
            if (rc <= 0)
            {
                /* Case 3 */
                curr->bn_rsibling = next->bn_rsibling;
                binomial_heap_node_link (next, curr);
            }
            else
            {
                /* Case 4 */
                if (prev == NULL)
                {
                    result->bh_head = next;
                }
                else
                {
                    prev->bn_rsibling = next;
                }
                binomial_heap_node_link (curr, next);
                curr = next;
            }
        }
        next = curr->bn_rsibling;
    }
    return result;
}


/****************************************************************************
 *                              APIS                                        *
 ****************************************************************************/

/**
 * @brief binomial_heap_new 
 *
 * @param[] pbh
 */
void
binomial_heap_new (binomial_heap_t** pbh)
{
    binomial_heap_t* bh;

    if (!pbh)
    {
        return;
    }
    bh   = calloc (1, sizeof(*bh));
    *pbh = bh;
    if (bh)
    {
        bh->bh_head  = NULL;
        bh->bh_count = 0;
    }
}


/**
 * @brief binomial_heap_destroy 
 *
 * @param[] pbh
 * @param[] destroy_all
 */
void 
binomial_heap_destroy (binomial_heap_t** pbh, bool destroy_all)
{
    binomial_heap_t*      bh;
    binomial_heap_node_t* bn;
    uint32_t nitems;

    if (!pbh) 
    {
        return;
    }
    bh     = *pbh;
    bn     = (*pbh)->bh_head;
    nitems = (*pbh)->bh_count;
    *pbh   = NULL;
    free (bh);

    if (destroy_all)
    {
        binomial_heap_node_destroy (bn, nitems);
    }
}

/**
 * @brief binomial_heap_min Find the binomial heap node with minimum key.
 *
 * @param[] bh
 * @param[] compare Comparator function; returns <0 if first key is smaller,
 * 0 if both keys are equal OR >0 if first key is larger.
 *
 * @return 
 */
binomial_heap_node_t*
binomial_heap_min (binomial_heap_t* bh, int (*compare) (void*, void*))
{
    binomial_heap_node_t* current;
    binomial_heap_node_t* minimum;
    int                   rc;

    current = bh->bh_head;
    minimum = current;
    while (current) 
    {
        rc = binomial_heap_node_compare (minimum, current, compare);
        if (rc > 0) 
        {
            minimum = current;
        }
        current = current->bn_rsibling;
    }
    return minimum;
}

/**
 * @brief binomial_heap_insert Insert an item into binomial heap
 *
 * @param[inout] bh   Binomial heap
 * @param[in] key     Item
 * @param[in] compare Comparator
 *
 * @return 
 */
binomial_heap_node_t*
binomial_heap_insert (binomial_heap_t** pbh, 
                      void*             key, 
                      int               (*compare) (void*, void*))
{
    binomial_heap_t*      bh;
    binomial_heap_t       h1;
    binomial_heap_node_t* node;
    uint32_t              nitems;

    if (!pbh)
    {
        return;
    }
    bh = *pbh;

    binomial_heap_node_new (&node, key);
    if (!node)
    {
        return;
    }
    h1.bh_head       = node;
    h1.bh_count      = 1; 
    nitems           = bh->bh_count;
    *pbh             = binomial_heap_union (bh, &h1, compare);
    (*pbh)->bh_count = nitems + 1;
    binomial_heap_destroy (&bh, false);
    return node;
}

/**
 * @brief binomial_heap_extract_min Extract minimum value element from
 * binomial heap
 *
 * @param[] pbh
 * @param[] compare
 *
 * @return 
 */
binomial_heap_node_t*
binomial_heap_extract_min (binomial_heap_t** pbh, 
                           int               (*compare) (void*, void*) )
{
    binomial_heap_node_t* current;
    binomial_heap_node_t* minimum;
    binomial_heap_node_t* min_prev;
    binomial_heap_node_t* prev;
    binomial_heap_t       h1;
    binomial_heap_t*      bh;
    uint32_t              nitems;
    int                   rc;

    bh       = *pbh;
    nitems   = bh->bh_count;
    current  = bh->bh_head;
    minimum  = current;
    min_prev = NULL;
    prev     = NULL;

    while (current) 
    {
        rc = binomial_heap_node_compare (minimum, current, compare);
        if (rc > 0) 
        {
            min_prev = prev;
            minimum = current;
        }
        prev = current;
        current = current->bn_rsibling;
    }

    if (minimum)
    {
        if (minimum == bh->bh_head)
        {
            bh->bh_head = minimum->bn_rsibling;
        }
        if (min_prev)
        {
            min_prev->bn_rsibling = minimum->bn_rsibling;
        }
        minimum->bn_rsibling = NULL;
    }

    /* 
     * Before doing this, we have broken the right sibling link from
     * minimum node and it'll be owned by either min_prev or head. Now, let
     * us take care of left child.
     */ 
    h1.bh_head = NULL;
    if (minimum->bn_lchild)
    {
        binomial_heap_node_sibling_sll_reverse__ (&minimum->bn_lchild);
        h1.bh_head         = minimum->bn_lchild;
        minimum->bn_lchild = NULL;
    }
    *pbh = binomial_heap_union (bh, &h1, compare);
    binomial_heap_destroy (&bh, false);

    (*pbh)->bh_count = nitems - 1;

    return (minimum);
}

/**
 * @brief binomial_heap_decrease_key Decrease key operation on a binomial
 * queue.
 *
 * @param[] bh
 * @param[] node
 * @param[] newkey
 * @param[] compare
 */
binomial_heap_node_t*
binomial_heap_decrease_key (binomial_heap_t*      bh,
                            binomial_heap_node_t* node,
                            void*                 newkey,
                            int                   (*compare) (void*, void*))
{
    binomial_heap_node_t* y;
    binomial_heap_node_t* z;
    binomial_heap_node_t tmp;
    void* temp;
    int rc;

    tmp.bn_key = newkey;
    rc = binomial_heap_node_compare (&tmp, node, compare);
        //(*compare) (newkey, node->bn_key);
    if (rc > 0) 
    {
        return;
    }

    node->bn_key = newkey;
    y = node;
    z = y->bn_parent;

    while (z && 
           (binomial_heap_node_compare (y, 
                                        z, 
                                        compare) < 0))
    {
        temp      = y->bn_key;
        y->bn_key = z->bn_key;
        z->bn_key = temp;
        y         = z;
        z         = y->bn_parent;
        node      = z;
    }

    return node;
}

/**
 * @brief binomial_heap_delete 
 *
 * @param[] pbh
 * @param[] node
 * @param[] compare
 */
binomial_heap_node_t*
binomial_heap_delete (binomial_heap_t**     pbh, 
                      binomial_heap_node_t* node,
                      int (*compare) (void*, void*) )
{
    void* oldkey;

    oldkey       = node->bn_key;
    binomial_heap_decrease_key (*pbh, node, NULL, compare);
    node         = binomial_heap_extract_min (pbh, compare);
    node->bn_key = oldkey;
    return node;
}

#ifdef ETEST
#include <stdio.h>
#include <time.h>

int intcomp (void* a, void* b)
{
//    printf ("Comparing (%d, %d)\n", *(int*)a, *(int*)b);
    return (*(int*)a - *(int*)b);
}

int main (int argc, char* argv[])
{
    binomial_heap_t*      heap;
    binomial_heap_node_t* node;
    binomial_heap_node_t* nlist [1000];
    int                   array [1000];
    int                   array1 [1000];
    int i;
    int j;

    srandom(time(NULL));


    ProfilerStart("./profiler.out");

    for (j = 0; j < 10; ++j)
    {
        binomial_heap_new (&heap);
        for (i = 0; i < 1000; ++i)
        {
            array [i] = random() % 1000;
        }


        for (i = 0; i < 1000; ++i)
        {
            binomial_heap_insert (&heap, &array [i],  intcomp);
        }

        for (i = 0; i < 1000; ++i)
        {
            node = binomial_heap_min (heap, intcomp);
            node = binomial_heap_extract_min (&heap, intcomp);
            if (node)
            {
                printf ("Extract-Min %d;\t", *(int*)(node->bn_key));
                fflush(stdout);
                free (node);
            }
        }
        binomial_heap_destroy (&heap, true);
        printf ("\n");
    }

    for (j = 0; j < 10; ++j)
    {
        binomial_heap_new (&heap);
        for (i = 0; i < 1000; ++i)
        {
            binomial_heap_insert (&heap, &array [i],  intcomp);
        }

        printf ("nitems(1) = %d\n", heap->bh_count);
        binomial_heap_destroy (&heap, true);
    }

    binomial_heap_new (&heap);
    for (i = 0; i < 1000; ++i)
    {
        nlist [i] = binomial_heap_insert (&heap, &array [i],  intcomp);
    }

    for (i = 0; i < 1000; ++i) 
    {
        j = array[i];
        array1[i] = array[i] - random() % 1000;
        printf ("Decrease-Key (old=%d, new=%d)\n", j, array[i]);
        binomial_heap_decrease_key (heap, nlist [i], &array1[i], intcomp);
    }
    for (i = 0; i < 1000; ++i)
    {
        node = binomial_heap_delete (&heap, nlist[i], intcomp);
        if (node)
        {
            if (node->bn_key)
                printf ("Delete %d;\n", *(int*)(node->bn_key));
            fflush(stdout);
            free (node);
        }
    }

    binomial_heap_destroy (&heap, true);

    ProfilerStop();

}
#endif
