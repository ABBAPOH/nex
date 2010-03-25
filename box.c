#include "box.h"

/*!
  \fn unsigned hash_native(long long index, int lvl)
  \brief Native hash-function.

*/
unsigned hash_native(long long index, int lvl)
{
	return (index>>(8*(lvl))   )&255;
}

/*!
  \fn void levelAlloc_native(struct boxtemp* b, int lvl)
  \brief Returns box level size.

*/
void levelAlloc_native(struct boxheadertemp* bh)
{
	assert(bh);
	
	int i;
	
	bh->maxLevel=4;
	bh->levelSizes=malloc(sizeof(int)*bh->maxLevel);
	
	for(i=0;i<bh->maxLevel;i++)
		bh->levelSizes[i]=256;
}

/*!
  \fn void boxFromNative(BoxHeader *bh, int dataSize)
  \brief Creates native container with element size \a dataSize.

*/
void boxFromNative(BoxHeader *bh, int dataSize)
{
	assert(bh);
	assert(dataSize>0);
	
	bh->dataSize=dataSize;
	bh->hash=hash_native;
	bh->levelAlloc=levelAlloc_native;
	bh->levelAlloc(bh);
	
	bh->box=malloc(sizeof(BoxNode));
	boxNodeNew(bh->box,bh);
}

/*!
  \fn void boxNodeNew(BoxNode *b, BoxHeader *bh)
  \brief Inner function. Fills node \a b with defaut values.

*/
void boxNodeNew(BoxNode *b, BoxHeader *bh)
{
	assert(b);
	assert(bh);
	
	b->index=-1;
	
	b->data=NULL;
	b->nextlvl=NULL;
}

/*!
  \fn void boxFree(BoxHeader *bh)
  \brief Frees box \a bh.

*/
void boxFree(BoxHeader *bh)
{
	assert(bh);
	
	bh->hash=NULL;
	bh->levelAlloc=NULL;
	bh->maxLevel=-1;
	bh->dataSize=-1;
	
	boxNodeFree(bh->box, bh, 0);
	free(bh->box);
	free(bh->levelSizes);
}

/*!
  \fn void boxNodeFree(BoxNode *b, BoxHeader *bh, int level)
  \brief Inner function. Recursively frees nodes.

  Used in boxFree.

*/
void boxNodeFree(BoxNode *b, BoxHeader *bh, int level)
{
	assert(b);
	assert(bh);
	
        if (b->data != NULL)
	{
		MPI_Free_mem(b->data);
		b->data=NULL;
	}
	
        if (b->nextlvl != NULL)
	{
		int i;
		for(i=0;i<bh->levelSizes[level];i++)
			boxNodeFree(   &((b->nextlvl)[i])  , bh, level+1);
		free(b->nextlvl);
		b->nextlvl=NULL;
	}
	
	b->index=-1;
}

/*!
  \fn void *boxNodeGet(BoxNode *b, long long index, BoxHeader *bh, int level)
  \brief Inner function. Resursively searches element index.
  \param b Node to start with
  \param index Index to search
  \param bh Box Header
*/
void *boxNodeGet(BoxNode *b, long long index, BoxHeader *bh, int level)
{
	assert(b);
	assert(bh);
	
	if((b->data!=NULL) && (index==b->index))
		return b->data;
	
	if(b->nextlvl==NULL)
		return NULL;
	
	return boxNodeGet(  &(b->nextlvl[bh->hash(index,level)]) , index, bh, level+1);
}

/*!
  \fn void boxNodePut(BoxNode *b, long long index, void* data, BoxHeader *bh, int level, unsigned char copyFlag)
  \brief Inner function. Inserts \a data by index \a index
  \param b Node to start with
  \param index Index to insert
  \param data Data to insert
  \param bh Box Header
  \param copyFlag If true, copy of data is stored, otherwise - pointer to it

  Allocates memory for cells if necessary
*/
void boxNodePut(BoxNode *b, long long index, void* data, BoxHeader *bh, int level, unsigned char copyFlag)
{
	assert(b);
	assert(bh);
	assert(data);
	
	if(b->data==NULL)
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
		b->index=index;
		return;
	}
	
	//overwrite
	if(b->index==index)
	{
		if(copyFlag)
		{
			memcpy(b->data,data, bh->dataSize);
		}
		else
		{
			MPI_Free_mem(b->data);
			b->data=data;
		}
		return;
	}
	
	assert(level < bh->maxLevel);
	
	if(b->nextlvl==NULL)
	{
		b->nextlvl=malloc(sizeof(BoxNode) * bh->levelSizes[level]);
		int i;
		for(i=0;i<bh->levelSizes[level];i++)
		{
			boxNodeNew(  &((b->nextlvl)[i])  , bh);
		}
	}
	
	boxNodePut(  &(b->nextlvl[bh->hash(index,level)])  , index, data, bh, level+1, copyFlag);
	
}

