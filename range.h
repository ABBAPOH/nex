#ifndef __RANGE
#define __RANGE

#include "defs.h"

typedef struct 
{
	int start,end;
	void* (*map)(int index);
} abRange;

typedef struct 
{
	void *bits; 
	int size;
} bitmapRange;

typedef enum {abr=0,bmr=1} rangeType;

typedef struct
{
	rangeType type; 
	union
	{
		abRange ab; 
		bitmapRange bm;
	};
} uRange;

typedef struct
{
	uRange **ranges;
	int size;
} range;

void allocRanges(range* r, int num);

#endif
