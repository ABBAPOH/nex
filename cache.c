#include "cache.h"

void cacheBoxFromArray1(Cache* c, array1* a)
{
	assert(c);
	assert(a);
	
	c->type=Tbox;
	c->typeA=TAarray1;
	
	c->put=cachePut_box;
	c->get=cacheGet_box;
	c->flush=cacheFlush_box;
	c->clear=cacheClear_box;
	
	c->real_array=a;
	c->real_cache=malloc(sizeof(BoxHeader));
	boxFromNative((BoxHeader*)(c->real_cache), a->objSize);
}

void cacheBoxFromArray2(Cache* c, array2* a)
{
	assert(c);
	assert(a);
	
	c->type=Tbox;
	c->typeA=TAarray2;
	
	c->put=cachePut_box;
	c->get=cacheGet_box;
	c->flush=cacheFlush_box;
	c->clear=cacheClear_box;
	
	c->real_array=a;
	c->real_cache=malloc(sizeof(BoxHeader));
	boxFromNative((BoxHeader*)(c->real_cache), a->objSize);
}

void cacheFree(Cache* c)
{
	assert(c);
	
	c->put=NULL;
	c->get=NULL;
	c->flush=NULL;
	c->clear=NULL;
	
	switch (c->type)
	{
		case Tbox: {
			boxFree((BoxHeader*)(c->real_cache));
			free(c->real_cache);
		} break;
		default: {
			assert(0);
		} break;
	}
	
	c->type=Terror;
	
	c->real_array=NULL;
}

void ncacheBoxFromnarray(ncache* c, narray* na)
{
	assert(c);
	assert(na);
	
	c->type = Tbox;
	
	c->put = ncachePut_box;
	c->get = ncacheGet_box;
	c->getInBuffer = ncacheGetInBuffer_box;
	c->putLine = ncachePutLine_box;
	c->getLine = ncacheGetLine_box;
	c->getLineInBuffer = ncacheGetLineInBuffer_box;
	c->putBlock = ncachePutBlock_box;
	c->getBlock = ncacheGetBlock_box;
	c->getBlockInBuffer = ncacheGetBlockInBuffer_box;
	c->flush = ncacheFlush_box;
	c->clear = ncacheClear_box;
	
	c->array = na;
	c->real_cache = malloc(sizeof(BoxHeader));
	boxFromNative((BoxHeader*)(c->real_cache), na->objSize);
}

void ncacheFree(ncache* c)
{
	assert(c);
	
	c->put = NULL;
	c->get = NULL;
	c->getInBuffer = NULL;
	c->putLine = NULL;
	c->getLine = NULL;
	c->getLineInBuffer = NULL;
	c->putBlock = NULL;
	c->getBlock = NULL;
	c->getBlockInBuffer = NULL;
	c->flush = NULL;
	c->clear = NULL;
	
	switch (c->type)
	{
		case Tbox: {
			boxFree((BoxHeader*)(c->real_cache));
			free(c->real_cache);
		} break;
		default: {
			assert(0);
		} break;
	}
	
	c->type = Terror;
	
	c->array = NULL;
}

void cacheMapFlush_box(void* obj, long long index, BoxHeader *bh, void* ext)
{
	Cache* c=(Cache*)ext;
	//CacheItem* ci=(CacheItem*)obj;
	//isLocal check
	/*switch (c->typeA)
	{
		case TAarray1: {
				if(array1IsLocal((array1*)(c->real_array), index))
			return;
		} break;
		case TAarray2: {
			array2* ar2=(array2*)(c->real_array);
			int x = index;
			int y = index % ar2->;
				if(array2IsLocal(ar2, index))
			return;
		} break;
		case TAarray21: {
				if(array11IsLocal(((array21*)(c->real_array))->arr1, index))
			return;
		} break;
		default: {
			assert(0);
		} break;
	}*/
	
	abstractPointer p;
	int x=0;
	int y=0;
	switch(c->typeA)
	{
		case TAarray1:p=newP1D(index);break;
		case TAarray2:
		{
			x=(int)(index / ((array2*)(c->real_array))->sizeY);
			y=(int)(index % ((array2*)(c->real_array))->sizeY);
			
			p=newP2D(x,y);
		}break;
		default:assert(0);break;
		
	}
	
	cachePut_box(c,p, obj/*ci->data*/, F_THROUGH | F_COPYCACHE);
	
}

