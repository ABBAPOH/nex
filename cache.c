#include "cache.h"

void cacheBoxFromArray1(cache* c, array1* a)
{
	assert(c);
	assert(a);
	
	c->type=Tbox;
	
	c->put=cachePut_box;
	c->get=cacheGet_box;
	c->flush=cacheFlush_box;
	
	c->real_array=a;
	c->real_cache=malloc(sizeof(BoxHeader));
	boxFromNative((BoxHeader*)(c->real_cache), a->objSize);
}

void cacheFree(cache* c)
{
	assert(c);
	
	c->put=NULL;
	c->get=NULL;
	c->flush=NULL;
	
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
	cache* c=(cache*)ext;
	cacheItem* ci=(cacheItem*)obj;
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
	
}

void cachePut_box(cache *c, long long i, void* send, unsigned flags)
{
	assert(c);
	assert(c->type==Tbox);
	
}

void cacheGet_box(cache *c, long long i, void** recv, unsigned flags)
{
	assert(c);
	assert(c->type==Tbox);
	
}

void cacheFlush_box(cache *c)
{
	assert(c);
	assert(c->type==Tbox);
	
	boxMapAll((BoxHeader*)(c->real_cache), cacheMapFlush_box, c);
	
}
