/*
 *  fgrid3.h
 *  
 *
 *  Created by arch on 15.11.09.//hehe i'll let him be proud of himself
 *  Copyright 2009 МГУ. All rights reserved.
 *
 */

#ifndef __FGRID3
#define __FGRID3

#include "defs.h"

typedef struct fgrid3temp
{
	int (*map)(int, int, int, struct fgrid3temp *g);
	void(*reverse)(struct fgrid3temp *g);
	int id;
	MPI_Comm comm;
	int x, y, z;
	int width, height, length;
	
	MPI_Comm xSide;
	MPI_Comm ySide;
	MPI_Comm zSide;
	MPI_Comm xyLine;
	MPI_Comm yzLine;
	MPI_Comm xzLine;
	
	int xSideSelf;
	int ySideSelf;
	int zSideSelf;
	int xyLineSelf;
	int yzLineSelf;
	int xzLineSelf;
	
	Topo topo;
	
} fgrid3;

void fgrid3CreateXYLine(fgrid3 * g);
void fgrid3CreateYZLine(fgrid3 * g);
void fgrid3CreateXZLine(fgrid3 * g);
void fgrid3CreateXSide(fgrid3 * g);
void fgrid3CreateYSide(fgrid3 * g);
void fgrid3CreateZSide(fgrid3 * g);
int fgrid3_native(int x, int y, int z, fgrid3 *fg);
void fgrid3_reverse(fgrid3 *g);

void fgrid3FromRange(fgrid3 * g, MPI_Comm comm,int x, int y, int z);
void fgrid3FromNative(fgrid3 * g);
void fgrid3Slice(fgrid3 * g,fgrid3 * ng, int dim, int sliceIndex);
void fgrid3SliceLinear(fgrid3 * g,fgrid3 * ng, int xSlice, int ySlice, int zSlice);

void fgrid3Free(fgrid3 * g);

void fgrid3Barrier(fgrid3 * g);
void fgrid3BarrierX(fgrid3 * g);
void fgrid3BarrierY(fgrid3 * g);
void fgrid3BarrierZ(fgrid3 * g);
void fgrid3BarrierXY(fgrid3 * g);
void fgrid3BarrierYZ(fgrid3 * g);
void fgrid3BarrierXZ(fgrid3 * g);

#endif