void cachePut_box(Cache *c, abstractPointer p, void* send, unsigned flags)
{
	assert(c);
	assert(send);
	assert(c->type==Tbox);
	assert((c->typeA==TAarray1)&&(p.dim==1)  ||  (c->typeA==TAarray2)&&(p.dim==2));
	
	//long long i = p.pointer.p1.i;
	long long cacheIndex;
	switch(p.dim)
	{
		case 1:cacheIndex=p.pointer.p1.i;break;
		case 2:cacheIndex=p.pointer.p2.x * ((array2*)(c->real_array))->sizeY + p.pointer.p2.y;break;
		default: assert(0);break;
	}
	
	char F_THROUGH_SET = flags&F_THROUGH;
	char F_INCACHE_SET = flags&F_INCACHE;
	char F_COPYCACHE_SET = flags&F_COPYCACHE;
	BoxHeader* realc=(BoxHeader*)(c->real_cache);
	char isIn = (boxGet(realc, cacheIndex) != NULL);
	
	printf("cachePut_box(%d)  :  F_THROUGH_SET=%d  F_INCACHE_SET=%d  F_COPYCACHE_SET=%d  isIn=%d\n", cacheIndex, F_THROUGH_SET, F_INCACHE_SET, F_COPYCACHE_SET, isIn);
	
	if(isIn)
		if(F_THROUGH_SET)
		{
			if(F_COPYCACHE_SET)
				boxPut(realc, cacheIndex, send);
			else
				boxPutNoCopy(realc, cacheIndex, send);
			
			switch(c->typeA)
			{
				case TAarray1:array1Put((array1*)(c->real_array), p.pointer.p1.i, send);break;
				case TAarray2:array2Put((array2*)(c->real_array), p.pointer.p2.x, p.pointer.p2.y, send);break;
				default: assert(0);break;
			}
			
		}
		else
		{
			if(F_COPYCACHE_SET)
				boxPut(realc, cacheIndex, send);
			else
				boxPutNoCopy(realc, cacheIndex, send);
		}
	else
		if(F_INCACHE_SET)
		{
			if(F_COPYCACHE_SET)
				boxPut(realc, cacheIndex, send);
			else
				boxPutNoCopy(realc, cacheIndex, send);
			
			switch(c->typeA)
			{
				case TAarray1:array1Put((array1*)(c->real_array), p.pointer.p1.i, send);break;
				case TAarray2:array2Put((array2*)(c->real_array), p.pointer.p2.x, p.pointer.p2.y, send);break;
				default: assert(0);break;
			}
		}
		else
		{
			switch(c->typeA)
			{
				case TAarray1:array1Put((array1*)(c->real_array), p.pointer.p1.i, send);break;
				case TAarray2:array2Put((array2*)(c->real_array), p.pointer.p2.x, p.pointer.p2.y, send);break;
				default: assert(0);break;
			}
		}
	
}

