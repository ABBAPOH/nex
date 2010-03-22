#ifndef __BOX
#define __BOX

#include "defs.h"

//currently unused
#define BIndex int

typedef struct boxtemp 
{ 
	void* data;  
	struct boxtemp* nextlvl;
        unsigned index;
	unsigned short int levelSize;  
	unsigned char level;
} BoxNode;

typedef struct boxheadertemp
{
	BoxNode *box;
        unsigned (*hash)(unsigned , int lvl);
	void (*levelAlloc)(struct boxtemp* b,int lvl);
	unsigned char maxLevel;
	int dataSize;
} BoxHeader;

unsigned hash_native(unsigned index, int lvl);
void levelAlloc_native(struct boxtemp* b,int lvl);

void boxFromNative(BoxHeader *bh, int dataSize);
void boxNodeNew(BoxNode *b, BoxHeader *bh);
void boxFree(BoxHeader *bh);
void boxNodeFree(BoxNode *b);
void* boxNodeGet(BoxNode *b, unsigned index, BoxHeader *bh);
void boxNodePut(BoxNode *b, unsigned index,void* data, BoxHeader *bh, unsigned char copyFlag);
void boxNodeDel(BoxNode *b, unsigned index, BoxHeader *bh);
void* boxGet(BoxHeader *bh, unsigned index);
void boxPut(BoxHeader *bh, unsigned index, void* data);
void boxPutNoCopy(BoxHeader *bh, unsigned index, void* data);
void boxDel(BoxHeader *bh, unsigned index);

void boxMapAll(BoxHeader *bh, void (*mapFunc)(void* obj, unsigned index, BoxHeader *bh));
void boxMapSome(BoxHeader *bh, void (*mapFunc)(void* obj, unsigned index, BoxHeader *bh), int (*ifFunc)(unsigned index));

#endif
