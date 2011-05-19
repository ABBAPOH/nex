#ifndef __FGRIDN
#define __FGRIDN

#include "defs.h"

typedef struct fgridntemp
{
	int (*map)(int* index, struct fgridntemp *g);
	void(*reverse)(struct fgridntemp *g);
	int id;
	MPI_Comm comm;
	int dimsCount;
	int* index;
	int* sizes;
	int totalSize;
	char* indexString;//for debug
	
	MPI_Comm* subDims;
	int* subDimsSelf;
	int subdimsCount;
	unsigned int dimsMask;
	
	Topology topology;
	
} fgridn;

void fgridnCreateSubDim(fgridn * g, int* dims);
void fgridnCreateSubDimMask(fgridn * g, unsigned int dims);
void fgridnMkIndexString(fgridn * g);

int fgridn_native(int* index, fgridn *fg);
void fgridn_reverse(fgridn *g);

void fgridnFromRange(fgridn* g, MPI_Comm comm, int dimsCount, int* sizes, int(*map)(int*, fgridn *), void(*reverse)(fgridn *));
void fgridnFromNative(fgridn* g);
Topology fgridnGetTopology(fgridn* g);

void fgridnSlice(fgridn * g,fgridn * ng, int dim, int sliceIndex);
void fgridnSliceLinear(fgridn * g,fgridn * ng, int* sliceSteps);
void fgridnSliceParts(fgridn * g, fgridn * ng, int** parts, int* size);

void fgridnDivide(fgridn * g,fgridn * ng, int dim, int divideStep);

void fgridnFree(fgridn * g);

void fgridnBarrier(fgridn * g);
void fgridnBarrierSub(fgridn * g, int dims[]);
void fgridnBarrierSubMask(fgridn * g, unsigned int dims);

unsigned int fgridnBoolArrayToMask(fgridn * g, int dims[]);
#endif
