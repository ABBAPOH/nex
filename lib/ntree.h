#ifndef __NTREE
#define __NTREE

#include "defs.h"
#include "fgrid2.h"
#include "fgrid3.h"
#include "fgridn.h"

typedef struct ntreetemp {
	int id;
	int pid; 
	MPI_Comm comm;  
	int num; 
	struct ntreetemp *parent; 
	Topology topo;
} ntree;

void ntreeFromNative(ntree * t);
void ntreeFromRange(ntree * t, MPI_Comm comm, int pid, ntree * parent);
void ntreeFromfgrid2(ntree * t, fgrid2* fg);
void ntreeFromfgrid2Trans(ntree * t, fgrid2* fg);
void ntreeFromfgrid2Param(ntree * t, fgrid2* fg, int orient, int toproot, int bottomroot);
void ntreeFromfgrid3(ntree * t, fgrid3* fg);
void ntreeFromfgridn(ntree * t, fgridn* fg, unsigned int dims[], int dimRoots[], int dimCount);
void ntreeFromfgridnNums(ntree * t, fgridn* fg, unsigned int dimNums[], int dimRoots[]);

void ntreeFree(ntree * t);

void ntreeBCast(ntree * t, void* buffer, int count, MPI_Datatype datatype);
void ntreeReduce(ntree * t, void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op);
int ntreeGather(ntree * t, void *sendbuf, int sendcount, MPI_Datatype datatype, void **recvbuf);
void ntreeScatter(ntree * t, void *sendbuf, int sendcount, MPI_Datatype datatype, void *recvbuf);
int ntreeIsMegaRoot(ntree * t);
void ntreeBarrier(ntree * t);

#endif
