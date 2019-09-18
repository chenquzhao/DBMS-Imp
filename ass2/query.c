// query.c ... query scan functions
// part of Multi-attribute Linear-hashed Files
// Manage creating and using Query objects
// Last modified by John Shepherd, July 2019

#include "defs.h"
#include "query.h"
#include "reln.h"
#include "tuple.h"
#include "hash.h"
// A suggestion ... you can change however you like

struct QueryRep {
	Reln    rel;       // need to remember Relation info
	Bits    known;     // the hash value from MAH
	Bits    unknown;   // the unknown bits from MAH
	PageID  curpage;   // current page in scan
	int     is_ovflow; // are we in the overflow pages?
	Offset  curtup;	   // offset of current tuple with in page
	//TODO 
	Tuple	querytup;	// query tuple 
	PageID	curovpage;	// current overflow page in scan
	Count	ntup;		// current scan tuple index in page
};

// take a query string (e.g. "1234,?,abc,?")
// set up a QueryRep object for the scan

Query startQuery(Reln r, char *q)
{
	Query new = malloc(sizeof(struct QueryRep));
	assert(new != NULL);
	// TODO
	// Partial algorithm:
	Bits known,unknown;
	known=unknown=0;
	//check the query string
	char *c; 
	int nf = 1;
	for (c = q; *c != '\0'; c++)
		if (*c == ',') nf++;
	// invalid query string
	if (nf != nattrs(r)) return NULL;
	//store query tuple
	new->querytup=copyString(q);
	//split tuple to vals[]
	char **vals = malloc(nf*sizeof(char *));
	assert(vals != NULL);
	tupleVals(new->querytup, vals);
	//hash value 
	int i,j;
	for (i = 0; i < nf; i++) {
		if(vals[i][0] != '?'){
			// form known bits from known attributes
			Bits hash=hash_any((unsigned char *)vals[i],strlen(vals[i]));
			for(j=0;j<MAXBITS;j++){
				if((chvec(r)[j].att==i)&&bitIsSet(hash,chvec(r)[j].bit)){  
					known=setBit(known,j);
				}
			}
		}else{
			// form unknown bits from '?' attributes
			for(j=0;j<MAXBITS;j++){
				if(chvec(r)[j].att==i){
					unknown=setBit(unknown,j);
				}
			}
		}
	}
	// compute PageID of first page
	Bits p=getLower(known,depth(r));
	new->curpage=p;
	// set all values in QueryRep object
	new->known=known;
	new->unknown=unknown;
	new->rel=r;
	new->is_ovflow=0;
	new->curovpage=0;
	new->curtup=0;
	new->ntup=0;
	return new;
}

PageID getNextPage(Query q){
	PageID p=q->curpage;
	//mask depth
	int d;
	while(1){
		p++;
		if(p < npages(q->rel)){
			//depth
			d= depth(q->rel);
			//check pageid  
			if(getLower(q->known,d)==getLower(p&(~q->unknown),d)){
				//return matched pageid
				return p;
			}

		}else{
			return NO_PAGE;
		}
	}
	return NO_PAGE;
}

// get next tuple during a scan

Tuple getNextTuple(Query q)
{
	// TODO
	// Partial algorithm:
	// if (more tuples in current page)
	//    get next matching tuple from current page
	// else if (current page has overflow)
	//    move to overflow page
	//    grab first matching tuple from page
	// else
	//    move to "next" bucket
	//    grab first matching tuple from data page
	// endif
	// if (current page has no matching tuples)
	//    go to next page (try again)
	// endif
	Page pg;
	Tuple t;
	//scan all matched page
	while(1){
		//get current page
		if(q->is_ovflow==0){
			pg = getPage(dataFile(q->rel),q->curpage);
		}else{
			pg = getPage(ovflowFile(q->rel),q->curovpage);
		}
		
		Count ntups = pageNTuples(pg);
		char *c = pageData(pg)+q->curtup;
		
		//scan all tuple in current page
		for (Count i = q->ntup; i < ntups; i++) {
			//trans to tuple
			t=copyString(c);
			q->curtup+= strlen(c) + 1;
			q->ntup++;	
			//check match 
			if(tupleMatch(q->rel,q->querytup, t)==TRUE){
				free(pg);
				return t;
			}
			//release allocated buffer
			free(t);
			c += strlen(c) + 1;
		}
		//check next page
		if(pageOvflow(pg) == NO_PAGE){
			//next primary page(bucket)
			PageID p=getNextPage(q);
			if(p!=NO_PAGE){
				//move to next primary page(bucket)
				q->curpage=p;
				q->is_ovflow = 0; 
				q->curtup=0;
    			q->ntup=0;
			}else{
				free(pg);
				//scan end
				break;		
			}
		}else{
			//move to next overflow page
			q->curovpage = pageOvflow(pg);
			q->is_ovflow = 1;    
			q->curtup=0;
			q->ntup=0;
		}
		//release allocated buffer
		free(pg);
		
	}

	return NULL;
}

// clean up a QueryRep object and associated data

void closeQuery(Query q)
{
	// TODO
	//release allocated buffer
	free(q->querytup);
	free(q);
}
