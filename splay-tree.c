/****************************************************************************
 *     Copyright (C) [2012 - ] Harish Raghuveer - All Rights Reserved       *
 *                                                                          *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY   *
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE      *
 * IMPLIED WARRANTIES OF MECHANTABILITY AND/OR FITNESS FOR A PARTICULAR     *
 * PURPOSE.                                                                 *
 ****************************************************************************/

/**
 * @file    splay-tree.c
 *
 * @brief   This file implements splay tree operations. Please note that
 *          splay trees can store only unique key values. The self
 *          adjustment nature doesn't go very well with duplicate keys.
 *
 *          Nice applications for splay trees are the ones that needs a fast
 *          priority queue implementations (such as Dijkstra's SPF, Prim's
 *          MST etc.).
 *
 * @author  Harish Raghuveer
 *
 * @version 1.0
 *
 * @date    2014-01-19
 */

/* Standard headers */

#include <stdlib.h>
#include <time.h>
#include <assert.h>

/* Our headers */

#include "base-types.h"
#include "bst-template.h"
#include "splay-tree.h"

/****************************************************************************
 *                    PRIVATE variables and functions                       *
 ****************************************************************************/

static splay_node_t splay_nil = 
{ 
    .splay_parent = NULL,
    .splay_left   = NULL,
    .splay_right  = NULL,
    .splay_item   = NULL,
};

static splay_node_t* splay_NILP = &splay_nil;

splay_node_t* 
splay_nilp (void)
{
    return splay_NILP;
}

/**
 * @brief This is a straightforward implementation of simple top-down
 *        splaying.
 *
 * @param root Current root of the splay tree
 * @param nilp Sentinel NIL
 * @param key  Key value at which tree is splayed.
 * @param comp Comparator
 *
 * @return Pointer to the new root of the splayed tree
 */
static splay_node_t*
splay (splay_node_t*      root, 
       splay_node_t*      nilp, 
       void*              key, 
       splay_comparator_t comp)
{
    splay_node_t  dummy;
    splay_node_t* left;
    splay_node_t* right;
    splay_node_t* next;
    splay_node_t* node;
    int           rc;

    if (root == nilp)
        return root;

    left  = &dummy;
    right = &dummy;
    

    for (node = root; ; )
    {
        rc = (*comp) (key, node->splay_item);
        if (rc < 0)
        {
            if (node->splay_left == nilp)
                break;
            if ((*comp) (key, node->splay_left->splay_item) < 0)
            {
                /* Rotate right */

                next              = node->splay_left;
                node->splay_left  = next->splay_right;
                next->splay_right = node;
                node              = next;

                if (node->splay_left == nilp)
                    break;
            }

            /* link right */

            right->splay_left = node;
            right             = node;
            node              = node->splay_left;
        }
        else if (rc > 0)
        {
            if (node->splay_right == nilp)
                break;

            if ((*comp) (key, node->splay_right->splay_item) > 0)
            {
                /* Rotate left */

                next              = node->splay_right;
                node->splay_right = next->splay_left;
                next->splay_left  = node;
                node              = next;

                if (node->splay_right == nilp)
                    break;
            }

            /* Link left */

            left->splay_right = node;
            left              = node;
            node              = node->splay_right;
        }
        else
        {
            break;
        }
    }

    left->splay_right = node->splay_left;
    right->splay_left = node->splay_right;
    node->splay_left  = dummy.splay_right;
    node->splay_right = dummy.splay_left;

    RETVAL(node);
}

/****************************************************************************
 *                       PUBLIC variables and functions                     *
 ****************************************************************************/

/**
 * @brief        Intialize a splay tree node.
 *
 * @param spnode Pointer to the node to be initialized
 * @param item   Item to store in the splay tree node
 */
void
splay_node_init (splay_node_t* spnode, void* item)
{
    spnode->splay_left   = splay_NILP;
    spnode->splay_right  = splay_NILP;
    if (item)
        spnode->splay_item   = item;
}

/**
 * @brief Allocate a splay tree node dynamically
 *
 * @param splay_nodep POinter to the location where splay tree node is to be
 *                    store.
 * @param data        Item to be stored.
 *
 * @return            ROK on success; RFAIL upon failure
 */
rc_t
splay_node_new (splay_node_t** splay_nodep, void* data)
{
    splay_node_t* spnode;

    assert (splay_nodep);
    spnode = calloc (1, sizeof(*spnode));
    if (spnode)
        splay_node_init (spnode, data);
    *splay_nodep = spnode;

    RETVAL ((spnode) ? ROK : RFAIL);
}

/**
 * @brief Deallocate a splay tree node.
 *
 * @param splay_nodep
 */
void
splay_node_delete (splay_node_t** splay_nodep)
{
    splay_node_t* spnode;

    assert (splay_nodep);
    spnode      = *splay_nodep;
    splay_nodep = NULL;
    if (spnode)
        free (spnode);
    RETVOID;
}

