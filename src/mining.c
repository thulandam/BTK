#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <stdio.h>
#include <stdarg.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include "scan.h"
#include "fptree.h"
#include "prepare.h"
#include <windows.h>
#include <psapi.h>


#pragma comment(lib, "psapi.lib")  
#pragma comment(linker, "/HEAP:2500000")

#define DPSAPI_VERSION 1

#ifdef STORAGE
#include "storage.h"
#endif

/*----------------------------------------------------------------------
  Preprocessor Definitions
----------------------------------------------------------------------*/
#define PRGNAME     "fpmining"
#define DESCRIPTION "find top k frequent item sets " \
                    "with the b-info list + subsume algorithm"
#define VERSION     "version 1.0 (2014.12.05)         " \
                    "(c) 2014-2014   Huy Lan"

/* --- output flags --- */
#define OF_REL      0x01        /* print relative support */
#define OF_ABS      0x02        /* print absolute support */
#define OF_DEV      0x04        /* print deviation from indep. occ. */
#define OF_SCANF    0x08        /* convert names to scanable form */

/* --- error codes --- */
#define E_OPTION    (-5)        /* unknown option */
#define E_OPTARG    (-6)        /* missing option argument */
#define E_ARGCNT    (-7)        /* too few/many arguments */
#define E_STDIN     (-8)        /* double assignment of stdin */
#define E_SUPP      (-9)        /* invalid item set support */
#define E_ITEMCNT  (-10)        /* invalid number of items */
#define E_NOTAS    (-11)        /* no items or transactions */
#define E_UNKNOWN  (-18)        /* unknown error */

#ifndef QUIET                   /* if not quiet version */
#define MSG(x)        x         /* print messages */
#else                           /* if quiet version */
#define MSG(x)                  /* suppress messages */
#endif

#define SEC_SINCE(t)  ((clock()-(t)) /(double)CLOCKS_PER_SEC)
#define RECCNT(s)     (tfs_reccnt(is_tfscan(s)) \
                      + ((tfs_delim(is_tfscan(s)) == TFS_REC) ? 0 : 1))
#define BUFFER(s)     tfs_buf(is_tfscan(s))

/*----------------------------------------------------------------------
  Constants
----------------------------------------------------------------------*/
#define LN_2       0.69314718055994530942   /* ln(2) */

#ifndef QUIET                   /* if not quiet version */
/* --- error messages --- */
static const char *errmsgs[] = {
  /* E_NONE      0 */  "no error\n",
  /* E_NOMEM    -1 */  "not enough memory\n",
  /* E_FOPEN    -2 */  "cannot open file %s\n",
  /* E_FREAD    -3 */  "read error on file %s\n",
  /* E_FWRITE   -4 */  "write error on file %s\n",
  /* E_OPTION   -5 */  "unknown option -%c\n",
  /* E_OPTARG   -6 */  "missing option argument\n",
  /* E_ARGCNT   -7 */  "wrong number of arguments\n",
  /* E_STDIN    -8 */  "double assignment of standard input\n",
  /* E_SUPP     -9 */  "invalid minimal support %g%%\n",
  /* E_ITEMCNT -10 */  "invalid number of items %d\n",
  /* E_NOTAS   -11 */  "no items or transactions to work on\n",
  /*    -12 to -15 */  NULL, NULL, NULL, NULL,
  /* E_ITEMEXP -16 */  "file %s, record %d: item expected\n",
  /* E_DUPITEM -17 */  "file %s, record %d: duplicate item %s\n",
  /* E_UNKNOWN -18 */  "unknown error\n"
};
#endif

/*----------------------------------------------------------------------
  Global Variables
----------------------------------------------------------------------*/
#ifndef QUIET
static char    *prgname;        /* program name for error messages */
#endif
static ITEMSET *itemset = NULL; /* item set */
static TASET   *taset   = NULL; /* transaction set */
static FPTREE  *fptree  = NULL; /* frequent pattern tree */
static FILE    *in      = NULL; /* input  file */
static FILE    *out     = NULL; /* output file */
static int     tacnt    = 0;    /* number of transactions */
static double  mindev   = -DBL_MAX; /* minimal value of deviation */
static double  *logfs   = NULL;     /* logarithms of item frequencies */
static int     flags    = OF_REL;   /* output flags */
static char    *fmt     = "%.1f";   /* output format for support */
static char    buf[4*TFS_SIZE+4];   /* buffer for formatting */

