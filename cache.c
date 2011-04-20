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
	
	char F_THROUGH_SET=flags&F_THROUGH;
	char F_INCACHE_SET=flags&F_INCACHE;
	char F_COPYCACHE_SET=flags&F_COPYCACHE;
	BoxHeader* realc=(BoxHeader*)(c->real_cache);
	char isIn= (boxGet(realc, cacheIndex) != NULL);
	
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
