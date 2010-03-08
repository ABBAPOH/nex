#include "array.h"

pointer array1_map(int i, array1 *a)
{
	assert(a);
	assert((i>=0) && (i<a->size));
	pointer p;
	p.comm=a->comm;
	p.id=(int)(i / a->thisSize);
	p.index=i % a->thisSize;
	
	return p;
}

void array1_alloc(array1 *a)
{
	assert(a);
	assert(a->size % a->nodes == 0);
	
	a->thisSize=(int)(a->size / a->nodes);
	a->thisStart=a->thisSize * a->id;
	
	MPI_Alloc_mem( a->thisSize * a->objSize, MPI_INFO_NULL, &(a->data) );

	MPI_Win_create(a->data,a->thisSize * a->objSize,a->objSize,MPI_INFO_NULL,a->comm,&(a->win));
}

pointer array2_map(int x, int y, array2 *a)
{
	assert(a);
	assert((x>=0) && (x<a->sizeX));
	assert((y>=0) && (y<a->sizeY));
	pointer p;
	p.comm=a->comm;
	
	int idX=(int)(x / a->thisSizeX);
	int idY=(int)(y / a->thisSizeY);
	
	p.id=((int)(a->sizeY / a->thisSizeY)) * idX + idY;
	
	int indexX=x % a->thisSizeX;
	int indexY=y % a->thisSizeY;
	
	p.index=indexX * a->thisSizeY + indexY;
	
	return p;
}

void factorizeWithRatio(int nodes, int w, int h, int* x, int* y)
{
	//bug
	//wh - array size; size - num nodes for array; xy - array dims(result)
	assert(x);
	assert(y);
	assert((nodes>0) && (w>0) && (h>0));
	
	//simple variant
	assert((w * h) % nodes == 0);
	
	int perX=nodes/w;
	int perY=nodes/h;
	int perNode=(int)(w*h)/nodes;//3
	int nodeX;
	int nodeY;
	
	int i=(int)(sqrt(perNode*h/w)); //closest to optimal nodeY //2
	int j=i;
	int res=-1;
	
	//printf("i=%d\n",i);
	
	while((i>1) || ((j<=perNode)))
	{
		if(i && ((perNode%i)==0) && /*((perX%i)==0) &&*/ ((perY%(perNode/i))==0))
		{
			res=i;
			break;
		}
		if(j && ((perNode%j)==0) && /*((perX%j)==0) &&*/ ((perY%(perNode/j))==0))
		{
			res=j;
			break;
		}
		if(i>1) i--;
		if(j<=perNode) j++;
	}
	
	if(res==-1)
		res=1;
	
	nodeY=res;
	nodeX=perNode/res;
	
	*y=nodeX;
	*x=nodeY;
	
}

void array2_alloc(array2 *a)
{
	assert(a);
	assert( (a->sizeX * a->sizeY) % a->nodes ==0);
	
	switch(a->topo.type)
	{
		case Tnone:
		{
			printf("HERE\n");
			factorizeWithRatio(a->nodes,a->sizeY,a->sizeX,&(a->thisSizeX),&(a->thisSizeY));
			int xmax=a->sizeX / a->thisSizeX;
			int ymax=a->sizeY / a->thisSizeY;
			
			a->x=(int) ((a->id)/ymax);
			a->y=(int) ((a->id)%ymax);
			
			a->thisStartX=a->thisSizeX * a->x;
			a->thisStartY=a->thisSizeY * a->y;
		}	break;
		
		case Tfgrid2:
		{
			assert(0);//not writen
		}	break;
		
		default:
		{
			assert(0);//not writen
		}	break;
		
	}
	
	MPI_Alloc_mem( a->thisSizeX * a->thisSizeY * a->objSize, MPI_INFO_NULL, &(a->data) );

	MPI_Win_create(a->data,a->thisSizeX * a->thisSizeY * a->objSize,a->objSize,MPI_INFO_NULL,a->comm,&(a->win));
}

