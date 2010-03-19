#ifndef __NARRAY
#define __NARRAY

#include "defs.h"
#include "box.h"

typedef struct 
{
	MPI_Comm comm; 
	int id;
        unsigned index;
	int bindex;
} npointer;
//win problem
typedef struct narray1temp 
{
	npointer (*map)(int i, struct narray1temp *g); 
	void(*alloc)(struct narray1temp *g); 
	int id; 
	MPI_Comm comm; 
	int objSize; 
	int blockSize; 
	int nodes;
	int size;
	int thisSize;
	BoxHeader box; 
	MPI_Win win; 
	Topo topo;
} narray1;

npointer narray1_map(int i, narray1 *na);
void narray1_alloc(narray1 *na);

void narray1Free(narray1 *na);

void narray1FromRange(narray1 *na, MPI_Comm comm, int size, int objSize);

void narray1Put(narray1 *a,int i, void *send);
void narray1Get(narray1 *a,int i,void**recv);
void narray1Fence(narray1 *a);

#endif
