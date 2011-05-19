#ifndef __CACHE
#define __CACHE

#include "defs.h"
#include "box.h"
#include "array.h"
#include "narray.h"

#define F_COPYCACHE 1
#define F_INCACHE 2
#define F_THROUGH 4

typedef enum {Terror=0, Tbox} cacheType;

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

typedef struct ncachetemp {
	void (*put)(struct ncachetemp *c, int index[], void* send, unsigned flags);
	void (*get)(struct ncachetemp *c, int index[], void** recv, unsigned flags);
	void (*getInBuffer)(struct ncachetemp *c, int index[], void** recv, unsigned flags);
	void (*putLine)(struct ncachetemp *c, int index[], void* send, int size, unsigned flags);
	void (*getLine)(struct ncachetemp *c, int index[], void** recv, int size, unsigned flags);
	void (*getLineInBuffer)(struct ncachetemp *c, int index[], void* recv, int size, unsigned flags);
	void (*putBlock)(struct ncachetemp *c, int index[], void* send, int sizes[], unsigned flags);
	void (*getBlock)(struct ncachetemp *c, int index[], void** recv, int sizes[], unsigned flags);
	void (*getBlockInBuffer)(struct ncachetemp *c, int index[], void* recv, int sizes[], unsigned flags);
	void (*flush)(struct ncachetemp *c);
	void (*clear)(struct ncachetemp *c);
	void* real_cache;
	narray* array;
	cacheType type;
} ncache;

void cacheBoxFromArray1(Cache* c, array1* a);
void cacheBoxFromArray2(Cache* c, array2* a);
void cacheFree(Cache* c);

void ncacheBoxFromnarray(ncache* c, narray* na);
void ncacheFree(ncache* c);


void cachePut_box(Cache *c, abstractPointer p, void* send, unsigned flags);
void cacheGet_box(Cache *c, abstractPointer p, void** recv, unsigned flags);
void cacheFlush_box(Cache *c);
void cacheClear_box(Cache *c);

void ncachePut_box(ncache *c, int index[], void* send, unsigned flags);
void ncacheGet_box(ncache *c, int index[], void** recv, unsigned flags);
void ncacheGetInBuffer_box(ncache *c, int index[], void** recv, unsigned flags);
void ncachePutLine_box(ncache *c, int index[], void* send, int size, unsigned flags);
void ncacheGetLine_box(ncache *c, int index[], void** recv, int size, unsigned flags);
void ncacheGetLineInBuffer_box(ncache *c, int index[], void* recv, int size, unsigned flags);
void ncachePutBlock_box(ncache *c, int index[], void* send, int sizes[], unsigned flags);
void ncacheGetBlock_box(ncache *c, int index[], void** recv, int sizes[], unsigned flags);
void ncacheGetBlockInBuffer_box(ncache *c, int index[], void* recv, int sizes[], unsigned flags);

void ncacheFlush_box(ncache *c);
void ncacheClear_box(ncache *c);

void cacheFence(Cache *c);
void ncacheFence(ncache *c);

#endif