void cacheGet_box(Cache *c, abstractPointer p, void** recv, unsigned flags)
{
	assert(c);
	assert(recv);
	assert(c->type==Tbox);
	assert((c->typeA==TAarray1)&&(p.dim==1)  ||  (c->typeA==TAarray2)&&(p.dim==2));
	
	//long long i = p.pointer.p1.i;
	long long cacheIndex;
	switch(p.dim)
	{
		case 1:cacheIndex=p.pointer.p1.i;break;
		case 2:cacheIndex=p.pointer.p2.x * ((array2*)(c->real_array))->sizeY + p.pointer.p2.y;break;
		default: assert(0);break;
	}
	
	char F_THROUGH_SET=flags&F_THROUGH;
	char F_INCACHE_SET=flags&F_INCACHE;
	char F_COPYCACHE_SET=1;//flags&F_COPYCACHE;
	BoxHeader* realc=(BoxHeader*)(c->real_cache);
	char isIn= (boxGet(realc, cacheIndex) != NULL);
	int realDataSize=    ((array1*)(c->real_array))->objSize;
	
	printf("cacheGet_box(%d)  :  F_THROUGH_SET=%d  F_INCACHE_SET=%d  F_COPYCACHE_SET=%d  isIn=%d\n", cacheIndex, F_THROUGH_SET, F_INCACHE_SET, F_COPYCACHE_SET, isIn);
	
	if(isIn)
		if(F_THROUGH_SET)
		{
			switch(c->typeA)
			{
				case TAarray1:array1Get((array1*)(c->real_array), p.pointer.p1.i, recv);break;
				case TAarray2:array2Get((array2*)(c->real_array), p.pointer.p2.x, p.pointer.p2.y, recv);break;
				default: assert(0);break;
			}
			
			if(F_COPYCACHE_SET)
				boxPut(realc, cacheIndex, *recv);
			else
				boxPutNoCopy(realc, cacheIndex, *recv);
		}
		else
		{
			if(F_COPYCACHE_SET)
			{
				void* temp = boxGet(realc, cacheIndex);
				*recv = malloc(realDataSize);
				memcpy(*recv,temp, realDataSize);
			}
			else
				*recv = boxGet(realc, cacheIndex);
		}
	else
		if(F_INCACHE_SET)
		{
			switch(c->typeA)
			{
				case TAarray1:array1Get((array1*)(c->real_array), p.pointer.p1.i, recv);break;
				case TAarray2:array2Get((array2*)(c->real_array), p.pointer.p2.x, p.pointer.p2.y, recv);break;
				default: assert(0);break;
			}
			
			if(F_COPYCACHE_SET)
				boxPut(realc, cacheIndex, *recv);
			else
				boxPutNoCopy(realc, cacheIndex, *recv);
		}
		else
		{
			switch(c->typeA)
			{
				case TAarray1:array1Get((array1*)(c->real_array), p.pointer.p1.i, recv);break;
				case TAarray2:array2Get((array2*)(c->real_array), p.pointer.p2.x, p.pointer.p2.y, recv);break;
				default: assert(0);break;
			}
		}
	
}

void cacheFlush_box(Cache *c)
{
	assert(c);
	assert(c->type==Tbox);
	
	boxMapAll((BoxHeader*)(c->real_cache), cacheMapFlush_box, c);
}

void cacheClear_box(Cache *c)
{
	assert(c);
	assert(c->type==Tbox);
	
	boxFree((BoxHeader*)(c->real_cache));
	int real_size;
	switch(c->typeA)
	{
		case TAarray1:real_size=((array1*)(c->real_array))->objSize;break;
		case TAarray2:real_size=((array2*)(c->real_array))->objSize;break;
		default:assert(0);break;
		
	}
	boxFromNative((BoxHeader*)(c->real_cache), real_size);
	
}

long long ncacheIndexToGlobalOffset(ncache *c, int index[])
{
	assert(c);
	assert(index);
	
	int i;
	long long result;
	
	for(i=0; i<(c->array->dimsCount - 1); i++)
	{
		result += index[i];
		result *= c->array->sizes[i+1];
	}
	result += index[i];
	
	return result;
}

void ncacheGlobalOffsetToIndex(ncache *c, long long offset, int index[])
{
	assert(c);
	assert(index);
	
	int i;
	
	for(i=(c->array->dimsCount - 1); i>=0; i++)
	{
		index[i] = offset % c->array->sizes[i];
		offset = offset / c->array->sizes[i];
	}
}

