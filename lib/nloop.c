#include "nloop.h"

void nloopFromTopo(nloop *loop, Topology topo, int sizes[], int dimsCount, npointer (*map)(int[], narray*), void (*alloc)(narray*), nloop_work_func func, nloop_depend_func depend)
{
	
}

void nloopFree(nloop *loop)
{
	
}

int nloopDo(nloop *loop, int index[], void* data)
{
	
}

int nloopDoWithDeps(nloop *loop, int index[], void* data)
{
	
}

int nloopDoLocal(nloop *loop, void* data)
{
	
}

int nloopCheckStatus(nloop *loop, int index[], int* result)
{
	
}

int nloopClearStatus(nloop *loop)
{
	
}


void nloopStatusFence(nloop* loop)
{
	
}

void nloopBarrier(nloop* loop)
{
	
}


