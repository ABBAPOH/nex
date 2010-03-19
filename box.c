#include "box.h"

unsigned hash_native(unsigned index, int lvl)
{
	return (index>>(8*(lvl-1))   )&255;
}

void levelAlloc_native(struct boxtemp* b,int lvl)
{
	assert(b);
	
	b->levelSize=256;
	
}

void boxFromNative(BoxHeader *bh,int dataSize)
{
	assert(bh);
	assert(dataSize>0);
	
	bh->dataSize=dataSize;
	bh->hash=hash_native;
	bh->levelAlloc=levelAlloc_native;
	bh->maxLevel=4;//depends on native
	
	MPI_Alloc_mem( sizeof(BoxNode), MPI_INFO_NULL, &(bh->box) );
	boxNodeNew(bh->box,bh);
	
	assert(bh->box->level  <=  bh->maxLevel);
}

void boxNodeNew(BoxNode *b, BoxHeader *bh)
{
	assert(b);
	assert(bh);
	
	b->index=-1;
	b->level=1;
	b->set=0;
	//b->numUsed=0;
	
	b->data=NULL;
	b->nextlvl=NULL;
	
	bh->levelAlloc(b,b->level);
	
}

void boxFree(BoxHeader *bh)
{
	assert(bh);
	
	bh->hash=NULL;
	bh->levelAlloc=NULL;
	bh->maxLevel=-1;
	bh->dataSize=-1;
	
	boxNodeFree(bh->box);
	MPI_Free_mem(bh->box);
}

void boxNodeFree(BoxNode *b)
{
	assert(b);
	
	//b->numUsed=-1;
	b->set=0;
	b->index=-1;
	b->level=-1;
	
        if (b->data != NULL)
	{
		MPI_Free_mem(b->data);
		b->data=NULL;
	}
	
        if (b->nextlvl != NULL)
	{
		int i;
		for(i=0;i<b->levelSize;i++)
			boxNodeFree(   &((b->nextlvl)[i])  );
		//MPI_Free_mem(b->nextlvl);
		free(b->nextlvl);
		b->nextlvl=NULL;
		b->levelSize=-1;
	}
}

void *boxNodeGet(BoxNode *b, unsigned index, BoxHeader *bh)
{
	assert(b);
	assert(bh);
	
	if(!(b->set))
		return NULL;
	
	if(index==b->index)
		return b->data;
	
	if(b->nextlvl==NULL)
		return NULL;
	
	return boxNodeGet(  &(b->nextlvl[bh->hash(index,b->level)]) , index,bh);
}

void boxNodePut(BoxNode *b, unsigned index, void* data, BoxHeader *bh, unsigned char copyFlag)
{
	assert(b);
	assert(bh);
	assert(data);
	
	if(b->set==0)
	{
		if(copyFlag)
		{
			MPI_Alloc_mem( bh->dataSize, MPI_INFO_NULL, &(b->data) );
			memcpy(b->data,data, bh->dataSize);
		}
		else
		{
			b->data=data;
		}
		b->set=1;
		b->index=index;
		return;
	}
	
	assert(b->level <= bh->maxLevel);
	
	if(b->nextlvl==NULL)
	{
		b->nextlvl=malloc(sizeof(BoxNode) * b->levelSize);
		int i;
		for(i=0;i<b->levelSize;i++)
		{
			boxNodeNew(  &((b->nextlvl)[i])  ,bh);
			b->nextlvl[i].level=b->level+1;
		}
	}
	
	boxNodePut(  &(b->nextlvl[bh->hash(index,b->level)])  , index, data, bh, copyFlag);
	
}

void* boxGet(BoxHeader *bh,unsigned index)
{
	assert(bh);
	
	return boxNodeGet(bh->box, index, bh);
}

void boxPut(BoxHeader *bh,unsigned index,void* data)
{
	assert(bh);
	assert(data);
	
	boxNodePut(bh->box, index, data, bh, 1);
}

void boxPutNoCopy(BoxHeader *bh,unsigned index,void* data)
{
	assert(bh);
	assert(data);
	
	boxNodePut(bh->box, index, data, bh, 0);
}

void boxMapRec(BoxHeader *bh, BoxNode *b, void (*mapFunc)(void* obj, unsigned index, BoxHeader *bh), int (*ifFunc)(unsigned index))
{
	//private
	assert(bh);
	assert(b);
	assert(mapFunc);
	
	if((b->set) && ( (ifFunc==NULL)  ||  (ifFunc(b->index)) ))
	{
		mapFunc(b->data, b->index, bh);
	}
	
	if(b->nextlvl!=NULL)
	{
		int i;
		for(i=0;i<b->levelSize;i++)
			boxMapRec(bh, &(b->nextlvl[i]), mapFunc, ifFunc);
	}
	
	return;
}

void boxMapAll(BoxHeader *bh, void (*mapFunc)(void* obj, unsigned index, BoxHeader *bh))
{
	assert(bh);
	assert(mapFunc);
	
	boxMapRec(bh, bh->box, mapFunc, NULL);
}

void boxMapSome(BoxHeader *bh, void (*mapFunc)(void* obj, unsigned index, BoxHeader *bh), int (*ifFunc)(unsigned index))
{
	assert(bh);
	assert(mapFunc);
	
	boxMapRec(bh, bh->box, mapFunc, ifFunc);
}
