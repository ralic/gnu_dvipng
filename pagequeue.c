#include "dvipng.h"

/*
 * Handle a queue of pages for dvipng.  Code would have been based on
 * dvips, but for the intended use of dvipng the ordering becomes
 * important.
 */

struct pagequeue {
  struct pagequeue *next; 
  int first,last;
  bool abspage;
} *hpagequeuep = NULL;  

/*-->FindQdPage*/
/**********************************************************************/
/****************************  FindQdPage  ****************************/
/**********************************************************************/
/* Return the page in turn on our queue */

struct page_list* FindQdPage(void)
{
  struct page_list* page;
  if (hpagequeuep==NULL) 
    return(NULL);

  page=FindPage(hpagequeuep->first, hpagequeuep->abspage);
  
  if (hpagequeuep->first==PAGE_LASTPAGE) {
    hpagequeuep->first=page->count[hpagequeuep->abspage ? 10 : 0];
  }

  if (hpagequeuep->first==hpagequeuep->last) {
    struct pagequeue *temp = hpagequeuep;
    hpagequeuep = hpagequeuep->next;
    free(temp);
  } else if (hpagequeuep->first<hpagequeuep->last) {
    hpagequeuep->first++;
  } else {
    hpagequeuep->first--;
  }
  return(page);
}

void QueuePage(int first, int last, bool abspage, bool reverse)
{
  struct pagequeue *new;

  if ((new = (struct pagequeue *)malloc(sizeof(struct pagequeue)))
      ==NULL)
    Fatal("cannot allocate memory for page queue");

  if (!reverse) {
    new->first=first;
    new->last=last;
  } else {
    new->first=last;
    new->last=first;
  }
  new->abspage=abspage;

  if (reverse || hpagequeuep==NULL) {
    new->next=hpagequeuep;
    hpagequeuep=new;
  } else {
    register struct pagequeue *temp = hpagequeuep;
    
    while(temp->next!=NULL)
      temp=temp->next;
    temp->next=new;
    new->next=NULL;
  }
}


bool QueueEmpty(void)
{
  return(hpagequeuep==NULL);
}

/* Parse a string representing a list of pages.  Return 0 iff ok.  As a
   side effect, the page(s) is (are) ap- or pre-pended to the queue. */
/* THIS is adapted from dvips */
bool QueueParse(register char * s, bool abspage, bool reverse)
{
    register int    c ;		/* current character */
    register int  n = 0,	/* current numeric value */
		    innumber;	/* true => gathering a number */
    int ps_low = 0, ps_high = 0 ;
    int     range,		/* true => saw a range indicator */
	    negative = 0;	/* true => number being built is negative */

#define white(x) ((x) == ' ' || (x) == '\t' || (x) == ',')

    range = 0;
    innumber = 0;
    for (;;) {
	c = *s++;
	if ( !innumber && !range) {/* nothing special going on */
	    if (c == 0)
		return(_FALSE);
	    if (white (c))
		continue;
	}
	if (c == '-' && !innumber) {
		innumber++;
		negative++;
		n = 0;
		continue;
	}
	if ('0' <= c && c <= '9') {	/* accumulate numeric value */
	    if (!innumber) {
		innumber++;
		negative = 0;
		n = c - '0';
		continue;
	    }
	    n *= 10;
	    n += negative ? '0' - c : c - '0';
	    continue;
	}
	if (c == '-' || c == ':') {/* here's a range */
	    if (range)
		return(_TRUE);
	    if (innumber) {	/* have a lower bound */
		ps_low = n;
	    }
	    else
		ps_low = PAGE_MINPAGE;
	    range++;
	    innumber = 0;
	    continue;
	}
	if (c == 0 || white (c)) {/* end of this range */
	    if (!innumber) {	/* no upper bound */
		ps_high = PAGE_MAXPAGE;
		if (!range)	/* no lower bound either */
		    ps_low = PAGE_MINPAGE;
	    }
	    else {		/* have an upper bound */
		ps_high = n;
		if (!range) {	/* no range => lower bound == upper */
		    ps_low = ps_high;
		}
	    }
	    QueuePage(ps_low, ps_high,abspage,reverse);
	    if (c == 0)
		return(_FALSE);
	    range = 0;
	    innumber = 0;
	    continue;
	}
	return(_TRUE);
    }
#undef white
}