static ItemFirstList	*itemfirstlist  = NULL; /* frequent pattern tree */
static SubsumeList		*sublist  = NULL;
static int topk  = 0;

void showMemoryInfo(void)
{
    HANDLE handle=GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(handle,&pmc,sizeof(pmc));
    printf("Memory usage: %d",pmc.PeakWorkingSetSize/1000);
}

/*----------------------------------------------------------------------
  Item Set Report Function
----------------------------------------------------------------------*/
  void Combination(int numInPair[], int indexInPair, int numArray[],int startOfArray, int endOfArray, int maxCount, int *ids, int cnt,int rank, int supp)
    {
		int i = 0, j = 0;
		const char *name; 
		if(endOfArray == 0 || indexInPair == maxCount) 
		{
			for(i = 0; i< maxCount; i++) 
			{ 
				//printf ("%d ",numInPair[i]);
				name = is_name(itemset, numInPair[i]);
				if (flags & OF_SCANF) {    
				  sc_format(buf, name, 0); name = buf; }
				fputs(name, out); fputc(' ', out);
			} 
			for (j = 0; j < cnt; j++) {   
				name = is_name(itemset, ids[j]);
				if (flags & OF_SCANF) {    
				  sc_format(buf, name, 0); name = buf; }
				fputs(name, out); fputc(' ', out);
			}                             
			fputs(" (", out);             
			fprintf(out, "%d-%d", rank, supp);	
			fputs(")\n", out);	
		}
		else
		{
			for(i = startOfArray; i < endOfArray; i++)
			{
				numInPair[indexInPair] = numArray[i];
				Combination(numInPair,indexInPair+1,numArray,i+1,endOfArray,maxCount, ids,  cnt, rank,  supp);				
			}     
		}
    }
   
    void makeCombination(int * intArray,int arrLength, int pairLength, int *ids, int cnt,int rank, int supp)
    {
		int * arrPair =  (int*) malloc(pairLength * sizeof(int)); 
		Combination(arrPair,0,intArray,0,arrLength, pairLength, ids,  cnt, rank,  supp);
		free(arrPair);
    }
static int _print (int *ids, int cnt,int rank, int supp,  int sumecount, int *sumeidx)
{  
	int        i, j = 0;                 
	const char *name;             
	double     dev = 0;
	assert(ids);   
	for (i = 0; i < cnt; i++) {   
			name = is_name(itemset, ids[i]);
			if (flags & OF_SCANF) {    
			  sc_format(buf, name, 0); name = buf; }
			fputs(name, out); fputc(' ', out);
	}                             
	fputs(" (", out);             
	fprintf(out, "%d-%d", rank, supp);	
	fputs(")\n", out);

	if(sumecount > 0)        
	{		
		for (j = 1; j <= sumecount; j++)
		{
			makeCombination(sumeidx, sumecount, j,  ids,  cnt, rank,  supp);
		}
	}
	
	return 1;                     
}  

/*----------------------------------------------------------------------
  Main Functions
----------------------------------------------------------------------*/

static void error (int code, ...)
{                               /* --- print an error message */
  #ifndef QUIET                 /* if not quiet version */
  va_list    args;              /* list of variable arguments */
  const char *msg;              /* error message */

  assert(prgname);              /* check the program name */
  if (code < E_UNKNOWN) code = E_UNKNOWN;
  if (code < 0) {               /* if to report an error, */
    msg = errmsgs[-code];       /* get the error message */
    if (!msg) msg = errmsgs[-E_UNKNOWN];
    fprintf(stderr, "\n%s: ", prgname);
    va_start(args, code);       /* get variable arguments */
    vfprintf(stderr, msg, args);/* print error message */
    va_end(args);               /* end argument evaluation */
  }
  #endif
  #ifndef NDEBUG                /* if debug version */
  if (fptree)  fpt_delete(fptree);    /* clean up memory */
  if (taset)   tas_delete(taset, 0);  /* and close files */
  if (itemset) is_delete(itemset);
  if (in  && (in  != stdin))  fclose(in);
  if (out && (out != stdout)) fclose(out);
  #endif
  #ifdef STORAGE                /* if storage debugging */
  showmem("at end of program"); /* check memory usage */
  #endif
  exit(code);                   /* abort the program */
}  /* error() */

