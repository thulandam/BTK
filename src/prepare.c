#include <stdio.h>
#include <stdlib.h>

#include <malloc.h>
#include <string.h>
#include <assert.h>
#include "fptree.h"
#include "prepare.h"
#define NODELISTSIZE  3553

void sume_show (SubsumeList *sume, ItemFirstList *first)
{
	int k, a;
	printf("\n");
	for (k = 0; k < sume->size; k++)
	{		 
		printf( "%s-%d-%d", sume->sume[k].itemname,first->lists[k].support,  sume->sume[k].subcnt);
		for (a = 0; a < sume->sume[k].subcnt; a++)
		{			
			printf("[%d]", sume->sume[k].subindex[a]);
		}
		printf("\n");
	}  
} 


void first_show (ItemFirstList *first)
{
	int k, a;
	printf("\n");
	for (k = 0; k < first->size; k++)
	{		 

		printf( "%s", first->lists[k].itemname);
		for (a = 0; a < first->lists[k].nodecnt; a++)
		{			
			printf("[%d-%d] \t", first->lists[k].nodelist[a].x,first->lists[k].nodelist[a].y);
		}
		printf("\n");		
	}  
}

int isInsubsume(int id, Subsume idxs){
	int i;							
	if (idxs.subcnt == 0)
		return 0;
	for (i=0; i < idxs.subcnt; i++) {
		if (id == idxs.subindex[i] )
			return 1;
	}
	return 0;
}
int checksubsume(ItemNode *a, int sizea, ItemNode *b, int sizeb) 
{

/* Kiểm tra xem b có phải thuộc trong subsume của a*/
/* b thuộc subsume a: order(b) > order(a) */					
	int c = 0, i=0, j=0;													/* b là cha, a là con */
	
	while ((i < sizeb) && (j < sizea)) 
	{
		if ((b[i].x < a[j].x) && (b[i].y > a[j].y))
		{
			j++;	
			//c++;
		}
		else
		{
			i++;
		}

	}
	if(j == sizea)
		return 1;
	return 0;
}
SubsumeList* find_subsume(ItemFirstList *first)
{
	int			i, k; 
	int			c = 0;
	int			m = 0;
	int			*item;
	int *items;
	int mem = 0;

	SubsumeList *sumelist;
	Subsume		*itemlist;

	sumelist = (SubsumeList*)malloc(sizeof(SubsumeList)) ;
	if (!sumelist) return NULL;	
	sumelist -> size	= first->size;
	itemlist = (Subsume*)malloc(sumelist -> size * sizeof(Subsume)) ;
	mem+=sumelist -> size * sizeof(Subsume);

	for (i = first->size-1; i >=0 ; i--)
	{
		itemlist[i].itemname = first->lists[i].itemname;
		c = 0;		
		item = (int*)malloc(first->size*sizeof(int));
		///////////////////**************************///////////////
		for (k = i - 1; k >= 0; k--)
		{
			if (checksubsume(first->lists[i].nodelist, first->lists[i].nodecnt, first->lists[k].nodelist, first->lists[k].nodecnt) == 1)
			{	
				item[c] = k;				
				c++;
			}
		}

		items = (int*)malloc(c*sizeof(int)); mem+=c*sizeof(int);
		memcpy(items,item,c*sizeof(int));
		itemlist[i].subindex = items;		
		itemlist[i].subcnt = c;	
	}
	sumelist ->sume	= itemlist;
	free(item);
	return sumelist;
}


ItemNode* node_intersection(double sumone,double sumtwo, double minsup, ItemNode *one,int sizeone, ItemNode *two, int sizetwo, int* fre, int* nodecount)
{

	ItemNode *result, *finalResult;											/* one là con, two là cha */
	int i ,j, s;
	int  remainone, remaintwo;
	*fre  = 0; remainone = sumone; remaintwo = sumtwo;
	
	*nodecount = 0;
	i = j = 0 ;
	s = 1;
		///////////////////**************************///////////////
	result = (ItemNode*) malloc((sizetwo)*sizeof(ItemNode));	
	while (i < sizeone && j < sizetwo)
	 {
		 if (one[i].x > two[j].x)
			 if (one[i].y < two[j].y)
			 {
				 if (*nodecount > 0 && result[*nodecount -1].x == two[j].x)
				 {
					result[*nodecount-1].fre += one[i].fre;
				 }
				 else
				 {		
				    result[*nodecount].x = two[j].x;
					result[*nodecount].y = two[j].y;
					result[*nodecount].fre = one[i].fre;
					*nodecount +=1;
				 }
				 *fre += one[i].fre;
				 i++;	
			 }
			 else
			 {
				 i++;
			 }
		 else
		 {
			 j++;
		 }		
		
	}

	finalResult = (ItemNode*)realloc(result,(*nodecount)*sizeof(ItemNode));
	result=NULL;
	return finalResult;

}