void boxNodeDel(BoxNode *b, long long index, BoxHeader *bh, int level)
{
	assert(b);
	assert(bh);
	
	if((b->data!=NULL) && (index==b->index))
	{
		b->index=-1;
		MPI_Free_mem(b->data);
		b->data=NULL;
		return;
	}
	
	if(b->nextlvl!=NULL)
		boxNodeDel(  &(b->nextlvl[bh->hash(index,level)]) , index, bh, level+1);
}

/*!
  \fn void* boxGet(BoxHeader *bh, long long index)
  \brief Gets element by \a index

*/
void* boxGet(BoxHeader *bh, long long index)
{
	assert(bh);
	
	return boxNodeGet(bh->box, index, bh, 0);
}

/*!
  \fn void boxPut(BoxHeader *bh, long long index, void* data)
  \brief Inserts \a data by \a index

*/
void boxPut(BoxHeader *bh, long long index, void* data)
{
	assert(bh);
	assert(data);
	
	boxNodePut(bh->box, index, data, bh, 0, 1);
}

/*!
  \fn void boxPutNoCopy(BoxHeader *bh, long long index, void* data)
  \brief Inserts \a pointer to data (not a copy) by \a index.

*/
void boxPutNoCopy(BoxHeader *bh, long long index, void* data)
{
	assert(bh);
	assert(data);
	
	boxNodePut(bh->box, index, data, bh, 0, 0);
}

void boxDel(BoxHeader *bh, long long index)
{
	assert(bh);
	
	boxNodeDel(bh->box, index, bh, 0);
}

/*!
  \fn void boxMapRec(BoxHeader *bh, BoxNode *b, void (*mapFunc)(void* obj, long long index, BoxHeader *bh, void* ext), int (*ifFunc)(long long index, void* ext), void* ext, int level)
  \brief Inner Function. Realization of boxMap*.

  If \a ifFunc == 0, all works with all nodes, otherwise selects them using \a IfFunc

*/
void boxMapRec(BoxHeader *bh, BoxNode *b, void (*mapFunc)(void* obj, long long index, BoxHeader *bh, void* ext), int (*ifFunc)(long long index, void* ext), void* ext, int level)
{
	//private
	assert(bh);
	assert(b);
	assert(mapFunc);
	
	if((b->data!=NULL) && ( (ifFunc==NULL)  ||  (ifFunc(b->index, ext)) ))
	{
		mapFunc(b->data, b->index, bh, ext);
	}
	
	if(b->nextlvl!=NULL)
	{
		int i;
		for(i=0;i<bh->levelSizes[level];i++)
			boxMapRec(bh, &(b->nextlvl[i]), mapFunc, ifFunc, ext, level+1);
	}
	
	return;
}

/*!
  \fn void boxMapAll(BoxHeader *bh, void (*mapFunc)(void* obj, long long index, BoxHeader *bh))
  \brief Applies \a mapFunc to all data in box \a bh.

*/
void boxMapAll(BoxHeader *bh, void (*mapFunc)(void* obj, long long index, BoxHeader *bh, void* ext), void* ext)
{
	assert(bh);
	assert(mapFunc);
	
	boxMapRec(bh, bh->box, mapFunc, NULL, ext, 0);
}

/*!
  \fn void boxMapSome(BoxHeader *bh, void (*mapFunc)(void* obj, long long index, BoxHeader *bh), int (*ifFunc)(long long index))
  \brief Applies \a mapFunc to data in box \a bh only if \a ifFunc returns 1 being applied to that \a index.

*/
void boxMapSome(BoxHeader *bh, void (*mapFunc)(void* obj, long long index, BoxHeader *bh, void* ext), int (*ifFunc)(long long index, void* ext), void* ext)
{
	assert(bh);
	assert(mapFunc);
	
	boxMapRec(bh, bh->box, mapFunc, ifFunc, ext, 0);
}
