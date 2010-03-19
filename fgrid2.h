#ifndef __FGRID2
#define __FGRID2

#include "defs.h"

typedef struct fgrid2temp
{
	int (*map)(int, int, struct fgrid2temp *g);
	void(*reverse)(struct fgrid2temp *g);
	int id;
	MPI_Comm comm;
	int x,y;
	int width, height;
	MPI_Comm xLine;
	MPI_Comm yLine;
	int xLineSelf,yLineSelf;
	Topo topo;
} fgrid2;

int fgrid2_native(int x,int y, fgrid2 *fg);
void fgrid2_reverse(fgrid2 *g);
void fgrid2CreateXLine(fgrid2 * g);
void fgrid2CreateYLine(fgrid2 * g);

void fgrid2FromRange(fgrid2 * g,MPI_Comm comm, int x, int y);
void fgrid2FromNative(fgrid2 * g);

void fgrid2Free(fgrid2 * g);

void fgrid2Slice(fgrid2 * g,fgrid2 * ng, int dim, int sliceIndex);
void fgrid2SliceLinear(fgrid2 * g,fgrid2 * ng, int xSlice, int ySlice);
void fgrid2SliceParts(fgrid2 * g,fgrid2 * ng, int* xParts, int* yParts, int xSise, int ySize);
void fgrid2Barrier(fgrid2 * g);

#endif
