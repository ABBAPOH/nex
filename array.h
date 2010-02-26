#ifndef __ARRAY
#define __ARRAY

#include "defs.h"

typedef struct {MPI_Comm comm; int id,index;} pointer;

typedef struct array1temp {pointer (*map)(int i, struct array1temp *g); void(*alloc)(struct array1temp *g); int id; MPI_Comm comm; int size,nodes,thisStart,thisSize,objSize; void *data; MPI_Win win; Topo topo;} array1;

typedef struct array2temp {pointer (*map)(int x, int y, struct array2temp *g); void(*alloc)(struct array2temp *g); int id,x,y; MPI_Comm comm; int sizeX,sizeY,nodes,thisStartX,thisSizeX,thisStartY,thisSizeY,objSize; void *data; MPI_Win win; Topo topo;} array2;

void factorizeWithRatio(int nodes, int w, int h, int* x, int* y);

pointer array1_map(int i, array1 *a);
void array1_alloc(array1 *a);
pointer array2_map(int x, int y, array2 *a);
void array2_alloc(array2 *a);

void array1Free(array1 *a);
void array2Free(array2 *a);

void array1FromRange(array1 *a, MPI_Comm comm, int size, int objSize);
void array2FromRange(array2 *a, MPI_Comm comm, int sizeX, int sizeY, int objSize);

void array1Put(array1 *a,int i, void *send);
void array2Put(array2 *a, int x, int y, void *send);
void array1Get(array1 *a,int i,void**recv);
void array2Get(array2 *a, int x, int y,void**recv);
void array1Fence(array1 *a);
void array2Fence(array2 *a);

#endif
