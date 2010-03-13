#include "fgrid2.h"

void fgrid2_reverse(fgrid2 *g)
{
	assert(g);
	
	g->x = (int)(g->id / g->w);
	g->y = g->id % g->w;
}

void fgrid2Slice(fgrid2 * g,fgrid2 * ng, int dim, int sliceIndex)
{
	assert(g);
	assert(ng);
	assert((dim>=0)&&(dim<2));
	assert((sliceIndex>=0) && ((dim==0) ? (sliceIndex < g->h) : (sliceIndex < g->w)));
	
	if(dim==0)
	{
		//slice on x
		ng->w=g->w;
		
		if(g->x<sliceIndex)
			ng->h=sliceIndex;//in first part
		else
			ng->h=g->h-sliceIndex;//in second part
		
		MPI_Comm_split(g->comm, g->x <sliceIndex, 0, &(ng->comm));
	}
	else
	{
		//slice on y
		ng->h=g->h;
		
		if(g->y<sliceIndex)
			ng->w=sliceIndex;//in first part
		else
			ng->w=g->w-sliceIndex;//in second part
		
		MPI_Comm_split(g->comm, g->y <sliceIndex, 0, &(ng->comm));
	}
	
	MPI_Comm_rank(ng->comm,&(ng->id));
	
	ng->topo.obj=g;
	ng->topo.type=Tfgrid2;
	
	ng->reverse=g->reverse;
	ng->reverse(ng);
	ng->map=g->map;
	
	//!!!!!!!!!!lines
	fgrid2CreateXLine(ng);
	fgrid2CreateYLine(ng);
	//!!!!!!!!!!untested
	
	
}

void fgrid2SliceLinear(fgrid2 * g,fgrid2 * ng, int xSlice, int ySlice)
{
	assert(g);
	assert(ng);
	assert(xSlice>0);
	assert(ySlice>0);
	assert((g->h % xSlice) == 0);
	assert((g->w % ySlice) == 0);
	
	ng->h=xSlice;
	ng->w=ySlice;
	int ww=g->w / ySlice;
	
	MPI_Comm_split(g->comm, (g->x / xSlice)*ww + (g->y / ySlice), 0, &(ng->comm));
	
	MPI_Comm_rank(ng->comm,&(ng->id));
	
	ng->topo.obj=g;
	ng->topo.type=Tfgrid2;
	
	ng->reverse=g->reverse;
	ng->reverse(ng);
	ng->map=g->map;
	
	//!!!!!!!!!!lines
	fgrid2CreateXLine(ng);
	fgrid2CreateYLine(ng);
	//!!!!!!!!!!untested
}

void fgrid2SliceParts(fgrid2 * g,fgrid2 * ng, int* xParts, int* yParts, int xSize, int ySize)
{
	assert(g);
	assert(ng);
	assert(xParts);
	assert(yParts);
	assert(xSize>0);
	assert(ySize>0);
	
	int i;
	int* xSum=malloc(sizeof(int) * xSize);
	int* ySum=malloc(sizeof(int) * ySize);
	
	xSum[0]=0;
	ySum[0]=0;
	
	for(i=1;i<xSize;i++)
		xSum[i]=xParts[i-1]+xSum[i-1];
	for(i=1;i<ySize;i++)
		ySum[i]=yParts[i-1]+ySum[i-1];
	
	assert( ! (g->h - xSum[xSize-1] - xParts[xSize-1]) );
	assert( ! (g->w - ySum[ySize-1] - yParts[ySize-1]) );
	
	free(xSum);
	free(ySum);
}

int fgrid2_native(int x,int y, fgrid2 *fg)
{
	assert((x<fg->h) && (y<fg->w) && (x>=0) && (y>=0));//?
	return x*fg->w+y;
}

void fgrid2CreateXLine(fgrid2 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->x, 0, &(g->xLine));
	MPI_Comm_rank(g->xLine,&(g->xLineSelf));
}

void fgrid2CreateYLine(fgrid2 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->y, 0, &(g->yLine));
	MPI_Comm_rank(g->yLine,&(g->yLineSelf));
}

void fgrid2FromRange(fgrid2 * g,MPI_Comm comm,int x, int y)
{
	assert(g);
	assert((x>0) && (y>0));
	
	int numNodes;
	MPI_Comm_size(comm,&numNodes);
	assert(x*y==numNodes);
	
	g->w=y;
	g->h=x;
	
	MPI_Comm_dup(comm, &(g->comm));
	MPI_Comm_rank(g->comm,&(g->id));
	
	g->topo.obj=NULL;
	g->topo.type=Tnone;
	
	g->reverse=(void(*)(fgrid2 *g)) (fgrid2_reverse);
	g->reverse(g);
	g->map=(int(*)(int,int,fgrid2 *)) (fgrid2_native);
	
	fgrid2CreateXLine(g);
	fgrid2CreateYLine(g);
	
}

void fgrid2FromNative(fgrid2 * g)
{
	assert(g);
	
	fgrid2FromRange(g,MPI_COMM_WORLD,nativeX,nativeY);
	
	g->topo.obj=NULL;
	g->topo.type=Tnone;
}

void fgrid2Barrier(fgrid2 * g)
{
	assert(g);
	MPI_Barrier(g->comm);
}

void fgrid2Free(fgrid2 * g)
{
	assert(g);
	
	MPI_Comm_free(&(g->xLine));
	MPI_Comm_free(&(g->yLine));
	MPI_Comm_free(&(g->comm));
	
	g->xLineSelf=-1;
	g->yLineSelf=-1;
	
	g->id=-1;
	g->x=g->y=-1;
	g->w=g->h=-1;
	
	g->map=NULL;
	g->reverse=NULL;
	
	g->topo.type=Tnone;
	g->topo.obj=NULL;
}