void ncacheIncIndex(ncache *c, int index[], int sizes[])
{
	assert(c);
	assert(index);
	assert(sizes);
	
	int i;
	
	for(i=c->array->dimsCount-1; i>=0; i--)
	{
		index[i] += 1;
		if(index[i] < sizes[i])
			break;
		else
			index[i] = 0;
	}
	
	//assert(index[i] >= sizes[i]);
	
}

void ncacheIndexBlockToGlobalOffset(ncache *c, int index[], int sizes[], long long *result)
{
	assert(c);
	assert(index);
	assert(sizes);
	assert(result);
	
	int i;
	int totalSize = 1;
	int* index2 = malloc(sizeof(int) * c->array->dimsCount);
	
	for(i=0; i<c->array->dimsCount; i++)
	{
		assert((sizes[i] > 0) && (sizes[i] <= c->array->sizes[i]));
		totalSize *= sizes[i];
		index2[i] = index[i];
	}
	
	for(i=0; i<totalSize; i++)
	{
		result[i] = ncacheIndexToGlobalOffset(c, index2);
		ncacheIncIndex(c, index2, sizes);
	}
	
	free(index2);
}

void ncachePut_box(ncache *c, int index[], void* send, unsigned flags)
{
	assert(c);
	assert(index);
	assert(send);
	assert(c->type == Tbox);
	
	long long cacheIndex = ncacheIndexToGlobalOffset(c, index);
	
	char F_THROUGH_SET = flags & F_THROUGH;
	char F_INCACHE_SET = flags & F_INCACHE;
	char F_COPYCACHE_SET = flags & F_COPYCACHE;
	
	BoxHeader* realCache = (BoxHeader*)(c->real_cache);
	char isIn = (boxGet(realCache, cacheIndex) != NULL);
	
	printf("ncachePut_box(%d)  :  F_THROUGH_SET=%d  F_INCACHE_SET=%d  F_COPYCACHE_SET=%d  isIn=%d\n", cacheIndex, F_THROUGH_SET, F_INCACHE_SET, F_COPYCACHE_SET, isIn);
	
	if(isIn)
		if(F_THROUGH_SET)
		{
			if(F_COPYCACHE_SET)
				boxPut(realCache, cacheIndex, send);
			else
				boxPutNoCopy(realCache, cacheIndex, send);
			
			narrayPut(c->array, index, send);
		}
		else
		{
			if(F_COPYCACHE_SET)
				boxPut(realCache, cacheIndex, send);
			else
				boxPutNoCopy(realCache, cacheIndex, send);
		}
	else
		if(F_INCACHE_SET)
		{
			if(F_COPYCACHE_SET)
				boxPut(realCache, cacheIndex, send);
			else
				boxPutNoCopy(realCache, cacheIndex, send);
			
			narrayPut(c->array, index, send);
		}
		else
		{
			narrayPut(c->array, index, send);
		}
	
}

void ncacheGet_box(ncache *c, int index[], void** recv, unsigned flags)
{
	assert(c);
	assert(index);
	assert(recv);
	assert(c->type == Tbox);
	
	long long cacheIndex = ncacheIndexToGlobalOffset(c, index);
	
	char F_THROUGH_SET = flags & F_THROUGH;
	char F_INCACHE_SET = flags & F_INCACHE;
	char F_COPYCACHE_SET = flags & F_COPYCACHE;
	//char F_COPYCACHE_SET = 1;
	
	BoxHeader* realCache = (BoxHeader*)(c->real_cache);
	char isIn = (boxGet(realCache, cacheIndex) != NULL);
	
	int realDataSize = c->array->objSize;
	
	printf("ncacheGet_box(%d)  :  F_THROUGH_SET=%d  F_INCACHE_SET=%d  F_COPYCACHE_SET=%d  isIn=%d\n", cacheIndex, F_THROUGH_SET, F_INCACHE_SET, F_COPYCACHE_SET, isIn);
	
	if(isIn)
		if(F_THROUGH_SET)
		{
			narrayGet(c->array, index, recv);
			
			if(F_COPYCACHE_SET)
				boxPut(realCache, cacheIndex, *recv);
			else
				boxPutNoCopy(realCache, cacheIndex, *recv);
		}
		else
		{
			if(F_COPYCACHE_SET)
			{
				void* temp = boxGet(realCache, cacheIndex);
				*recv = malloc(realDataSize);
				memcpy(*recv, temp, realDataSize);
			}
			else
				*recv = boxGet(realCache, cacheIndex);
		}
	else
		if(F_INCACHE_SET)
		{
			narrayGet(c->array, index, recv);
			
			if(F_COPYCACHE_SET)
				boxPut(realCache, cacheIndex, *recv);
			else
				boxPutNoCopy(realCache, cacheIndex, *recv);
		}
		else
		{
			narrayGet(c->array, index, recv);
		}
}

