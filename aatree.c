/***************************************************************************
 *     Copyright (C) [2012 - ] Harish Raghuveer - All Rights Reserved      *
 *                                                                         *
 *   THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF    *
 *   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO   *
 *   THE IMPLIED WARRANTIES OF MECHANTABILITY AND/OR FITNESS FOR A         *
 *   PARTICULAR PURPOSE.                                                   *
 ***************************************************************************/
/***************************************************************************
 * @file    aatree.c
 * @brief 
 * @author  Harish Raghuveer
 * @version 1.0
 * @date    2016-07-07
 ***************************************************************************/


#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "aatree.h"

static const aa_node_t
aa_nil_node =
{
    {
        (aa_node_t*) &aa_nil_node, 
        (aa_node_t*) &aa_nil_node 
    },
    0,
    0
};

static aa_node_t *AA_NIL = (aa_node_t *) &aa_nil_node;

/**************************************************************************
 *                          STATIC FUNCTIONS                              *
 **************************************************************************/

static inline void
aan_new (aa_node_t **pn, 
         uintptr_t   data, 
         uint8_t     lvl)
{
    aa_node_t *n;

    n = calloc (1, sizeof(*n));
    if (n)
    {
        n->aa_level = lvl;
        n->aa_data  = data;
        LLINK(n)    = AA_NIL;
        RLINK(n)    = AA_NIL;
    }

    *pn = n;
}

static inline void
aan_delete (aa_node_t **pn)
{
    if (!pn || !*pn)
        return;
    free(*pn);
    pn = NULL;
}

static inline void
skew (aa_node_t **rootp)
{
    aa_node_t *save;

    if ((LLINK(*rootp)->aa_level == (*rootp)->aa_level) &&
        ((*rootp)->aa_level != 0))
    {
        save          = LLINK(*rootp);
        LLINK(*rootp) = RLINK(save);
        RLINK(save)   = *rootp;
        *rootp        = save;
    }
}

static inline void
split (aa_node_t **rootp)
{
    aa_node_t *save;


    if ((RLINK(RLINK(*rootp))->aa_level == (*rootp)->aa_level) &&
        ((*rootp)->aa_level != 0))
    {
       
        save          = RLINK(*rootp);
        RLINK(*rootp) = LLINK(save);
        LLINK(save)   = *rootp;
        *rootp        = save;
        ++(*rootp)->aa_level;
    }
}

static void
aa_int_insert (aa_node_t **rootp,
               uintptr_t   data,
               aa_cmp_t    cmp)
{
    aa_node_t *up [256];
    aa_node_t *it;
    int        top;
    int        dir;
    int        rc;

    if (*rootp == AA_NIL) 
    {
        aan_new (rootp, data, 1);
        return;
    }

    it  = *rootp;
    top = 0;
    dir = 0;

    for (;;)
    {
        up[top++] = it;
        rc        = (*cmp)((void *)it->aa_data, (void *)data);
        dir       = (rc < 0);

        if (it->aa_link[dir] == AA_NIL)
            break;

        it = (rc < 0) ? RLINK(it) : LLINK(it);
    }

    aan_new (&it->aa_link[dir], data, 1);

    while (--top >= 0)
    {
        if (top != 0)
        {
            rc = (*cmp)((void *) RLINK(up [top - 1])->aa_data,
                        (void *) up [top]->aa_data);
            dir = (rc == 0);
        }

        skew  (&up [top]);
        split (&up [top]);

        if (top != 0)
            RLINK(up [top - 1]) = up [top];
        else
        *rootp = up [top];
    }
}

rc_t
aa_int_find (aa_node_t  *root,
             void       *data,
             aa_cmp_t    cmp,
             aa_node_t **nodep)
{
    aa_node_t *node;
    int        rc;

    node = root;
    while (1)
    {                                               
        if (node == AA_NIL)                     
            break;                                  
        rc = (*cmp)((void *) data, 
                    (void *) node->aa_data); 
        if (rc == 0)
            break;
        node = ((rc < 0)        ? 
                    LLINK(node) : 
                    RLINK(node));   
    }                                               
    *nodep = node;
    return ((node != AA_NIL) ? ROK : RFAIL); 
}

static void
aa_int_remove (aa_node_t **rootp,
               aa_node_t  *node,
               aa_cmp_t    cmp)
{
    aa_node_t *up [256];
    aa_node_t *it;
    aa_node_t *heir;
    aa_node_t *prev;
    int        top;
    int        dir;
    int        dir2;
    int        rc;

    if (*rootp == AA_NIL)
        return;

    it  = *rootp;
    top = 0;
    dir = 0;

    for (::)
    {
        up [top++] = it;
        if (it == AA_NIL)
            return;

        rc = (*cmp)(node->aa_data, it->aa_data);
        if (rc == 0)
            break;
        dir = (rc > 0);
        it  = it->aa_link [dir];
    }

    if (LLINK(it) == AA_NIL || 
        RLINK(it) == AA_NIL)
    {
        dir2 = (LLINK(it) == AA_NIL);

        if (--top != 0)
            up [top - 1]->aa_link [dir] = it->aa_link [dir2];
        else
            *rootp = RLINK(it);
    }
    else
    {
        heir = RLINK(it);
        prev = it;

        while (LLINK(heir) != AA_NIL)
        {
            up[top++] = heir;
            prev      = heir;
            heir      = LLINK(heir);
        }

        /* This is not good: handes must be immutable! */
        it->aa_data                 = heir->aa_data;
        prev->aa_link[(prev == it)] = RLINK(heir);
    }

    while (--top >= 0)
    {

    }
}

int
aa_int_height (aa_node_t  *root)
{
    int hl, hr;

    if (root != AA_NIL)
    {
        hl = aa_int_height (LLINK(root));
        hr = aa_int_height (RLINK(root));
        if (hl >= hr)
            return hl + 1;
        else 
            return hr + 1;

    }
    return 0;
}
/**************************************************************************
 *                          EXTERN FUNCTIONS                              *
 **************************************************************************/

#ifdef ETEST

int
intcmp (void* a, void* b)
{
    return ((int) a - (int) b);
}

void
intprint (void *cb __attribute__((unused)), aa_node_t *n)
{
    printf("%d ", (int) n->aa_data);
}

int
main (int argc, char **argv)
{
    aa_node_t *n = AA_NIL;
    aa_node_t *p = AA_NIL;
    int        nfound = 0;
    int        found  = 0;
    int        i;

    for (i = 0; i <  4000000; ++i) {
        aa_int_insert (&n, i, intcmp);
        if (n == AA_NIL)
            printf ("insert(%d) failed!\n", (i+1));
    }

   for (i = 0; i < 4000000; ++i)
    {
        aa_int_find (n, (void*) i, intcmp, &p);
        if (p == AA_NIL) {
            printf ("%d not foubd!\n", i);
            ++nfound;
        }
        else
            ++found;
    }

    printf ("found %d; missed %d\n", found, nfound);
    printf ("root %d\n", n->aa_data);
    printf ("Height of AA-tree is %d\n",
            aa_int_height (n));

    return 0;
}
#endif
