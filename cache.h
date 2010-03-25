#ifndef __CACHE
#define __CACHE

#include "defs.h"
#include "box.h"
#include "array.h"

#define F_COPYCACHE 1
#define F_INCACHE 2
#define F_THROUGH 4

typedef enum {Terror=0, Tbox} cacheType;

typedef struct {
	void* data; 
	unsigned char flag;
} cacheItem;

typedef struct cachetemp {
	void (*put)(struct cachetemp *c, long long i, void* send, unsigned flags); 
	void (*get)(struct cachetemp *c, long long i, void** recv, unsigned flags); 
	void (*flush)(struct cachetemp *c); 
	void* real_cache;
	void* real_array;
	cacheType type;
	arrayType typeA;
	
} cache;

void cacheBoxFromArray1(cache* c, array1* a);

void cacheFree(cache* c);

void cachePut_box(cache *c, long long i, void* send, unsigned flags);
void cacheGet_box(cache *c, long long i, void** recv, unsigned flags);
void cacheFlush_box(cache *c);

#endif