void ncacheGetInBuffer_box(ncache *c, int index[], void** recv, unsigned flags)
{
	assert(c);
	assert(index);
	assert(recv);
	assert(c->type == Tbox);
	
	long long cacheIndex = ncacheIndexToGlobalOffset(c, index);
	
	char F_THROUGH_SET = flags & F_THROUGH;
	char F_INCACHE_SET = flags & F_INCACHE;
	//char F_COPYCACHE_SET = flags & F_COPYCACHE;
	char F_COPYCACHE_SET = 1;//cos it is getInBuffer
	
	BoxHeader* realCache = (BoxHeader*)(c->real_cache);
	char isIn = (boxGet(realCache, cacheIndex) != NULL);
	
	int realDataSize = c->array->objSize;
	
	printf("ncacheGetInBuffer_box(%d)  :  F_THROUGH_SET=%d  F_INCACHE_SET=%d  F_COPYCACHE_SET=%d  isIn=%d\n", cacheIndex, F_THROUGH_SET, F_INCACHE_SET, F_COPYCACHE_SET, isIn);
	
	if(isIn)
		if(F_THROUGH_SET)
		{
			narrayGetInBuffer(c->array, index, recv);
			
			if(F_COPYCACHE_SET)
			{
				boxPut(realCache, cacheIndex, recv);
			}
			else
			{
				//copy_cache should be always set
				//boxPutNoCopy(realCache, cacheIndex, recv);
			}
		}
		else
		{
			if(F_COPYCACHE_SET)
			{
				void* temp = boxGet(realCache, cacheIndex);
				memcpy(recv, temp, realDataSize);
			}
			else
			{
				//recv = boxGet(realCache, cacheIndex);
				//nothing actually
			}
		}
	else
		if(F_INCACHE_SET)
		{
			narrayGetInBuffer(c->array, index, recv);
			
			if(F_COPYCACHE_SET)
			{
				boxPut(realCache, cacheIndex, recv);
			}
			else
			{
				//recv = boxGet(realCache, cacheIndex);
				//nothing actually
			}
		}
		else
		{
			narrayGetInBuffer(c->array, index, recv);
		}
}