void printentry(FPPRINT print, SubsumeList *sumels)
{
	struct node *r;
	int rnk = count();
	r=head;	
	if(r==NULL)
	{
		return;
	}	
	while(r!=NULL)
	{
		rnk = rnk;
		printsub(print, r->data, r->supp, rnk, sumels);		
		r=r->next;
		rnk = rnk--;
	}	
}
int printsub(FPPRINT print, subnode *sub, int supp, int rnk, SubsumeList *sumels)
{
	while(sub!=NULL) {	
		print(sub->S, sub->size, rnk, supp, sumels->sume[sub->sumeidx].subcnt, sumels->sume[sub->sumeidx].subindex);
		sub=sub->next;		
	}	
	return 1;
}
int comp(const void *p, const void *q)
{
	return (*(int *)p - *(int *)q);
}
int checkOne(int *p, int size, int idx, candidate *n)
{
	int i = 0;
	candidate *temp;	
	temp=n;
	while(temp!=NULL)
	{		
		if (temp->patternsize == size - 1)
		{
			int flag=1;
			for (i = 0; i < idx; i++)
			{
				if (p[i] != temp->pattern[i])
				{
					flag=0;
					break;
				}
			}
			for (i = idx+1; i < size; i++)
			{
				if (p[i] != temp->pattern[i-1])
				{
					flag=0;
					break;
				}
			}
			if (flag == 1)
				return 1;
		}
		temp=temp->next;
	}	
	return 0;
}
int checkSubsetInCandidate(int *p, int size, candidate *n)
{
	int i = 0;	
	for (i = 0; i < size; i++)
	{
		if (checkOne(p, size, i, n) == 0)
			return 0;
	}
	return 1;
}
subnode* validatePattern(int *one,int sizeone, int *two, int sizetwo, SubsumeList *sumels, candidate *n){
	int i = 0, minidx, subid;
	subnode *ret = NULL;
	if ((sizeone != sizetwo) || (sizeone < 1)) return NULL;
	if (one[0] < two[0])
	{
		minidx = one[0];
		subid = two[0];
		if (isInsubsume(minidx, sumels->sume[two[sizeone-1]]) == 1) return NULL;
	}
	else
	{
		minidx = two[0];
		subid = one[0];
		if (isInsubsume(minidx, sumels->sume[one[sizeone-1]]) == 1) return NULL;
	}
	
	for (i = 1; i < sizeone; i++)
	{
		if (one[i] != two[i]) return NULL;
	}

	ret =(subnode*) malloc(sizeof(subnode));
	ret->size = sizeone + 1;	
	ret->next = NULL;
	ret->S =(int*) malloc((sizeone+1)*sizeof(int));
	
	ret->S[0] = minidx;
	ret->S[1] = subid;
	for (i = 1; i < sizeone; i++)
	{
		ret->S[i+1] = one[i];
	}
	
	if (sizeone >1)
		ret->sumeidx = one[sizeone-1];
	else
		ret->sumeidx = subid;
	return ret;
}

void Free_Candidate(candidate* n)
{
	if (n == NULL)
	{
		free(n);
		return;
	}
	else
	{		
		subnode *Temp = n->next;
		candidate *_free = n;
		free(_free->pattern);
		free(_free->nodelist);
		free(_free);

		while(Temp != NULL)
		{
			_free = Temp;
			Temp = _free->next;
			if(_free->pattern != NULL){				
				free(_free->pattern);
			}  
			if(_free->nodelist != NULL){				
				free(_free->nodelist);
			}	
			
			free(_free);
		}
		_free = n = Temp;
	}  
}