void array1FromRange(array1 *a, MPI_Comm comm, int size, int objSize)
{
	assert(a);
	assert(size>0);
	
	MPI_Comm_dup(comm, &(a->comm));
	a->size=size;
	a->data=NULL;
	a->objSize=objSize;
	MPI_Comm_rank(a->comm,&(a->id));
	MPI_Comm_size(a->comm,&(a->nodes));
	
	////////////////////////////////////////////////////
	a->topo.obj=NULL;
	a->topo.type=Tnone;
	
	a->alloc=(void(*)(array1 *a)) (array1_alloc);
	a->alloc(a);
	a->map=(pointer(*)(int i,array1 *a)) (array1_map);
	
}

void array2FromRange(array2 *a, MPI_Comm comm, int sizeX, int sizeY, int objSize)
{
	assert(a);
	assert(sizeX>0);
	assert(sizeY>0);
	
	MPI_Comm_dup(comm, &(a->comm));
	a->sizeX=sizeX;
	a->sizeY=sizeY;
	a->data=NULL;
	a->objSize=objSize;
	MPI_Comm_rank(a->comm,&(a->id));
	MPI_Comm_size(a->comm,&(a->nodes));
	
	a->topo.obj=NULL;
	a->topo.type=Tnone;
	
	a->alloc=(void(*)(array2 *a)) (array2_alloc);
	a->alloc(a);
	a->map=(pointer(*)(int x, int y,array2 *a)) (array2_map);
}

void array1Free(array1 *a)
{
	assert(a);
	
	if(a->data!=NULL)
	{
		MPI_Win_free(&(a->win));
		MPI_Free_mem(a->data);
		a->data=NULL;
	}
	MPI_Comm_free(&(a->comm));
	
	a->id=-1;
	a->size=a->nodes=-1;
	a->thisSize=a->thisStart=-1;
	a->objSize=-1;
	
	a->map=NULL;
	a->alloc=NULL;
	
	a->topo.type=Tnone;
	a->topo.obj=NULL;
}

void array2Free(array2 *a)
{
	assert(a);
	
	if(a->data!=NULL)
	{
		MPI_Win_free(&(a->win));
		MPI_Free_mem(a->data);
		a->data=NULL;
	}
	MPI_Comm_free(&(a->comm));
	
	a->id=-1;
	a->y=-1;
	a->x=-1;
	a->sizeX=a->sizeY=a->nodes=-1;
	a->thisSizeX=a->thisStartX=a->thisSizeY=a->thisStartY=-1;
	a->objSize=-1;
	
	a->map=NULL;
	a->alloc=NULL;
	
	a->topo.type=Tnone;
	a->topo.obj=NULL;
}

void array1Put(array1 *a, int i, void *send)
{
	assert(a);
	assert(send);
	pointer p=a->map(i,a);
	
	MPI_Put(send, a->objSize, MPI_CHAR, p.id, p.index, a->objSize, MPI_CHAR, a->win);
	
}

void array2Put(array2 *a, int x, int y, void *send)
{
	assert(a);
	assert(send);
	pointer p=a->map(x,y,a);
	
	MPI_Put(send, a->objSize, MPI_CHAR, p.id, p.index, a->objSize, MPI_CHAR, a->win);
	
}

void array1Get(array1 *a, int i,void**recv)
{
	assert(a);
	assert(recv);
	
	pointer p=a->map(i,a);
	*recv=malloc(a->objSize);
	
	MPI_Get(*recv, a->objSize, MPI_CHAR, p.id, p.index, a->objSize, MPI_CHAR, a->win);
}

void array2Get(array2 *a, int x, int y,void**recv)
{
	assert(a);
	assert(recv);
	
	pointer p=a->map(x,y,a);
	*recv=malloc(a->objSize);
	
	MPI_Get(*recv, a->objSize, MPI_CHAR, p.id, p.index, a->objSize, MPI_CHAR, a->win);
}

void array1Fence(array1 *a)
{
	assert(a);
	MPI_Win_fence(0, a->win);	
}

void array2Fence(array2 *a)
{
	assert(a);
	MPI_Win_fence(0, a->win);	
}