/**
 * @brief             Insert an item into a splay tree.
 *
 * @param splay_rootp Pointer to the address of splay tree's root node.
 * @param nilp        Sentinel NIL node.
 * @param splay_comp  Lexicographic comparator function
 * @param node        Splay node containing the pointer to item/key.
 *                    Insertion of splay node containing no item (NULL) is
 *                    not allowed.
 *
 * @return            Valid node on success; NIL node on failure.
 */
splay_node_t*
splay_insert (splay_node_t**     splay_rootp,
              splay_node_t*      nilp, 
              splay_comparator_t splay_comp,
              splay_node_t*      node)
              
{
    void*         item;
    splay_node_t* root;
    int           rc;

    assert          (node);
    splay_node_init (node, NULL);

    if (*splay_rootp == nilp) 
    {
        *splay_rootp = node;
        RETVAL (node);
    }

    item = node->splay_item;
    assert (item);

    root = *splay_rootp;
    root = splay (root, nilp, item, splay_comp);
    rc   = (*splay_comp) (item, 
                          root->splay_item);
    if (rc < 0)
    {
        node->splay_left  = root->splay_left;
        node->splay_right = root;
        root->splay_left  = nilp;
        *splay_rootp      = node;
    } 
    else if (rc > 0)
    {
        node->splay_right = root->splay_right;
        node->splay_left  = root;
        root->splay_right = nilp;
        *splay_rootp      = node;
    }
    else
    {
        *splay_rootp = root;
        //splay_node_delete (&node);
        node         = nilp;
    }

    RETVAL (node);
}


/**
 * @brief             Remove an item from splay tree.
 *
 * @param splay_rootp Pointer to the address of splay tree.
 * @param nilp        Sentinal NIL to the splay tree
 * @param splay_comp  Comparator
 * @param node        Splay node which is to be removed.
 *
 * @return            If the node is in the tree, it is removed and
 *                    returned. Or the sentinel NIL is returned.
 */
splay_node_t*
splay_remove (splay_node_t**     splay_rootp, 
              splay_node_t*      nilp, 
              splay_comparator_t splay_comp,
              splay_node_t*      node)
{
    void*         item;
    splay_node_t* root;
    splay_node_t* curr;
    int           rc;

    item  = node->splay_item;
    assert (item);

    root = *splay_rootp;
    if (root == nilp)
        RETVAL (nilp);

    root = splay (root, nilp, item, splay_comp);
    rc   = (*splay_comp) (item, root->splay_item);
    if (rc == 0)
    {
        if (root->splay_left == nilp)
        {
            curr = root->splay_right;
        }
        else
        {
            curr = splay (root->splay_left, nilp, item, splay_comp);
            curr->splay_right = root->splay_right;
        }
        *splay_rootp = curr;
        splay_node_init (root, NULL);
        RETVAL          (root);
    }

    *splay_rootp = root;
    RETVAL (nilp);
}

/**
 * @brief Find if the splay node exists in a splay tree based on the key
 *        stored in it. The net effect is that the node splays up the tree
 *        and subsequent accesses will be fast
 *
 * @param splay_rootp Pointer to the address of splay tree.
 * @param nilp        Sentinal NIL to the splay tree
 * @param splay_comp  Comparator
 * @param node        Splay node which is to be removed.
 *
 * @return            Pointer to the splay node if the item stored in it
 *                    exists; NIL otherwise
 */
splay_node_t*
splay_find_node (splay_node_t**     splay_rootp, 
                 splay_node_t*      nilp, 
                 splay_comparator_t splay_comp,
                 splay_node_t*      node)
{
    void*         item;
    splay_node_t* root;

    item         = node->splay_item;
    root         = *splay_rootp;
    if (root == nilp)
        RETVAL (nilp);
    root         = splay (root, nilp, item, splay_comp);
    *splay_rootp = root;

    RETVAL ( ((*splay_comp) (item, 
                             root->splay_item) == 0) ? root : nilp);
}

/**
 * @brief Check if the key value stored in 'item' argument exists in a splay
 *        tree.
 *
 * @param splay_rootp Pointer to the address of splay tree.
 * @param nilp        Sentinal NIL to the splay tree
 * @param splay_comp  Comparator
 * @param item        The 'key' to be looked up.
 *
 * @return            Pointer to the splay node if the item stored in it
 *                    exists; NIL otherwise
 */
splay_node_t*
splay_find (splay_node_t**     splay_rootp, 
            splay_node_t*      nilp, 
            splay_comparator_t splay_comp,
            void*              item)
{
    splay_node_t* root;

    root = *splay_rootp;
    if (root == nilp)
        RETVAL (nilp);
    root         = splay (root, nilp, item, splay_comp);
    *splay_rootp = root;

    RETVAL ( ((*splay_comp) (item, 
                             root->splay_item) == 0) ? root : nilp);
}