void ncachePutLine_box(ncache *c, int index[], void* send, int size, unsigned flags)
{
	assert(c);
	assert(index);
	assert(send);
	assert(size > 0);
	assert(size <= c->array->sizes[c->array->dimsCount - 1]);
	assert(c->type == Tbox);
	
	char F_THROUGH_SET = flags & F_THROUGH;
	char F_INCACHE_SET = flags & F_INCACHE;
	char F_COPYCACHE_SET = flags & F_COPYCACHE;
	
	BoxHeader* realCache = (BoxHeader*)(c->real_cache);
	int realDataSize = c->array->objSize;
	
	int i;
	long long* cacheIndexes = malloc(sizeof(long long) * size);
	ncacheIndexBlockToGlobalOffset(c, index, &size, cacheIndexes);
	
	int isIn = 0;
	for(i=0; i<size; i++)
		isIn = isIn || (boxGet(realCache, cacheIndexes[i]) != NULL);
	
	printf("ncachePutLine_box  :  F_THROUGH_SET=%d  F_INCACHE_SET=%d  F_COPYCACHE_SET=%d  isIn=%d\n", F_THROUGH_SET, F_INCACHE_SET, F_COPYCACHE_SET, isIn);
	
	if(isIn)
		if(F_THROUGH_SET)
		{
			if(F_COPYCACHE_SET)
				for(i=0; i<size; i++)
					boxPut(realCache, cacheIndexes[i], send + i*realDataSize);
			else
				for(i=0; i<size; i++)
					boxPutNoCopy(realCache, cacheIndexes[i], send + i*realDataSize);
			
			narrayPutLine(c->array, index, send, size);
		}
		else
		{
			if(F_COPYCACHE_SET)
				for(i=0; i<size; i++)
					boxPut(realCache, cacheIndexes[i], send + i*realDataSize);
			else
				for(i=0; i<size; i++)
					boxPutNoCopy(realCache, cacheIndexes[i], send + i*realDataSize);
		}
	else
		if(F_INCACHE_SET)
		{
			if(F_COPYCACHE_SET)
				for(i=0; i<size; i++)
					boxPut(realCache, cacheIndexes[i], send + i*realDataSize);
			else
				for(i=0; i<size; i++)
					boxPutNoCopy(realCache, cacheIndexes[i], send + i*realDataSize);
			
			narrayPutLine(c->array, index, send, size);
		}
		else
		{
			narrayPutLine(c->array, index, send, size);
		}
	
	free(cacheIndexes);
}

void ncacheGetLine_box(ncache *c, int index[], void** recv, int size, unsigned flags)
{
	assert(c);
	assert(index);
	assert(recv);
	assert(c->type == Tbox);
	assert(size > 0);
	assert(size <= c->array->sizes[c->array->dimsCount - 1]);
	
	char F_THROUGH_SET = flags & F_THROUGH;
	char F_INCACHE_SET = flags & F_INCACHE;
	//char F_COPYCACHE_SET = flags & F_COPYCACHE;
	char F_COPYCACHE_SET = 1;
	
	BoxHeader* realCache = (BoxHeader*)(c->real_cache);
	
	int i;
	
	long long* cacheIndexes = malloc(sizeof(long long) * size);
	ncacheIndexBlockToGlobalOffset(c, index, &size, cacheIndexes);
	
	int isIn = 0;
	for(i=0; i<size; i++)
		isIn = isIn || (boxGet(realCache, cacheIndexes[i]) != NULL);
	
	int realDataSize = c->array->objSize;
	
	printf("ncacheGetLine_box  :  F_THROUGH_SET=%d  F_INCACHE_SET=%d  F_COPYCACHE_SET=%d  isIn=%d\n", F_THROUGH_SET, F_INCACHE_SET, F_COPYCACHE_SET, isIn);
	
	if(isIn)
		if(F_THROUGH_SET)
		{
			narrayGetLine(c->array, index, recv, size);
			
			if(F_COPYCACHE_SET)
				for(i=0; i<size; i++)
					boxPut(realCache, cacheIndexes[i], *recv + i*realDataSize);
			else
				for(i=0; i<size; i++)
					boxPutNoCopy(realCache, cacheIndexes[i], *recv + i*realDataSize);
		}
		else
		{
			if(F_COPYCACHE_SET)
			{
				*recv = malloc(realDataSize * size);
				memset(*recv, 0, realDataSize * size);
				
				void* ptr;
				for(i=0; i<size; i++)
				{
					ptr = boxGet(realCache, cacheIndexes[i]);
					if(ptr != NULL)
						memcpy(*recv + i*realDataSize, ptr, realDataSize);
				}
			}
			else
			{
				//nothing to do. unreachable
				//*recv = boxGet(realCache, cacheIndex);
			}
		}
	else
		if(F_INCACHE_SET)
		{
			narrayGetLine(c->array, index, recv, size);
			
			if(F_COPYCACHE_SET)
				for(i=0; i<size; i++)
					boxPut(realCache, cacheIndexes[i], *recv + i*realDataSize);
			else
				for(i=0; i<size; i++)
					boxPutNoCopy(realCache, cacheIndexes[i], *recv + i*realDataSize);
		}
		else
		{
			narrayGetLine(c->array, index, recv, size);
		}
	
	free(cacheIndexes);
}

