#ifndef __BOX
#define __BOX

#include "defs.h"

//currently unused
#define BIndex int

typedef struct boxtemp 
{
	uint (*hash)(uint , int lvl); 
	void (*levelAlloc)(struct boxtemp* b,int lvl); 
	uint index; 
	int level,maxLevel; 
	void* data; 
	int dataSize; 
	struct boxtemp* nextlvl; 
	int levelSize; 
	int set; 
	int numUsed} Box;

uint hash_native(uint index, int lvl);
void levelAlloc_native(struct boxtemp* b,int lvl);

void boxFromNative(Box *b, int dataSize);
void boxFree(Box *b);
void* boxGet(Box *b,uint index);
void boxPut(Box *b,uint index,void* data);

#endif