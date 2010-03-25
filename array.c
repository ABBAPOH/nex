#include "array.h"

/*!
  \fn void factorizeWithRatio(int nodes, int w, int h, int* x, int* y)
  \brief Maps array[\a w][\a h] and number of \a nodes to size (\a x and \a y) of local part (in current process) of array

*/
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

/*!
  \fn pointer array1_map(int i, array1 *a)
  \brief Returns pointer to process containing a[i] element and local index in this process (a[i] -> {pid, offset})

*/
pointer array1_map(long long i, array1 *a)
{
	assert(a);
	assert((i>=0) && (i<a->size));
	pointer p;
	p.comm=a->comm;
	p.id=(int)(i / a->thisSize);
	p.index=i % a->thisSize;
	
	return p;
}

/*!
  \fn void array1_alloc(array1 *a)
  \brief Allocates memory for local part of array \a a.

*/
void array1_alloc(array1 *a)
{
	assert(a);
	assert(a->size % a->nodes == 0);
	
	a->thisSize=(int)(a->size / a->nodes);
	a->thisStart=a->thisSize * a->id;
	
	MPI_Alloc_mem( a->thisSize * a->objSize, MPI_INFO_NULL, &(a->data) );

	MPI_Win_create(a->data,a->thisSize * a->objSize,a->objSize,MPI_INFO_NULL,a->comm,&(a->win));
}

/*!
  \fn pointer array2_map(int x, int y, array2 *a)
  \brief Returns pointer to process containing a[x][y] element and local index in this process (a[x][y] -> {pid, offset}).

*/
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

/*!
  \fn void array2_alloc(array2 *a)
  \brief Allocates memory for local part of array \a a.

*/
void array2_alloc(array2 *a)
{
	assert(a);
	assert( (a->sizeX * a->sizeY) % a->nodes ==0);

	switch(a->topo.type)
	{
		case Tnone:
		{
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

long long array21_map(int x, int y, array21 *a)
{
	assert(a);
	assert((x>=0) && (x<a->sizeX));
	assert((y>=0) && (y<a->sizeY));
	
	return x*a->sizeY + y;
}

/*!
  \fn void array1Free(array1 *a)
  \brief Frees array \a a.

  Fills data of an array with -1
*/
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

/*!
  \fn void array2Free(array2 *a)
  \brief Frees array \a a.

  Fills data of an array with -1
*/
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

void array21Free(array21 *a)
{
	assert(a);
	
	a->sizeX=a->sizeY=-1;
	a->map=NULL;
	a->arr1=NULL;
	
	a->topo.type=Tnone;
	a->topo.obj=NULL;
}

/*!
  \fn void array1FromRange(array1 *a, MPI_Comm comm, int size, int objSize)
  \brief Creates one-dimensional array with size \a size and element size \a objSize from communicator \a comm.

*/
void array1FromRange(array1 *a, MPI_Comm comm, long long size, int objSize)
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
	a->map=(pointer(*)(long long i,array1 *a)) (array1_map);
	
}

/*!
  \fn void array2FromRange(array2 *a, MPI_Comm comm, int sizeX, int sizeY, int objSize)
  \brief Creates two-dimensional array with size \a sizeX*\a sizeY and element size \a objSize from communicator \a comm.

*/
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

void array21FromArray1(array21 *a, array1 *a1, int sizeX, int sizeY)
{
	assert(a);
	assert(a1);
	assert(sizeX>0);
	assert(sizeY>0);
	assert((long long)sizeX * (long long)sizeY == a1->size);
	a->sizeX=sizeX;
	a->sizeY=sizeY;
	
	a->topo.obj=NULL;
	a->topo.type=Tnone;
	
	a->arr1=a1;
	
	a->map=(long long(*)(int x, int y, array21 *a)) (array21_map);
}

/*!
  \fn void array1Put(array1 *a, int i, void *send)
  \brief Sends element \a send to \a a[\a i]

*/
void array1Put(array1 *a, long long i, void *send)
{
	assert(a);
	assert(send);
	pointer p=a->map(i,a);
	
	MPI_Put(send, a->objSize, MPI_CHAR, p.id, p.index, a->objSize, MPI_CHAR, a->win);
	
}

/*!
  \fn void array2Put(array2 *a, int x, int y, void *send)
  \brief Sends element \a send to \a a[\a x][\a y]

*/
void array2Put(array2 *a, int x, int y, void *send)
{
	assert(a);
	assert(send);
	pointer p=a->map(x,y,a);
	
	MPI_Put(send, a->objSize, MPI_CHAR, p.id, p.index, a->objSize, MPI_CHAR, a->win);
	
}

void array21Put(array21 *a, int x, int y, void *send)
{
	assert(a);
	assert(send);
	
	array1Put(a->arr1, a->map(x, y, a), send);
}

/*!
  \fn void array1Get(array1 *a, int i,void**recv)
  \brief Gets element \a a[\a i] to \a recv

*/
void array1Get(array1 *a, long long i, void**recv)
{
	assert(a);
	assert(recv);
	
	pointer p=a->map(i,a);
	*recv=malloc(a->objSize);
	
	MPI_Get(*recv, a->objSize, MPI_CHAR, p.id, p.index, a->objSize, MPI_CHAR, a->win);
}

/*!
  \fn void array2Get(array2 *a, int x, int y, void**recv)
  \brief Gets element \a a[\a x][\a y] to \a recv

*/
void array2Get(array2 *a, int x, int y, void**recv)
{
	assert(a);
	assert(recv);
	
	pointer p=a->map(x,y,a);
	*recv=malloc(a->objSize);
	
	MPI_Get(*recv, a->objSize, MPI_CHAR, p.id, p.index, a->objSize, MPI_CHAR, a->win);
}

void array21Get(array21 *a, int x, int y, void**recv)
{
	assert(a);
	assert(recv);
	
	array1Get(a->arr1, a->map(x, y, a), recv);
}

/*!
  \fn void array1Fence(array1 *a)
  \brief Wrapper over MPI_Win_fence.

  NOTE: all put/get operations must be surrounded with *Fence functions!

*/
void array1Fence(array1 *a)
{
	assert(a);
	MPI_Win_fence(0, a->win);
}

/*!
  \fn void array2Fence(array2 *a)
  \brief Wrapper over MPI_Win_fence.

  NOTE: all put/get operations must be surrounded with *Fence functions!

*/
void array2Fence(array2 *a)
{
	assert(a);
	MPI_Win_fence(0, a->win);
}

void array21Fence(array21 *a)
{
	assert(a);
	array1Fence(a->arr1);
}

int array1IsLocal(array1 *a, long long i)
{
	assert(a);
	return a->map(i, a).id == a->id;
}

int array2IsLocal(array2 *a, int x, int y)
{
	assert(a);
	return a->map(x, y, a).id == a->id;
}

int array21IsLocal(array21 *a, int x, int y)
{
	assert(a);
	return a->arr1->map(a->map(x, y, a), a->arr1).id == a->arr1->id;
}