void ncacheGetLineInBuffer_box(ncache *c, int index[], void* recv, int size, unsigned flags)
{
	assert(c);
	assert(index);
	assert(recv);
	assert(c->type == Tbox);
	assert(size > 0);
	assert(size <= c->array->sizes[c->array->dimsCount - 1]);
	
	char F_THROUGH_SET = flags & F_THROUGH;
	char F_INCACHE_SET = flags & F_INCACHE;
	//char F_COPYCACHE_SET = flags & F_COPYCACHE;
	char F_COPYCACHE_SET = 1;
	
	BoxHeader* realCache = (BoxHeader*)(c->real_cache);
	
	int i;
	
	long long* cacheIndexes = malloc(sizeof(long long) * size);
	ncacheIndexBlockToGlobalOffset(c, index, &size, cacheIndexes);
	
	int isIn = 0;
	for(i=0; i<size; i++)
		isIn = isIn || (boxGet(realCache, cacheIndexes[i]) != NULL);
	
	int realDataSize = c->array->objSize;
	
	printf("ncacheGetLineInBuffer_box  :  F_THROUGH_SET=%d  F_INCACHE_SET=%d  F_COPYCACHE_SET=%d  isIn=%d\n", F_THROUGH_SET, F_INCACHE_SET, F_COPYCACHE_SET, isIn);
	
	if(isIn)
		if(F_THROUGH_SET)
		{
			narrayGetLineInBuffer(c->array, index, recv, size);
			
			if(F_COPYCACHE_SET)
				for(i=0; i<size; i++)
					boxPut(realCache, cacheIndexes[i], recv + i*realDataSize);
			else
			{
				//should do nothing
				/*for(i=0; i<size; i++)
					boxPutNoCopy(realCache, cacheIndexes[i], *recv + i*realDataSize);*/
			}
		}
		else
		{
			if(F_COPYCACHE_SET)
			{
				memset(recv, 0, realDataSize * size);
				
				void* ptr;
				for(i=0; i<size; i++)
				{
					ptr = boxGet(realCache, cacheIndexes[i]);
					if(ptr != NULL)
						memcpy(recv + i*realDataSize, ptr, realDataSize);
				}
			}
			else
			{
				//nothing to do. unreachable
				//recv = boxGet(realCache, cacheIndex);
			}
		}
	else
		if(F_INCACHE_SET)
		{
			narrayGetLineInBuffer(c->array, index, recv, size);
			
			if(F_COPYCACHE_SET)
				for(i=0; i<size; i++)
					boxPut(realCache, cacheIndexes[i], recv + i*realDataSize);
			else
			{
				//should do nothing
				/*for(i=0; i<size; i++)
					boxPutNoCopy(realCache, cacheIndexes[i], recv + i*realDataSize);*/
			}
		}
		else
		{
			narrayGetLineInBuffer(c->array, index, recv, size);
		}
	
	free(cacheIndexes);
}

void ncachePutBlock_box(ncache *c, int index[], void* send, int sizes[], unsigned flags)
{
	assert(c);
	assert(send);
	assert(index);
	assert(sizes != NULL);
	
	int i;
	int j;
	
	for(i=0; i<c->array->dimsCount; i++)
		assert((sizes[i] > 0) && (sizes[i] <= c->array->sizes[i]));
	
	int* index2 = malloc(sizeof(int) * c->array->dimsCount);
	int linesCount = 1;
	int lineOffset = 0; // in terms of objects, not bytes
	
	for(i=0; i<c->array->dimsCount; i++)
	{
		index2[i] = index[i];
		linesCount *= sizes[i];
	}
	linesCount /= sizes[c->array->dimsCount - 1];
	
	for(i=0; i<linesCount; i++)
	{
		//make index2 point to line[i]
		int lineNum = i;
		
		for(j = c->array->dimsCount-2 ; j >= 0; j--)
		{
			index2[j] = (lineNum % sizes[j]);
			lineNum = lineNum / sizes[j];
		}
		index2[c->array->dimsCount - 1] = 0;
		
		for(j=0; j<c->array->dimsCount; j++)
		{
			index2[j] += index[j];
		}
		
		ncachePutLine_box(c, index2, send + lineOffset*c->array->objSize, sizes[c->array->dimsCount - 1], flags);
		lineOffset += sizes[c->array->dimsCount - 1];
	}
	
	free(index2);
}

