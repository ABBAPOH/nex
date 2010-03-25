#ifndef __BOX
#define __BOX

#include "defs.h"

//currently unused
#define BIndex int

typedef struct boxtemp 
{ 
	void* data;  
	struct boxtemp* nextlvl;
	long long index;  
} BoxNode;

typedef struct boxheadertemp
{
	BoxNode *box;
        unsigned (*hash)(long long , int lvl);
	void (*levelAlloc)(struct boxheadertemp* bh);
	unsigned short maxLevel;
	int* levelSizes;
	int dataSize;
} BoxHeader;

unsigned hash_native(long long index, int lvl);
void levelAlloc_native(struct boxheadertemp* bh);

void boxFromNative(BoxHeader *bh, int dataSize);
void boxNodeNew(BoxNode *b, BoxHeader *bh);
void boxFree(BoxHeader *bh);
void boxNodeFree(BoxNode *b, BoxHeader *bh, int level);
void* boxNodeGet(BoxNode *b, long long index, BoxHeader *bh, int level);
void boxNodePut(BoxNode *b, long long index,void* data, BoxHeader *bh, int level, unsigned char copyFlag);
void boxNodeDel(BoxNode *b, long long index, BoxHeader *bh, int level);
void* boxGet(BoxHeader *bh, long long index);
void boxPut(BoxHeader *bh, long long index, void* data);
void boxPutNoCopy(BoxHeader *bh, long long index, void* data);
void boxDel(BoxHeader *bh, long long index);

void boxMapAll(BoxHeader *bh, void (*mapFunc)(void* obj, long long index, BoxHeader *bh, void* ext), void* ext);
void boxMapSome(BoxHeader *bh, void (*mapFunc)(void* obj, long long index, BoxHeader *bh, void* ext), int (*ifFunc)(long long index, void* ext), void* ext);

#endif
