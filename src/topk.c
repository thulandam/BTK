#include "topk.h"
#define CANNODESIZE  6553 
int count()
{
    struct node *n;
    int c=0;
    n=head;
    while(n!=NULL)
    {
		n=n->next;
		c++;
    }
    return c;
}

void append(int num, subnode *data)
{
    struct node *temp,*right;
    temp= (struct node *)malloc(sizeof(struct node));
    temp->supp=num;
	temp->data=data;
    right=(struct node *)head;
    while(right->next != NULL)
	right=right->next;
    right->next =temp;
    right=temp;
    right->next=NULL;
}

void add(int num, subnode *data)
{
    struct node *temp;
    temp=(struct node *)malloc(sizeof(struct node));
    temp->supp=num;
	temp->data=data;
    if (head== NULL)
    {
		head=temp;
		head->next=NULL;
    }
    else
    {
		temp->next=head;
		head=temp;
    }
}

void addafter(int num, subnode *data, int loc)
{
    int i;
    struct node *temp,*left,*right;
    right=head;
    for(i=1;i<loc;i++)
    {
		left=right;
		right=right->next;
    }
    temp=(struct node *)malloc(sizeof(struct node));
    temp->supp=num;
	temp->data=data;
    left->next=temp;
    left=temp;
    left->next=right;
    return;
}

void Free_All(subnode *p)
{	
	if (p == NULL)
	{
		free(p);
		return;
	}
	else
	{
		subnode *Temp = p->next;
		subnode *_free = p;
		
		free(_free->S);
		free(_free);
	
		while(Temp != NULL)
		{
			_free = Temp;
			Temp = _free->next;
			if(_free->S != NULL){
				free(_free->S);
			}  
	       
			free(_free);
		}
		_free = p = Temp;
	}  
}
void removehead()
{
    struct node *temp;
    int c = 0;
    temp=head;
    if(temp!=NULL)
    {	
		head=temp->next;
		if (temp->data != NULL)
			Free_All(temp->data);
		free(temp);
		
    }
}

int insert(int num, subnode *sdata)
{
	int c=0;
	int lscount=0;
    int exist=0;
    struct node *temp;
	subnode *data=(subnode*)malloc(sizeof(subnode));
    temp=head;
	lscount = count();
	
	memcpy(data, sdata, sizeof(subnode));	
    if(temp==NULL)
    {
		add(num, data);
		return lscount + 1;
    }
    else
    {
		while(temp!=NULL)
		{
			if(temp->supp == num)
			{
				exist = 1;
				if (temp->data == NULL)
				{
					temp->data = data;
				}
				else
				{
					subnode *right=temp->data;
					while(right->next != NULL)
						right=right->next;
					right->next=data;
					
				}				
				break;
			}
		    if(temp->supp<num)
				c++;
		    temp=temp->next;
		}
		if (exist == 1) 
		{
			return lscount;
		}
		if(c==0)
		    add(num, data);
		else if(c<lscount)
		    addafter(num, data, ++c);
		else
		    append(num, data);
    } 
	return lscount + 1;
   
}


int remove(int num)
{
    struct node *temp, *prev;
    temp=head;
    while(temp!=NULL)
    {
		if(temp->supp==num)
		{
		    if(temp==head)
		    {
				head=temp->next;
				free(temp);
				return 1;
		    }
		    else
		    {
				prev->next=temp->next;
				free(temp);
				return 1;
		    }
		}
		else
		{
		    prev=temp;
		    temp= temp->next;
		}
    }
    return 0;
}


void  display(struct node *r)
{
	subnode	*s =NULL ;
	int k=0;
    r=head;	
    if(r==NULL)
    {
		return;
    }

    while(r!=NULL)
    {
		printf("\n");
		printf("%d ",r->supp);
		s = r->data;
		while (s!=NULL)
		{
			if (s->size > 0)
				for (k=0; k < s->size; k++)
				{
					printf("%d",s->S[k]);
				}
				printf(" ");
			s = s->next;
		}
		r=r->next;
    }
    printf("\n");
}

subnode *createNode(subnode *stack, int val) {
	subnode *node =(subnode*) malloc(sizeof(subnode));
	node->size = 1;		
	node->S =(int*) malloc(sizeof(int));
	node->S[0] = val;
	node->sumeidx = val;		
	if(stack ==NULL)
	{	 
	  stack=node;
	  node->next = NULL;	 
	}
	else
	{
	   node->next=stack;
	   stack=node; 
	}	 
	return stack;
}

candidate *createCandidate(candidate *stack, int *pattern, int patternsize, int support, int nodesize, ItemNode *nodelist) 
{	
	candidate *can =(candidate*) malloc(sizeof(candidate));	

	int *data=(int*)malloc(patternsize*sizeof(int));
	memcpy(data, pattern, patternsize*sizeof(int));
	can->pattern = data;
	can->patternsize = patternsize;
	can->support = support;
	can->nodesize = nodesize;
	can->nodelist = nodelist;

	if(stack ==NULL)
	{	 
	  stack=can;
	  can->next = NULL;	 
	}
	else
	{
	   can->next=stack;
	   stack=can; 
	}	 
	return stack;
}