void ncacheGetBlock_box(ncache *c, int index[], void** recv, int sizes[], unsigned flags)
{
	assert(c);
	assert(recv);
	assert(index);
	assert(sizes);
	
	int i;
	int blockSize = 1;
	
	for(i=0; i<c->array->dimsCount; i++)
	{
		assert((sizes[i] > 0) && (sizes[i] <= c->array->sizes[i]));
		blockSize *= sizes[i];
	}
	
	*recv = malloc(c->array->objSize * blockSize);
	
	ncacheGetBlockInBuffer_box(c, index, recv, sizes, flags);
}

void ncacheGetBlockInBuffer_box(ncache *c, int index[], void* recv, int sizes[], unsigned flags)
{
	assert(c);
	assert(recv);
	assert(index);
	assert(sizes != NULL);
	
	int i;
	int j;
	
	for(i=0; i<c->array->dimsCount; i++)
		assert((sizes[i] > 0) && (sizes[i] <= c->array->sizes[i]));
	
	int* index2 = malloc(sizeof(int) * c->array->dimsCount);
	int linesCount = 1;
	int lineOffset = 0; // in terms of objects, not bytes
	
	for(i=0; i<c->array->dimsCount; i++)
	{
		index2[i] = index[i];
		linesCount *= sizes[i];
	}
	linesCount /= sizes[c->array->dimsCount - 1];
	
	for(i=0; i<linesCount; i++)
	{
		//make index2 point to line[i]
		int lineNum = i;
		
		for(j = c->array->dimsCount-2 ; j >= 0; j--)
		{
			index2[j] = (lineNum % sizes[j]);
			lineNum = lineNum / sizes[j];
		}
		index2[c->array->dimsCount - 1] = 0;
		
		for(j=0; j<c->array->dimsCount; j++)
		{
			index2[j] += index[j];
		}
		
		ncacheGetLineInBuffer_box(c, index2, recv + lineOffset*c->array->objSize,sizes[c->array->dimsCount - 1], flags);
		
		lineOffset += sizes[c->array->dimsCount - 1];
	}
	
	free(index2);
}

void ncacheMapFlush_box(void* obj, long long index, BoxHeader *bh, void* ext)
{
	ncache* c = (ncache*)ext;
	
	int* index2 = malloc(sizeof(int) * c->array->dimsCount);
	
	ncacheGlobalOffsetToIndex(c, index, index2);
	ncachePut_box(c, index2, obj, F_THROUGH | F_COPYCACHE);
	
	free(index2);
}

void ncacheFlush_box(ncache *c)
{
	assert(c);
	assert(c->type == Tbox);
	
	boxMapAll((BoxHeader*)(c->real_cache), ncacheMapFlush_box, c);
}

void ncacheClear_box(ncache *c)
{
	assert(c);
	assert(c->type == Tbox);
	
	boxFree((BoxHeader*)(c->real_cache));
	int real_size = c->array->objSize;
	
	boxFromNative((BoxHeader*)(c->real_cache), real_size);
}

void cacheFence(Cache *c)
{
	assert(c);
	
	switch(c->typeA)
	{
		case TAarray1:array1Fence((array1*)(c->real_array));break;
		case TAarray2:array2Fence((array2*)(c->real_array));break;
		default:assert(0);break;
	}
}

void ncacheFence(ncache *c)
{
	assert(c);
	narrayFence(c->array);
}