void fi_searchtopk (ItemFirstList *first, SubsumeList *sumels, int top, FPPRINT print)
{   
	int i, j = 0, numofcan,numofcan2;
	int total = 0, entrycount=0;
	int mintopk = 0;
	int isentryfull = 0;
	int	crosssupport = 0;
	int	crossnodecount = 0;
	ItemNode	*intersectionnode = NULL;	
	candidate *cani, *caniplus1,*temp;
	subnode *stack = NULL;
	candidate *n;
	candidate *tail;	
	cani= NULL;

	numofcan = 0;	
	for (i = 0; i < first->size; i++)	
	{			
		if (first->lists[i].support >= mintopk)
		{
			int *pattern  = (int*) malloc(sizeof(int));
			subnode *item = NULL;
			item = createNode(item,i);				
			entrycount = insert(first->lists[i].support, item);		
			if (entrycount >= top)
			{
				isentryfull = 1;
				if (entrycount > top)
					removehead();
			}
			pattern[0] = i;
			cani = createCandidate(cani, pattern, 1, first->lists[i].support, first->lists[i].nodecnt, first->lists[i].nodelist);

			if ((isentryfull == 1) && (head != NULL))
				mintopk = head->supp;
			free(pattern);
		}
		else if (isentryfull == 1) break;
	}	
	printf("Num of 1-items %d \n", i);	
	while (i >1)
	{				
		
		n=cani;
		caniplus1 = NULL;
		i=0;		
		while(n !=NULL)
		{				//tail = can[i];		
			tail = n->next;
			while(tail!=NULL)
			{		
						
				subnode *item = NULL;
				item = validatePattern(n->pattern, n->patternsize, tail->pattern, tail->patternsize, sumels, cani);				
				if ((item != NULL)) 
				{						
					intersectionnode = NULL;
					crosssupport = 0;
					crossnodecount = 0;
					if (n->pattern[0] < tail->pattern[0])
						intersectionnode = node_intersection(tail->support, n->support,  mintopk, tail->nodelist,tail->nodesize,
						n->nodelist,n->nodesize, &crosssupport, &crossnodecount);
					else
						intersectionnode = node_intersection(n->support, tail->support, mintopk, n->nodelist,n->nodesize,
						tail->nodelist,tail->nodesize, &crosssupport, &crossnodecount);		

					if (intersectionnode !=NULL) numofcan++;					
					if (intersectionnode !=NULL && crosssupport >= mintopk) //satisfied
					{
						entrycount = insert(crosssupport, item);
						i++;
						if (entrycount > top)
						{								
							isentryfull = 1;
							removehead();										
						}								
						caniplus1 = createCandidate(caniplus1, item->S, item->size, crosssupport, crossnodecount, intersectionnode);

						if ((isentryfull == 1) && (head != NULL))
						{
							mintopk = head->supp;									
						}						
					}
								
				}	
				tail=tail->next;
			}			
			n=n->next;
		}		
		cani=caniplus1;	
	}

	printf("\nNumber of candidates: %d\n",numofcan);
	printentry(print, sumels);	
}  /* _search() */


void ifirst_free (ItemFirstList *f) {
	if (f != NULL) {
		free (f->lists);
		free (f);
	}
}

void subsume_free (SubsumeList *sub) {
	if (sub != NULL) {
		free (sub->sume);
		free (sub);
	}
}


/* Tạo danh sách I-1 item với node list tương ứng */
ItemFirstList* itemfirstlist_create(FPTREE *fpt)
{ 	
	int				i, c, cnt;
	ItemFirstList	*first;    
	ItemNode		*nodelist;
	FPTNODE			*node;  
	assert(fpt);  
	cnt = fpt->cnt;
	first			=	(ItemFirstList*)malloc(sizeof(ItemFirstList) + (cnt) * sizeof(ItemFirst) ) ;
	if (!first) return NULL;
	first -> size	=	cnt;
	first->mem = ms_create(sizeof(ItemNode), NODELISTSIZE);
	if (!first->mem) { free(first); return NULL; }

	for (i = 0; i < fpt->cnt; i++) {
		first->lists[i].support		=	fpt->lists[i].cnt;
		first->lists[i].itemname	=   is_name(fpt->itemset, i);	
		c = 0	;
		for (node = fpt->lists[i].node; node; node = node->succ) {			
			c++;				
		} 
		nodelist = (ItemNode*)malloc(c * sizeof(ItemNode)) ;
		c = 0	;
		for (node = fpt->lists[i].node; node; node = node->succ) {	
			nodelist[c].x		=   node->preorder;
			nodelist[c].y		=   node->posorder;
			nodelist[c].fre		=   node->cnt;
			c++;				
		} 
		free(node);
		first->lists[i].nodelist	=	nodelist;
		first->lists[i].nodecnt		=	c;
	}  

	return first;   
}  /* ItemFirstList_create() */  

