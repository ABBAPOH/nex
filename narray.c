#include "narray.h"

npointer narray1_map(int i, narray1 *na)
{
	assert(na);
	assert((i>=0) && (i<na->size));
	npointer p;
	p.comm=na->comm;
	p.id=(int)(i / na->thisSize);
	p.index=i % na->thisSize;
	p.bindex=0;
	
	return p;
}

void narray1_alloc(narray1 *na)
{
	/*
	assert(na);
	assert(na->size % na->nodes == 0);
	
	na->thisSize=(int)(na->size / na->nodes);
	na->thisStart=na->thisSize * na->id;
	
	MPI_Alloc_mem( na->thisSize * na->objSize, MPI_INFO_NULL, &(na->data) );

	MPI_Win_create(na->data,na->thisSize * na->objSize,na->objSize,MPI_INFO_NULL,na->comm,&(na->win));
	*/
}

void narray1FromRange(narray1 *na, MPI_Comm comm, int size, int objSize)
{
	/*
	assert(na);
	assert(size>0);
	
	MPI_Comm_dup(comm, &(na->comm));
	na->size=size;
	na->data=NULL;
	na->objSize=objSize;
	MPI_Comm_rank(na->comm,&(na->id));
	MPI_Comm_size(na->comm,&(na->nodes));
	
	////////////////////////////////////////////////////
	na->topo.obj=NULL;
	na->topo.type=Tnone;
	
	na->alloc=(void(*)(narray1 *na)) (narray1_alloc);
	na->alloc(na);
	na->map=(npointer(*)(int i,narray1 *na)) (narray1_map);
	*/
}

void narray1Free(narray1 *na)
{
	/*
	assert(na);
	
	if(na->data!=NULL)
	{
		MPI_Win_free(&(na->win));
		MPI_Free_mem(na->data);
		na->data=NULL;
	}
	MPI_Comm_free(&(na->comm));
	
	na->id=-1;
	na->size=na->nodes=-1;
	na->thisSize=na->thisStart=-1;
	na->objSize=-1;
	
	na->map=NULL;
	na->alloc=NULL;
	
	na->topo.type=Tnone;
	na->topo.obj=NULL;
	*/
}

void narray1Put(narray1 *na, int i, void *send)
{
	/*
	assert(na);
	assert(send);
	npointer p=na->map(i,na);
	
	MPI_Put(send, na->objSize, MPI_CHAR, p.id, p.index, na->objSize, MPI_CHAR, na->win);
	*/
}

void narray1Get(narray1 *na, int i,void**recv)
{
	/*
	assert(na);
	assert(recv);
	
	npointer p=na->map(i,na);
	*recv=malloc(na->objSize);
	
	MPI_Get(*recv, na->objSize, MPI_CHAR, p.id, p.index, na->objSize, MPI_CHAR, na->win);
	*/
}

void narray1Fence(narray1 *na)
{
	/*
	assert(na);
	MPI_Win_fence(0, na->win);
	*/
}