/*--------------------------------------------------------------------*/

int main (int argc, char *argv[])
{                               /* --- main function */
  int     i, k = 0, n;          /* loop variables, counters */
  char    *s;                   /* to traverse the options */
  char    **optarg = NULL;      /* option argument */
  char    *fn_in   = NULL;      /* name of input  file */
  char    *fn_out  = NULL;      /* name of output file */
  char    *blanks  = NULL;      /* blanks */
  char    *fldseps = NULL;      /* field  separators */
  char    *recseps = NULL;      /* record separators */
  char    *cominds = NULL;      /* comment indicators */
  int	  topk     = 1;         /* top k  */
  int     sort     = -2;        /* flag for item sorting and recoding */
  int     heap     =  1;        /* flag for heap sort vs. quick sort */
  int     *map;                 /* identifier map for recoding */
  clock_t t;                    /* timer for measurements */
  int supp = 0;
  	
  #ifndef QUIET                 /* if not quiet version */
  prgname = argv[0];            /* get program name for error msgs. */
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
  _CrtDumpMemoryLeaks();

  /* --- print usage message --- */
  if (argc > 1) {               /* if arguments are given */
    fprintf(stderr, "%s - %s\n", argv[0], DESCRIPTION);
    fprintf(stderr, VERSION); } /* print a startup message */
  else {                        /* if no arguments given */
    printf("usage: %s [options] infile outfile\n", argv[0]);
    printf("%s\n", DESCRIPTION);
    printf("%s\n", VERSION);
    printf("-k#      top k rank frequent item sets \n"); 
    printf("infile   file to read transactions from\n");
    printf("outfile  file to write frequent item sets to\n");
    return 0;                   /* print a usage message */
  }                             /* and abort the program */
  #endif  /* #ifndef QUIET */

  /* --- evaluate arguments --- */
  for (i = 1; i < argc; i++) {  /* traverse arguments */
    s = argv[i];                /* get option argument */
    if (optarg) { *optarg = s; optarg = NULL; continue; }
    if ((*s == '-') && *++s) {  /* -- if argument is an option */
      while (*s) {              /* traverse options */
        switch (*s++) {         /* evaluate switches */
          case 'k': topk   = (int)strtol(s, &s, 0);   break;
          case 'd': mindev =      strtod(s, &s);    break;
          case 'p': optarg = &fmt;                  break;
          case 'a': flags |= OF_ABS;                break;
          case 'g': flags |= OF_SCANF;              break;
          case 'q': sort   = (int)strtol(s, &s, 0); break;
          case 'j': heap   = 0;                     break;
          case 'i': optarg = &cominds;              break;
          case 'b': optarg = &blanks;               break;
          case 'f': optarg = &fldseps;              break;
          case 'r': optarg = &recseps;              break;
          default : error(E_OPTION, *--s);          break;
        }                       /* set option variables */
        if (optarg && *s) { *optarg = s; optarg = NULL; break; }
      } }                       /* get option argument */
    else {                      /* -- if argument is no option */
      switch (k++) {            /* evaluate non-options */
        case  0: fn_in  = s;      break;
        case  1: fn_out = s;      break;
        default: error(E_ARGCNT); break;
      }                         /* note filenames */
    }
  }
  if (optarg) error(E_OPTARG);  /* check option argument */
  if (k != 2) error(E_ARGCNT);  /* and the number of arguments */
  if (mindev > -DBL_MAX) flags |= OF_DEV;

  /* --- create item set and transaction set --- */
  itemset = is_create();        /* create an item set and */
  if (!itemset) error(E_NOMEM); /* set the special characters */
  is_chars(itemset, blanks, fldseps, recseps, cominds);
  taset = tas_create(itemset);  /* create a transaction set */
  if (!taset) error(E_NOMEM);   /* to store the transactions */
  MSG(fprintf(stderr, "\n"));   /* terminate the startup message */

  /* --- read transactions --- */
  t = clock();                  /* start the timer */
  if (fn_in && *fn_in)          /* if an input file name is given, */
    in = fopen(fn_in, "r");     /* open input file for reading */
  else {                        /* if no input file name is given, */
    in = stdin; fn_in = "<stdin>"; }   /* read from standard input */
  MSG(fprintf(stderr, "reading %s ... ", fn_in));
  if (!in) error(E_FOPEN, fn_in);
  for (tacnt = 0; 1; tacnt++) { /* transaction read loop */
    k = is_read(itemset, in);   /* read the next transaction */
    if (k < 0) error(k, fn_in, RECCNT(itemset), BUFFER(itemset));
    if (k > 0) break;           /* check for error and end of file */
    if (tas_add(taset, NULL, 0) != 0)
      error(E_NOMEM);           /* add the loaded transaction */
  }                             /* to the transaction set */
  if (in != stdin) fclose(in);  /* if not read from standard input, */
  in = NULL;                    /* close the input file */
  n  = is_cnt(itemset);         /* get the number of items */
  
  MSG(fprintf(stderr, "[%d item(s),", n));
  MSG(fprintf(stderr, " %d transaction(s)] done ", tacnt));
  MSG(fprintf(stderr, "[%.2fs].", SEC_SINCE(t)));
  if ((n <= 0) || (tacnt <= 0)) error(E_NOTAS);
  MSG(fprintf(stderr, "\n"));   /* check for at least one transaction */
   
  MSG(fprintf(stderr, "sorting and recoding items ... "));
  //t   = clock();                /* start the timer */
  map = (int*)malloc(is_cnt(itemset) *sizeof(int));
  if (!map) error(E_NOMEM);     /* create an item identifier map */
  
  n = is_recode(itemset, 0, sort, map);   
  
  is_trunc(itemset, n);         /* truncate the itemset and */
  tas_recode(taset, map, n);    /* recode the loaded transactions */

  free(map);                    /* delete the item identifier map */
  MSG(fprintf(stderr, "[%d item(s)] ", n));
  MSG(fprintf(stderr, "done [%.2fs].", SEC_SINCE(t)));
  if (n <= 0) error(E_NOTAS);   /* print a log message and */
  MSG(fprintf(stderr, "\n\n"));   /* check the number of items */
 

  MSG(fprintf(stderr, "creating frequent pattern tree ... "));
  t = clock();                  /* start the timer */
  tas_sort(taset, heap);        /* sort the transactions */
  fptree = fpt_create(taset);   /* create a frequent pattern tree */ 	

  if (!fptree) error(E_NOMEM);  /* to represent the transactions */
  tas_delete(taset, 0);         /* delete the transaction set */
  taset = NULL;                 /* and print a success message */
  
  MSG(fprintf(stderr, "done [%.2fs].\n", SEC_SINCE(t))); 
  
  t = clock();  
  itemfirstlist  = itemfirstlist_create(fptree); 
  MSG(fprintf(stderr, "create b-info list [%.2fs].\n\n", SEC_SINCE(t)));

  t = clock(); 
  sublist = find_subsume(itemfirstlist);
  MSG(fprintf(stderr, "create subsume [%.2fs].\n\n", SEC_SINCE(t)));
  t = clock(); 
  if (fn_out && *fn_out)       
    out = fopen(fn_out, "w");  
  else {                       
    out = stdout; fn_out = "<stdout>"; }   
  MSG(fprintf(stderr, "writing %s ... ", fn_out));
  if (!out) error(E_FOPEN, fn_out); 
  
  fpt_delete(fptree);
 
  fi_searchtopk(itemfirstlist, sublist, topk, _print);  
  
  if (fflush(out) != 0) error(E_FWRITE, fn_out);
  if (out != stdout) fclose(out);
  out = NULL;                   /* close the output file */
  MSG(fprintf(stderr, "[%d set(s)] done ", k));
  MSG(fprintf(stderr, "[%.2fs].\n", SEC_SINCE(t)));
  showMemoryInfo();
 
  is_delete(itemset);  

  if (sublist->sume)  free(sublist->sume);  
  free(sublist);  

  ms_delete(itemfirstlist->mem); 
  
 // #endif
  
  return 0;                     /* return 'ok' */
}  /* main() */
