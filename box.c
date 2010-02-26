#include "box.h"

uint hash_native(uint index, int lvl)
{
	uint res=index>>(8*(lvl-1));
	res=res&255;
	return res;
}

void levelAlloc_native(struct boxtemp* b,int lvl)
{
	assert(b);
	
	b->levelSize=256;
	b->maxLevel=4;
	
	assert(b->level<=b->maxLevel);
	
}

void boxFromNative(Box *b,int dataSize)
{
	assert(b);
	assert(dataSize>0);
	
	b->index=-1;
	b->level=1;
	b->set=0;
	b->numUsed=0;
	
	b->data=NULL;
	b->nextlvl=NULL;
	
	b->dataSize=dataSize;
	b->hash=hash_native;
	b->levelAlloc=levelAlloc_native;
	b->levelAlloc(b,b->level);
}

void boxFree(Box *b)
{
	assert(b);
	
	b->hash=NULL;
	b->levelAlloc=NULL;
	b->set=0;
	b->index=-1;
	b->level=-1;
	b->numUsed=-1;
	b->maxLevel=-1;
	b->dataSize=-1;
	if(b->data != NULL)
	{
		MPI_Free_mem(b->data);
		b->data=NULL;
	}
	b->dataSize=-1;
	if(b->nextlvl != NULL)
	{
		int i;
		for(i=0;i<b->levelSize;i++)
			boxFree(   &((b->nextlvl)[i])  );
		MPI_Free_mem(b->nextlvl);
		b->nextlvl=NULL;
		b->levelSize=-1;
	}
	
}

void *boxGet(Box *b,uint index)
{
	assert(b);
	
	if(!(b->set))
		return NULL;
	
	if(index==b->index)
		return b->data;
	
	if(b->nextlvl==NULL)
		return NULL;
	
	return boxGet(  &(b->nextlvl[b->hash(index,b->level)]) ,index);
}

void boxPut(Box *b,uint index,void* data)
{
	assert(b);
	assert(data);
	
	if(b->set==0)
	{
		MPI_Alloc_mem( b->dataSize, MPI_INFO_NULL, &(b->data) );
		memcpy(b->data,data, b->dataSize);
		b->set=1;
		b->index=index;
		return;
	}
	
	assert(b->level<=b->maxLevel);
	
	if(b->nextlvl==NULL)
	{
		MPI_Alloc_mem(sizeof(Box) * b->levelSize, MPI_INFO_NULL, &(b->nextlvl));
		int i;
		for(i=0;i<b->levelSize;i++)
		{
			boxFromNative(  &((b->nextlvl)[i])  ,b->dataSize);
			b->nextlvl[i].level=b->level+1;
		}
	}
	
	boxPut(  &(b->nextlvl[b->hash(index,b->level)])  ,index,data);
	
}