/**
 * @brief Implements the 'delete-min' priority queue operation using a splay
 *        tree.
 *
 * @param splay_rootp Pointer to root address
 * @param nilp        The sentinel NIL
 * @param splay_comp  Comparator
 *
 * @return            Pointer to the splay node if the item stored in it
 *                    exists; NIL otherwise
 */
splay_node_t*
splay_delete_min (splay_node_t**     splay_rootp, 
                  splay_node_t*      nilp, 
                  splay_comparator_t splay_comp)
{
    splay_node_t* curr;
    splay_node_t* node;

    node = nilp;
    curr = *splay_rootp;
    while (curr != nilp)
    {
        node = curr;
        curr = curr->splay_left;
    }
    if (node != nilp) {
        node = splay_remove (splay_rootp, nilp, splay_comp, node);
    }
    RETVAL (node);
}

/**
 * @brief The 'Decrease-key' priority queue operation.
 *
 * @param splay_rootp Pointer to root address
 * @param nilp        The sentinel NIL
 * @param splay_comp  Comparator
 * @param node        Splay node.
 * @param newkey      The 'new' decremented key value.
 *
 * @return            Pointer to the splay node if the item stored in it
 *                    exists; NIL otherwise
 *
 * @sa                This API is not tested yet.
 */
splay_node_t*
splay_decrease_key (splay_node_t**     splay_rootp,
                    splay_node_t*      nilp,
                    splay_comparator_t splay_comp,
                    splay_node_t*      node,
                    void*              newkey)
{
    splay_node_t* curr;

    curr = splay_remove (splay_rootp, nilp, splay_comp, node);
    if (curr != nilp)
    {
        node->splay_item = newkey;
        RETVAL (splay_insert (splay_rootp, nilp, splay_comp, node));
    }
    RETVAL(nilp);
}


/****************************************************************************
 *                               API TEST *                                 *
 ****************************************************************************/

#ifdef ETEST

int intcomp (void* a, void* b)
{
    RETVAL (*(int*)a - *(int*)b);
}

int main (int argc, char* argv [])
{
    splay_node_t* root;
    splay_node_t* node;
    splay_node_t* node1;
    int *ilist;
    int count;
    int i;
    unsigned int   seed;

    count = 0;
    if (argc > 1) 
    {
        count = atoi (argv[1]);
    }
    if (count == 0)
        count = 32;
    printf ("count %d\n", count);

    seed  = time(NULL);
    root  = splay_NILP;
    ilist = calloc (count, sizeof(int));

    for (i = 0; i < count; ++i)
    {
        ilist [i] = rand_r (&seed) % 1000;
        //printf ("%d ", ilist [i]);
    }
    printf ("\n");

    for (i = 0; i < count; ++i)
    {
        splay_node_new (&node, (void*) &ilist [i]);
        node1 = splay_insert (&root, splay_NILP, intcomp, node);
        if (node1 == splay_NILP)
        {
            splay_node_delete (&node);
        }
        else
            assert (node1 == node);
        //printf ("Insert[%d] %p\n", ilist [i], node);
    }

    for (i = 0; i < count; ++i)
    {
        node = splay_find (&root, splay_NILP, intcomp, &ilist [i]);
        if (node != splay_NILP)
        {
            //printf ("found %d\n", *(int*)node->splay_item);
            node1 = splay_find_node (&root, splay_NILP, intcomp, node);
            assert (node1 == node);
        }
#if 0
        else
        {
            printf ("Not found %d\n", ilist[i]);
        }
#endif
    }

    for (i = 0; i < count; ++i)
    {
        node = splay_find (&root, splay_NILP, intcomp, &ilist [i]);
        //printf ("Remove[%d] %p\n", ilist [i], node);
        if (node != splay_NILP) {
            node = splay_remove (&root, splay_NILP, intcomp, node);
            splay_node_delete (&node);
        } 
#if 0
        else 
        {
            printf ("Cannot remove %d\n", ilist [i]);
        }
#endif
    }

    for (i = 0; i < count; ++i)
    {
        splay_node_new (&node, (void*) &ilist [i]);
        if (splay_insert (&root, 
                          splay_NILP, 
                          intcomp, 
                          node) == splay_NILP)
        {
            //printf ("Duplicate. Freeing..\n");
            splay_node_delete (&node);
        }
    }

    for (i = 0; i < count; ++i)
    {
        node = splay_delete_min (&root, splay_NILP, intcomp);
        if (node != splay_NILP)
        {
            //printf ("Delete-min %d\n", *(int*)node->splay_item);
            splay_node_delete (&node);
        }
    }


    free (ilist);
    return 0;
}

#endif
