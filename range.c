#include "range.h"

void allocRanges(range* r, int num)
{
	assert(r);
	assert(num>0);
	
	r->ranges=malloc(num * sizeof(uRange*));
	int i;
	for(i=0;i<num;i++)
		r->ranges[i]==NULL;
	
}
