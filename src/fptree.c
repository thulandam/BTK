/*----------------------------------------------------------------------
  File    : fptree.c
  Contents: frequent pattern tree management
  History : extend from algorithm of Christian Borgelt
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fptree.h"
#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definition
----------------------------------------------------------------------*/
#define BLKSIZE    6553         /* block size for memory management */



typedef struct {                /* --- structure for rec. search */
  int      supp;                /* minimum support (num. of trans.) */ 
  FPTREPFN *report;             /* report function for results */
  void     *data;               /* user data for report function */
  int      cnt;                 /* number of frequent item sets */
  int      items[1];            /* item vector for reporting */
} FPRS;                         /* (structure for rec. search) */

/*----------------------------------------------------------------------
  Main Functions
----------------------------------------------------------------------*/
int pre = 0;
int post = 0;
static FPTREE* _create (MEMSYS *mem, int cnt)
{                               /* --- create a base f.p. tree */
  FPTREE  *fpt;                 /* created frequent pattern tree */
  FPTLIST *list;                /* to traverse the node lists */

  assert(cnt > 0);              /* check the function arguments */
  fpt = (FPTREE*)malloc(sizeof(FPTREE) +(cnt-1) *sizeof(FPTLIST));
 
  if (!fpt) return NULL;        /* allocate the base structure */
  fpt->cnt = cnt;               /* and note the number of items */
  if (mem) fpt->mem = mem;      /* if a memory management is given, */
  else {                        /* simply store it, otherwise */
    fpt->mem = ms_create(sizeof(FPTNODE), BLKSIZE);
    if (!fpt->mem) { free(fpt); return NULL; }
  }                             /* allocate a memory system */
  for (list = fpt->lists +cnt; --cnt >= 0; ) {
    (--list)->cnt = 0; list->node = NULL; }
  return fpt;                   /* initialize the node lists and */
}  /* _create() */              /* return the created f.p. tree */

/*--------------------------------------------------------------------*/

static int _build (FPTREE *fpt, FPTNODE *parent,
                   TASET *taset, int lft, int rgt, int pos)
{                               /* --- recursively build f.p. tree */
  int     i, k;                 /* loop variable, buffer */
  int     item;                 /* to traverse the items at pos */
  FPTNODE *node;                /* created freq. pattern tree node */

  assert(fpt && taset && (pos >= 0)); /* check the function arguments */
  while ((lft <= rgt) && (tas_tsize(taset, lft) <= pos))
    lft++;                      /* skip trans. that are too short */
  if (lft > rgt) return 0;      /* check for an empty range */
  
  item = k = tas_tract(taset, i = rgt)[pos];    /* get first item */
  do { 		
								/* traverse the longer transactions */
    while (--i >= lft) {        /* while not at start of section */
      k = tas_tract(taset, i)[pos];
      if (k != item) break;     /* try to find a transaction */
    }                           /* with a different item */
    node = ms_alloc(fpt->mem);  /* create a new tree node */
    if (!node) return -1;       /* for the current item */

	node->item   = item;        /* and store the item */
    node->succ   = fpt->lists[item].node;
    fpt->lists[item].node = node;
    node->parent = parent;      /* insert the node into the item list */
    node->copy   = NULL;        /* and compute and sum the support */
    fpt->lists[item].cnt += node->cnt = rgt -i;
	node->preorder = ++pre;	
    if (_build(fpt, node, taset, i+1, rgt, pos+1) != 0)
      return -1;                /* build the child node recursively */
	node->posorder = ++post;
    item = k; rgt = i;          /* remove processed transaction from */
  } while (lft <= rgt);         /* the interval and note next item */
  return 0;                     /* return 'ok' */
}  /* _build() */

/*--------------------------------------------------------------------*/

FPTREE* fpt_create (TASET *taset)
{  
	/*
	1. is_cnt(tas_itemset(taset)): số 1-item thỏa mãn
	2. tas_cnt(taset): số transaction
	3. tas_itemset(taset): 
	
	/* --- create a freq. pattern tree */
  FPTREE *fpt;                  /* created frequent pattern tree */

  assert(taset);                /* check the function argument */
  fpt = _create(NULL, is_cnt(tas_itemset(taset)));
  if (!fpt) return NULL;        /* allocate a base f.p. tree */
  fpt->itemset = tas_itemset(taset);
  fpt->tra     = tas_cnt(taset);
  if ((fpt->tra > 0)            /* if there is at least one trans. */
  &&  (_build(fpt, NULL, taset, 0, fpt->tra -1, 0) != 0)) {
    fpt_delete(fpt); return NULL; }

  pre = 0;
  post = 0;
  return fpt;                   /* recursively build the frequent */
}  /* fpt_create() */           /* pattern tree and return it */

/*----------------------------------------------------------------------
The above function assumes that the items in each transaction in taset
are sorted and that the transactions are sorted accordingly.
----------------------------------------------------------------------*/

void fpt_delete (FPTREE *fpt)
{                               /* --- delete a freq. pattern tree */
  assert(fpt);                  /* check the function argument */
  ms_delete(fpt->mem);          /* delete the memory system */
  free(fpt);                    /* and the base structure */
}  /* fpt_delete() */

/*--------------------------------------------------------------------*/

int fpt_traverse (FPTREE *fpt, FPTNODE *parent)
{                               
  int     i;           
  FPTNODE *node;             
 
  for (i = 0; i < fpt->cnt; i++) {       
    for (node = fpt->lists[i].node; node; node = node->succ) {
      if (node->parent == parent)         
	  {
		  node->preorder = ++pre;
		  fpt_traverse(fpt, node);
		  node->posorder = ++post;
	  }
    }                     
  }  
  return 1;
}  /* fpt_traverse*/

#ifndef NDEBUG

void fpt_show (FPTREE *fpt, const char *title)
{                               /* --- show a freq. pattern tree */
  int     i;                    /* loop variable */
  FPTNODE *node;                /* to traverse the node lists */  
 
  printf("\n%s\n", title);      /* leave one line empty */
  for (i = 0; i < fpt->cnt; i++) {         /* traverse the items */
    printf("%s ", is_name(fpt->itemset, i));  /* print the item */
    printf("(%d):", fpt->lists[i].cnt);       /* and its support */
    for (node = fpt->lists[i].node; node; node = node->succ) {
      printf(" %d", node->cnt); /* print the node support */
      printf("[%d-%d]", node->preorder, node->posorder); /* print the node support */
      if (node->parent)         /* if the parent exists */
        printf("[%s]", is_name(fpt->itemset, node->parent->item));
    }                           /* print the item in the parent */
    printf("\n");               /* terminate the line */
  }                             /* after each node list */
}  /* fpt_show() */

#endif
