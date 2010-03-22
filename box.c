#include "box.h"

/*!
  \fn unsigned hash_native(unsigned index, int lvl)
  \brief Native hash-function.

*/
unsigned hash_native(unsigned index, int lvl)
{
	return (index>>(8*(lvl-1))   )&255;
}

/*!
  \fn void levelAlloc_native(struct boxtemp* b, int lvl)
  \brief Returns box level size.

*/
void levelAlloc_native(struct boxtemp* b, int lvl)
{
	assert(b);
	
	b->levelSize=256;
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
	bh->maxLevel=4;//depends on native
	
	//MPI_Alloc_mem( sizeof(BoxNode), MPI_INFO_NULL, &(bh->box) );
	bh->box=malloc(sizeof(BoxNode));
	boxNodeNew(bh->box,bh);
	
	assert(bh->box->level  <=  bh->maxLevel);
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
	b->level=1;
	b->set=0;
	//b->numUsed=0;
	
	b->data=NULL;
	b->nextlvl=NULL;
	
	bh->levelAlloc(b,b->level);
	
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
	
	boxNodeFree(bh->box);
	//MPI_Free_mem(bh->box);
	free(bh->box);
}

/*!
  \fn void boxNodeFree(BoxNode *b)
  \brief Inner function. Recursively frees nodes.

  Used in boxFree.

*/
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

/*!
  \fn void *boxNodeGet(BoxNode *b, unsigned index, BoxHeader *bh)
  \brief Inner function. Resursively searches element index.
  \param b Node to start with
  \param index Index to search
  \param bh Box Header
*/
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

/*!
  \fn void boxNodePut(BoxNode *b, unsigned index, void* data, BoxHeader *bh, unsigned char copyFlag)
  \brief Inner function. Inserts \a data by index \a index
  \param b Node to start with
  \param index Index to insert
  \param data Data to insert
  \param bh Box Header
  \param copyFlag If true, copy of data is stored, otherwise - pointer to it

  Allocates memory for cells if necessary
*/
void boxNodePut(BoxNode *b, unsigned index, void* data, BoxHeader *bh, unsigned char copyFlag)
{
	assert(b);
	assert(bh);
	assert(data);
	
	if(b->set==0)
	{
		if(copyFlag)
		{
			if(b->data==NULL)
				MPI_Alloc_mem( bh->dataSize, MPI_INFO_NULL, &(b->data) );
			memcpy(b->data,data, bh->dataSize);
		}
		else
		{
			if(b->data!=NULL)
				MPI_Free_mem(b->data);
			b->data=data;
		}
		b->set=1;
		b->index=index;
		return;
	}
	
	//if(b->set==1)//always true
	if(b->index==index)
	{
		if(copyFlag)
		{
			memcpy(b->data,data, bh->dataSize);
		}
		else
		{
			if(b->data!=NULL)
				MPI_Free_mem(b->data);
			b->data=data;
		}
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

/*!
  \fn void* boxGet(BoxHeader *bh, unsigned index)
  \brief Gets element by \a index

*/
void* boxGet(BoxHeader *bh, unsigned index)
{
	assert(bh);
	
	return boxNodeGet(bh->box, index, bh);
}

/*!
  \fn void boxPut(BoxHeader *bh, unsigned index, void* data)
  \brief Inserts \a data by \a index

*/
void boxPut(BoxHeader *bh, unsigned index, void* data)
{
	assert(bh);
	assert(data);
	
	boxNodePut(bh->box, index, data, bh, 1);
}

/*!
  \fn void boxPutNoCopy(BoxHeader *bh, unsigned index, void* data)
  \brief Inserts \a pointer to data (not a copy) by \a index.

*/
void boxPutNoCopy(BoxHeader *bh, unsigned index, void* data)
{
	assert(bh);
	assert(data);
	
	boxNodePut(bh->box, index, data, bh, 0);
}

/*!
  \fn void boxMapRec(BoxHeader *bh, BoxNode *b, void (*mapFunc)(void* obj, unsigned index, BoxHeader *bh), int (*ifFunc)(unsigned index))
  \brief Inner Function. Realization of boxMap*.

  If \a ifFunc == 0, all works with all nodes, otherwise selects them using \a IfFunc

*/
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

/*!
  \fn void boxMapAll(BoxHeader *bh, void (*mapFunc)(void* obj, unsigned index, BoxHeader *bh))
  \brief Applies \a mapFunc to all data in box \a bh.

*/
void boxMapAll(BoxHeader *bh, void (*mapFunc)(void* obj, unsigned index, BoxHeader *bh))
{
	assert(bh);
	assert(mapFunc);
	
	boxMapRec(bh, bh->box, mapFunc, NULL);
}

/*!
  \fn void boxMapSome(BoxHeader *bh, void (*mapFunc)(void* obj, unsigned index, BoxHeader *bh), int (*ifFunc)(unsigned index))
  \brief Applies \a mapFunc to data in box \a bh only if \a ifFunc returns 1 being applied to that \a index.

*/
void boxMapSome(BoxHeader *bh, void (*mapFunc)(void* obj, unsigned index, BoxHeader *bh), int (*ifFunc)(unsigned index))
{
	assert(bh);
	assert(mapFunc);
	
	boxMapRec(bh, bh->box, mapFunc, ifFunc);
}
