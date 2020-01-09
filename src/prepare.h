#include "memsys.h"
#include "topk.h"
typedef int FPPRINT (int *ids, int cnt, int rank, int supp, int sumecount, int *sumeidx);

typedef struct Subsume {
	char		*itemname;
	int			subcnt;
	int			*subindex; 
} Subsume;

typedef struct SubsumeList {
	int		size;
	Subsume *sume;
} SubsumeList;

typedef struct ItemFirst {	   
  char		*itemname;
  int		support;
  int		nodecnt;   
  ItemNode	*nodelist;
} ItemFirst;

typedef struct ItemFirstList {
  int		size;  
  MEMSYS  *mem;
  ItemFirst lists[1];
} ItemFirstList;
node;

typedef struct ItemTail {	   
  int		idx;
  int		nodecnt;   
  int		support;  
  ItemNode	*nodelist;
} ItemTail;

