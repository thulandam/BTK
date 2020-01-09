/*----------------------------------------------------------------------
  File    : fptree.h
  Contents: frequent pattern tree management
  History : extend from algorithm of Christian Borgelt
----------------------------------------------------------------------*/
#ifndef __FPTREE__
#define __FPTREE__
#include "memsys.h"
#include "tract.h"

/*----------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------*/
typedef struct _fptnode {       /* --- frequent pattern tree node --- */
  struct _fptnode *succ;        /* pointer to successor (same item) */
  struct _fptnode *parent;      /* pointer to parent node */
  struct _fptnode *copy;        /* auxiliary pointer to copy */
  int             item;         /* corresponding item */
  int			  preorder;
  int			  posorder;
  int             cnt;          /* number of transactions */
} FPTNODE;                      /* (frequent pattern tree node) */

typedef struct {                /* --- f.p. tree node list --- */
  int     cnt;                  /* number of transactions */
  FPTNODE *node;                /* pointer to first node in list */
} FPTLIST;                      /* (f.p. tree node list) */

typedef struct {                /* --- frequent pattern tree --- */
  ITEMSET *itemset;             /* underlying item set */
  int     cnt;                  /* number of items / node lists */
  int     tra;                  /* number of transactions */
  
  MEMSYS  *mem;                 /* memory management system */
  FPTLIST lists[1];             /* vector of one node list per item */
} FPTREE;                       /* (frequent pattern tree) */
typedef int FPTREPFN (int *ids, int cnt, int supp, void *data);
                                /* report function */

/*----------------------------------------------------------------------
  Main Functions
----------------------------------------------------------------------*/
extern FPTREE* fpt_create  (TASET *taset);
extern void    fpt_delete  (FPTREE *fpt);
extern void    fpt_itemset (FPTREE *fpt);

extern int    fpt_traverse (FPTREE *fpt,FPTNODE *parent);

#ifndef NDEBUG
extern void    fpt_show    (FPTREE *fpt, const char *title);
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define fpt_itemset(t)     ((t)->itemset)

#endif
