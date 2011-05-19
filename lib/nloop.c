#include "nloop.h"

void nloopFromTopo(nloop *loop, Topology topo, int sizes[], int dimsCount, npointer (*map)(int[], narray*), void (*alloc)(narray*), nloop_work_func func, nloop_depend_func depend)
{
	assert(loop);
	assert(func);
	assert(depend);
	
	loop->status = malloc(sizeof(narray));
	
	narrayFromRange(loop->status, topo, sizes, dimsCount, 1, map, alloc);
	
	loop->remoteStatuses = malloc(sizeof(BoxHeader));
	boxFromNative(loop->remoteStatuses, 1);
	
	loop->func = func;
	loop->depend = depend;
	loop->localDone = 0;
	
	loop->topo.type = topo.type;
	loop->topo.obj = topo.obj;
}

void nloopFree(nloop *loop)
{
	assert(loop);
	
	loop->func = NULL;
	loop->depend = NULL;
	loop->localDone = -1;
	
	loop->topo.type = Tnone;
	loop->topo.obj = NULL;
	
	if(loop->status)
	{
		narrayFree(loop->status);
		free(loop->status);
	}
	if(loop->remoteStatuses)
	{
		boxFree(loop->remoteStatuses);
		free(loop->remoteStatuses);
	}
}

void nloopOffsetToIndex(nloop *loop, int offset, int index[])
{
	assert(loop);
	assert(index);
	assert(offset >= 0);
	
	int i;
	
	for(i=(loop->status->dimsCount - 1); i>=0; i++)
	{
		index[i] = offset % loop->status->sizes[i];
		index[i] += loop->status->index[i];
		offset = offset / loop->status->sizes[i];
	}
}

long long nloopIndexToGlobalOffset(nloop *loop, int index[])
{
	assert(loop);
	assert(index);
	
	int i;
	long long result;
	
	for(i=0; i<(loop->status->dimsCount - 1); i++)
	{
		result += index[i];
		result *= loop->status->sizes[i+1];
	}
	result += index[i];
	
	return result;
}

int nloopDo(nloop *loop, int index[], void* data)
{
	assert(loop);
	assert(index);
	assert(data);
	
	npointer p;
	p = loop->status->map(index, loop->status);
	
	if(p.id != loop->status->id)
		return 0;
	
	int result = loop->func(loop, index, data);
	
	*((char*)(loop->status->data + p.offset)) = (char) result;
	
	return result;
}

int nloopDoWithDeps(nloop *loop, int index[], void* data, int askRemote)
{
	assert(loop);
	assert(index);
	assert(data);
	
	npointer p;
	p = loop->status->map(index, loop->status);
	
	if(p.id != loop->status->id)
		return 0;
	
	char statSelf = *((char*)(loop->status->data + p.offset));
	
	if(statSelf)
		return statSelf; 
	
	int* indexDep = malloc(sizeof(int) * loop->status->dimsCount);
	int ret;
	int ok = 1;
	
	while((ret = loop->depend(loop, index, indexDep, data)))
	{
		
		if(askRemote)
		{
			int statDep = nloopCheckStatus(loop, indexDep);
			ok = ok && (statDep != -1) && (statDep != 0);
		}
		else
		{
			npointer pDep = loop->status->map(indexDep, loop->status);
			char statDep = *((char*)(loop->status->data + p.offset));
			if(!statDep)
				return 0;
		}
	}
	
	free(indexDep);
	
	if(!ok)
		return 0;
	
	int result = loop->func(loop, index, data);
	*((char*)(loop->status->data + p.offset)) = (char) result;
	return result;
}

int nloopDoLocal(nloop *loop, void* data)
{
	assert(loop);
	assert(index);
	assert(data);
	
	int i;
	char* statuses = malloc(sizeof(char) * loop->status->totalSize);
	int* index = malloc(sizeof(int) * loop->status->dimsCount);
	
	for(i=0; i<loop->status->totalSize; i++)
	{
		nloopOffsetToIndex(loop, i, index);
		nloopDoWithDeps(loop, index, data, 1);
	}
	
	nloopStatusFence(loop);
	
	for(i=0; i<loop->status->totalSize; i++)
	{
		nloopOffsetToIndex(loop, i, index);
		nloopDoWithDeps(loop, index, data, 0);
	}
	
	free(index);
	free(statuses);
}

int nloopCheckStatus(nloop *loop, int index[])
{
	assert(loop);
	assert(index);
	
	npointer p;
	p = loop->status->map(index, loop->status);
	
	if(p.id == loop->status->id)
		return *((char*)(loop->status->data + p.offset));
	
	
	char stat = -1;
	char* possibleStat = NULL;
	
	long long glIndex = ncacheIndexToGlobalOffset(loop, index);
	possibleStat =  boxGet(loop->remoteStatuses, glIndex);
	
	if((!possibleStat) || (*possibleStat == -1) || (*possibleStat == 0))
	{
		boxPut(loop->remoteStatuses, glIndex, &stat);
		narrayGetInBuffer(loop->status, index, boxGet(loop->remoteStatuses, glIndex));
	}
	else
	{
		return *possibleStat;
	}
	
	return -1;
}

int nloopClearStatus(nloop *loop)
{
	//call fence before
	assert(loop);
	Topology topo;
	topo.type = loop->status->topo.type;
	topo.obj = loop->status->topo.obj;
	int dimsCount = loop->status->dimsCount;
	
	npointer (*map)(int[], narray*);
	void (*alloc)(narray*);
	
	map = loop->status->map;
	alloc = loop->status->alloc;
	
	int* sizes = malloc(sizeof(int) * dimsCount);
	
	int i;
	for(i=0; i<dimsCount; i++)
		sizes[i] = loop->status->sizes[i];
	
	narrayFree(loop->status);
	
	narrayFromRange(loop->status, topo, sizes, dimsCount, 1, map, alloc);
	
	boxFree(loop->remoteStatuses);
	boxFromNative(loop->remoteStatuses, 1);
	
	free(sizes);
}


void nloopStatusFence(nloop* loop)
{
	assert(loop);
	
	narrayFence(loop->status);
}

void nloopBarrier(nloop* loop)
{
	assert(loop);
	
	narrayBarrier(loop->status);
}


