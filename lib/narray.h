#ifndef __NARRAY
#define __NARRAY

#include "defs.h"
#include "box.h"
#include "fgridn.h"
#include "fgrid3.h"
#include "fgrid2.h"
#include "grid2.h"

typedef struct 
{
	MPI_Comm comm; 
	int id;
	unsigned offset;
} npointer;

typedef struct narraytemp 
{
	npointer (*map)(int index[], struct narraytemp *g);
	void(*alloc)(struct narraytemp *g);
	int id;
	MPI_Comm comm; 
	
	int* nodes;
	int totalNodes;
	
	int objSize;
	int* sizes;
	int totalSize;
	int dimsCount;
	
	int* index;
	int* thisSizes;
	int thisTotalSize;
	void* data;
	MPI_Win win;
	Topology topo;
} narray;

npointer narray_map(int index[], narray* na);
void narray_alloc(narray* na);
void printNPointer(npointer p);

void narrayFromRange(narray *na, Topology topo, int sizes[], int dimsCount, int objSize, npointer (*map)(int[], narray*), void (*alloc)(narray*));
void narrayFree(narray *na);

void narrayPut(narray* na, int index[], void* send);
void narrayPutLine(narray* na, int index[], void* send, int size);
void narrayPutBlock(narray* na, int index[], void* send, int sizes[]);

void narrayGet(narray* na, int index[], void** recv);
void narrayGetLine(narray* na, int index[], void** recv, int size);
void narrayGetBlock(narray* na, int index[], void** recv, int sizes[]);

void narrayGetInBuffer(narray* na, int index[], void* recv);
void narrayGetLineInBuffer(narray* na, int index[], void* recv, int size);
void narrayGetBlockInBuffer(narray* na, int index[], void* recv, int sizes[]);

void narrayFence(narray* na);
void narrayBarrier(narray* na);

#endif
