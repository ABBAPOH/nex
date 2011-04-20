#ifndef __CACHE
#define __CACHE

#include "defs.h"
#include "box.h"
#include "array.h"

#define F_COPYCACHE 1
#define F_INCACHE 2
#define F_THROUGH 4

typedef enum {Terror=0, Tbox} cacheType;

/*typedef struct {
	void* data; 
	unsigned char flag;
} CacheItem;*/

typedef struct cachetemp {
	void (*put)(struct cachetemp *c, abstractPointer p, void* send, unsigned flags); 
	void (*get)(struct cachetemp *c, abstractPointer p, void** recv, unsigned flags); 
	void (*flush)(struct cachetemp *c);
	void (*clear)(struct cachetemp *c);
	void* real_cache;
	void* real_array;
	cacheType type;
	arrayType typeA;
	
} Cache;

void cacheBoxFromArray1(Cache* c, array1* a);
void cacheBoxFromArray2(Cache* c, array2* a);

void cacheFree(Cache* c);

void cachePut_box(Cache *c, abstractPointer p, void* send, unsigned flags);
void cacheGet_box(Cache *c, abstractPointer p, void** recv, unsigned flags);
void cacheFlush_box(Cache *c);
void cacheClear_box(Cache *c);

void cacheFence(Cache *c);

#endif
