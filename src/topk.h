#include<stdio.h>
#include<stdlib.h>

typedef struct ItemNode {
  int x; 
  int y; 
  int fre;
} ItemNode;

typedef struct subnode {
	int *S, size, last_index, sumeidx;
	struct subnode* next;
} subnode;
struct node
{
    int supp;	
	struct subnode *data;
    struct node *next;
}*head;
typedef struct candidate {
	int *pattern, patternsize, support, nodesize;
	ItemNode *nodelist;
	struct candidate* next;
} candidate;