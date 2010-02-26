/*
 *  fgrid3.c
 *  
 *
 *  Created by arch on 15.11.09. //hehe i'll let him be proud of himself
 *  Copyright 2009 МГУ. All rights reserved.
 *
 */

#include "fgrid3.h"

void fgrid3_reverse(fgrid3 *g)
{
	assert(g);
	//g->x = (g->id / g->w) % g->h;
	//g->y = (g->id % g->w);
	//g->z = (g->id) / (g->w * g->h);
	g->x = (g->id) / (g->w * g->l);
	//g->y = (g->id / g->l) % g->w;
	g->y = (g->id / g->l) % g->w;
	g->z = (g->id) % (g->w) % (g->l);
	
}

int fgrid3_native(int x, int y, int z, const fgrid3 *fg)
{
	assert((x<fg->h) && (y<fg->w) && (z<fg->l) && (x>=0) && (y>=0) && (x>=0));//?
	//return z*fg->w*fg->h + x*fg->w+y;
	return x*fg->w*fg->l + y*fg->l+z;
	
}

void fgrid3CreateXYLine(fgrid3 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->x*g->w + g->y, 0, &(g->xyLine));
	MPI_Comm_rank(g->xyLine,&(g->xyLineSelf));
}

void fgrid3CreateYZLine(fgrid3 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->y*g->l + g->z, 0, &(g->yzLine));
	MPI_Comm_rank(g->yzLine,&(g->yzLineSelf));
}

void fgrid3CreateXZLine(fgrid3 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->x*g->l + g->z, 0, &(g->xzLine));
	MPI_Comm_rank(g->xzLine,&(g->xzLineSelf));
}

void fgrid3CreateXSide(fgrid3 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->x, 0, &(g->xSide));
	MPI_Comm_rank(g->xSide,&(g->xSideSelf));
	
}

void fgrid3CreateYSide(fgrid3 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->y, 0, &(g->ySide));
	MPI_Comm_rank(g->ySide,&(g->ySideSelf));
}

void fgrid3CreateZSide(fgrid3 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->z, 0, &(g->zSide));
	MPI_Comm_rank(g->zSide,&(g->zSideSelf));
}

void fgrid3Slice(fgrid3 * g,fgrid3 * ng, int dim, int sliceIndex)
{
	assert(g);
	assert(ng);
	assert((dim>=0)&&(dim<3));
	assert(sliceIndex>=0);
	switch (dim)
	{
		case 0: assert(sliceIndex < g->h); break;
		case 1: assert(sliceIndex < g->w); break;
		case 2: assert(sliceIndex < g->l); break;
		default: assert(0);
	}
	
	switch(dim)
	{
		case 0:{
			//slice on x
			ng->w=g->w;
			ng->l=g->l;
			
			if(g->x<sliceIndex)
				ng->h=sliceIndex;//in first part
			else
				ng->h=g->h-sliceIndex;//in second part
			
			MPI_Comm_split(g->comm, g->x <sliceIndex, 0, &(ng->comm));
		}break;
		
		case 1:{
			//slice on y
			ng->h=g->h;
			ng->l=g->l;
			
			if(g->y<sliceIndex)
				ng->w=sliceIndex;//in first part
			else
				ng->w=g->w-sliceIndex;//in second part
			
			MPI_Comm_split(g->comm, g->y <sliceIndex, 0, &(ng->comm));
		}break;
		
		case 2:{
			//slice on y
			ng->h=g->h;
			ng->w=g->w;
			
			if(g->z<sliceIndex)
				ng->l=sliceIndex;//in first part
			else
				ng->l=g->l-sliceIndex;//in second part
			
			MPI_Comm_split(g->comm, g->z <sliceIndex, 0, &(ng->comm));
		}break;
		
	}
	
	MPI_Comm_rank(ng->comm,&(ng->id));
	
	ng->topo.obj=g;
	ng->topo.type=Tfgrid3;
	
	ng->reverse=g->reverse;
	ng->reverse(ng);
	ng->map=g->map;
	
	//!!!!!!!!!!lines
	fgrid3CreateXYLine(ng);
	fgrid3CreateYZLine(ng);
	fgrid3CreateXZLine(ng);
	fgrid3CreateXSide(ng);
	fgrid3CreateYSide(ng);
	fgrid3CreateZSide(ng);
	//!!!!!!!!!!untested
	
	
}

void fgrid3FromRange(fgrid3 * g, MPI_Comm comm, int x, int y, int z)
{
	assert(g);
	assert((x>0) && (y>0) && (z>0));
	
	int numNodes;
	MPI_Comm_size(comm,&numNodes);
	assert(x*y*z==numNodes);
	
	g->w=y;
	g->h=x;
	g->l=z;
	
	MPI_Comm_dup(comm, &(g->comm));
	MPI_Comm_rank(g->comm,&(g->id));
	
	g->topo.obj=NULL;
	g->topo.type=Tnone;
	
	g->reverse=(void(*)(fgrid3 *g)) (fgrid3_reverse);
	g->reverse(g);
	g->map=(int(*)(int,int,int,fgrid3 *)) (fgrid3_native);
	
	fgrid3CreateXYLine(g);
	fgrid3CreateYZLine(g);
	fgrid3CreateXZLine(g);
	
	fgrid3CreateXSide(g);
	fgrid3CreateYSide(g);
	fgrid3CreateZSide(g);
	
	
}

void fgrid3FromNative(fgrid3 * g)
{
	
	fgrid3FromRange(g,MPI_COMM_WORLD,nativeX,nativeY,1);
	
	g->topo.obj=NULL;
	g->topo.type=Tnone;
}

void fgrid3Barrier(fgrid3 * g)
{
	assert(g);
	MPI_Barrier(g->comm);
}

void fgrid3Free(fgrid3 * g)
{
	assert(g);
	
	MPI_Comm_free(&(g->xSide));
	MPI_Comm_free(&(g->ySide));
	MPI_Comm_free(&(g->zSide));
	
	MPI_Comm_free(&(g->xyLine));
	MPI_Comm_free(&(g->yzLine));
	MPI_Comm_free(&(g->xzLine));
	
	g->xSideSelf=-1;
	g->ySideSelf=-1;
	g->zSideSelf=-1;
	g->xyLineSelf=-1;
	g->yzLineSelf=-1;
	g->xzLineSelf=-1;
	
	g->id=-1;
	g->x=g->y=g->z=-1;
	g->w=g->h=g->l=-1;
	
	g->map=NULL;
	g->reverse=NULL;
	
	g->topo.type=Tnone;
	g->topo.obj=NULL;
	
	MPI_Comm_free(&(g->comm));
	
}
