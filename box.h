#ifndef __BOX
#define __BOX

#include "defs.h"

//currently unused
#define BIndex int

typedef struct boxtemp 
{ 
	void* data;  
	struct boxtemp* nextlvl;
	uint index;
	short int levelSize;  
	//short int numUsed;
	unsigned char set;
	unsigned char level;
} BoxNode;

typedef struct boxheadertemp
{
	BoxNode *box;
	uint (*hash)(uint , int lvl); 
	void (*levelAlloc)(struct boxtemp* b,int lvl);
	unsigned char maxLevel;
	int dataSize;
} BoxHeader;

uint hash_native(uint index, int lvl);
void levelAlloc_native(struct boxtemp* b,int lvl);

void boxFromNative(BoxHeader *bh, int dataSize);
void boxNodeNew(BoxNode *b, BoxHeader *bh);
void boxFree(BoxHeader *bh);
void boxNodeFree(BoxNode *b);
void* boxNodeGet(BoxNode *b,uint index, BoxHeader *bh);
void boxNodePut(BoxNode *b,uint index,void* data, BoxHeader *bh, unsigned char copyFlag);
void* boxGet(BoxHeader *bh,uint index);
void boxPut(BoxHeader *bh,uint index,void* data);
void boxPutNoCopy(BoxHeader *bh,uint index,void* data);

#endif