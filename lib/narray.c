#include "narray.h"

npointer narray_map_example(int index[], narray* na)
{
	assert(na);
	assert(index);
	
	int i;
	for(i=0; i<na->dimsCount; i++)
		assert((index[i] >= 0) && (index[i] < na->sizes[i]));
	
	int id;
	int offset;
	npointer pointer;
	
	//here goes calculation of id and offse
	
	pointer.comm = na->comm;
	pointer.id = id;
	pointer.offset = offset;
	
	return pointer;
}

void narray_alloc_example(narray* na)
{
	assert(na);
	assert(na->dimsCount > 0);
	assert(na->nodes == NULL);
	
	int i;
	
	//here goes creation of nodes
	//na->nodes = malloc(sizeof(int) * na->dimsCount);
	
	assert(na->nodes != NULL);
	for(i=0; i<na->dimsCount; i++)
	{
		assert(na->nodes[i] > 0);
		assert((na->sizes[i] % na->nodes[i]) == 0);
	}
	
	//here goes calculation of local sizes
	//plus calculation og thisTotalSize
	
	//here goes calculation of index
}

void printNPointer(npointer p)
{
	printf("pointer: comm = %d id = %2d offset = %3d\n", (int)p.comm, p.id, p.offset);
}

void narrayFromRange(narray* na, Topology topo, int sizes[], int dimsCount, int objSize, npointer (*map)(int[], narray*), void (*alloc)(narray*))
{
	assert(na);
	assert(sizes);
	assert(objSize >= 0);
	assert(map);
	assert(alloc);
	assert(topo.obj != NULL);
	
	MPI_Comm comm;
	
	switch(topo.type)//TOPO
	{
		case Tfgridn: comm = ((fgridn*)(topo.obj))->comm; break;
		case Tfgrid3: comm = ((fgrid3*)(topo.obj))->comm; break;
		case Tfgrid2: comm = ((fgrid2*)(topo.obj))->comm; break;
		case Tgrid2: comm = ((grid2*)(topo.obj))->comm; break;
		case Tnone: comm = MPI_COMM_WORLD; break;
		
		default: assert(0);
	}
	
	int i;
	
	MPI_Comm_dup(comm, &(na->comm));
	na->data = NULL;
	na->objSize = objSize;
	na->dimsCount = dimsCount;
	MPI_Comm_rank(na->comm, &(na->id));
	MPI_Comm_size(na->comm, &(na->totalNodes));
	na->thisTotalSize = -1;
	na->thisSizes = NULL;
	na->index = NULL;
	
	na->totalSize = 1;
	na->sizes = malloc(sizeof(int) * na->dimsCount);
	na->nodes = NULL;
	
	for(i=0; i<na->dimsCount; i++)
	{
		assert(sizes[i] > 0);
		na->sizes[i] = sizes[i];
		na->totalSize *= na->sizes[i];
	}
	
	na->topo.obj = topo.obj;
	na->topo.type = topo.type;
	
	na->alloc = alloc;
	na->alloc(na);
	assert(na->thisTotalSize > 0);
	if(objSize > 0)
	{
		MPI_Alloc_mem(na->thisTotalSize * na->objSize, MPI_INFO_NULL, &(na->data) );
		MPI_Win_create(na->data, na->thisTotalSize * na->objSize, na->objSize, MPI_INFO_NULL, na->comm, &(na->win));
	}
	
	na->map = map;
	
}

void narrayFree(narray* na)
{
	assert(na);
	
	MPI_Comm_free(&(na->comm));
	
	if(na->data != NULL)
	{
		MPI_Win_free(&(na->win));
		MPI_Free_mem(na->data);
		na->data = NULL;
	}
	
	na->objSize = -1;
	na->dimsCount = -1;
	
	na->id = -1;
	na->totalNodes = -1;
	
	na->thisTotalSize = -1;
	if(na->thisSizes != NULL)
	{
		free(na->thisSizes);
		na->thisSizes = NULL;
	}
	if(na->index != NULL)
	{
		free(na->index);
		na->index = NULL;
	}
	
	na->totalSize = -1;
	free(na->sizes);
	//na->thisSize = na->thisStart=-1;
	
	na->map = NULL;
	na->alloc = NULL;
	
	na->topo.type = Tnone;
	na->topo.obj = NULL;
}

void narrayPut(narray* na, int index[], void* send)
{
	assert(na);
	assert(send);
	npointer p = na->map(index, na);
	
	MPI_Put(send, na->objSize, MPI_CHAR, p.id, p.offset, na->objSize, MPI_CHAR, na->win);
}

void narrayPutLine(narray* na, int index[], void* send, int size)
{
	assert(na);
	assert(send);
	assert(index);
	assert(size > 0);
	assert(size <= na->sizes[na->dimsCount - 1]);
	
	int* index2 = malloc(sizeof(int) * na->dimsCount);
	
	int i;
	
	for(i=0; i<na->dimsCount; i++)
		index2[i] = index[i];
	index2[na->dimsCount - 1] = index[na->dimsCount - 1] + (size - 1);
	
	npointer p1 = na->map(index, na);
	npointer p2 = na->map(index2, na);
	
	//printf("%d %d\n", index[0], index[1]);
	//printf("->%d %d\n", index2[0], index2[1]);
	
	//printNPointer(p1);
	//printNPointer(p2);
	
	if(p1.id == p2.id)
	{
		MPI_Put(send, na->objSize * size, MPI_CHAR, p1.id, p1.offset, na->objSize * size, MPI_CHAR, na->win);
	}
	else
	{
		printf("extended put\n");
		
		int sndOffset = 0;
		int sndLength = 1;
		
		for(i=1; i<size; i++)
		{
			index2[na->dimsCount - 1] = index[na->dimsCount - 1] + i;
			p2 = na->map(index2, na);
			if(p1.id == p2.id)
			{
				sndLength++;
			}
			else
			{
				MPI_Put(send + sndOffset * na->objSize, na->objSize * sndLength, MPI_CHAR, p1.id, p1.offset, na->objSize * sndLength, MPI_CHAR, na->win);
				//printf("send %d elems with offset = %d to %d:%d\n", sndLength, sndOffset, p1.id, p1.offset);
				p1 = na->map(index2, na);
				sndOffset += sndLength;
				sndLength = 1;
				
			}
		}
		
		MPI_Put(send + sndOffset * na->objSize, na->objSize * sndLength, MPI_CHAR, p1.id, p1.offset, na->objSize * sndLength, MPI_CHAR, na->win);
		//printf("send %d elems with offset = %d to %d:%d\n", sndLength, sndOffset, p1.id, p1.offset);
	}
	
	free(index2);
}

void narrayPutBlock(narray* na, int index[], void* send, int sizes[])
{
	assert(na);
	assert(send);
	assert(index);
	assert(sizes != NULL);
	
	int i;
	int j;
	
	for(i=0; i<na->dimsCount; i++)
		assert((sizes[i] > 0) && (sizes[i] <= na->sizes[i]));
	
	int* index2 = malloc(sizeof(int) * na->dimsCount);
	int linesCount = 1;
	int lineOffset = 0; // in terms of objects, not bytes
	
	for(i=0; i<na->dimsCount; i++)
	{
		index2[i] = index[i];
		linesCount *= sizes[i];
	}
	linesCount /= sizes[na->dimsCount - 1];
	
	for(i=0; i<linesCount; i++)
	{
		//make index2 point to line[i]
		int lineNum = i;
		
		for(j = na->dimsCount-2 ; j >= 0; j--)
		{
			index2[j] = (lineNum % sizes[j]);
			lineNum = lineNum / sizes[j];
		}
		index2[na->dimsCount - 1] = 0;
		
		for(j=0; j<na->dimsCount; j++)
		{
			index2[j] += index[j];
		}
		
		narrayPutLine(na, index2, send + lineOffset*na->objSize, sizes[na->dimsCount - 1]);
		lineOffset += sizes[na->dimsCount - 1];
	}
	
	free(index2);
}

void narrayGet(narray* na, int index[], void** recv)
{
	assert(na);
	assert(recv);
	
	npointer p = na->map(index, na);
	*recv = malloc(na->objSize);
	
	MPI_Get(*recv, na->objSize, MPI_CHAR, p.id, p.offset, na->objSize, MPI_CHAR, na->win);
}

void narrayGetLine(narray* na, int index[], void** recv, int size)
{
	assert(na);
	assert(recv);
	assert(index);
	assert(size > 0);
	assert(size <= na->sizes[na->dimsCount - 1]);
	
	*recv = malloc(na->objSize * size);
	narrayGetLineInBuffer(na, index, *recv, size);
}

void narrayGetBlock(narray* na, int index[], void** recv, int sizes[])
{
	assert(na);
	assert(recv);
	assert(index);
	assert(sizes);
	
	int i;
	int blockSize = 1;
	
	for(i=0; i< na->dimsCount; i++)
	{
		assert((sizes[i] > 0) && (sizes[i] <= na->sizes[i]));
		blockSize *= sizes[i];
	}
	
	*recv = malloc(na->objSize * blockSize);
	
	narrayGetBlockInBuffer(na, index, *recv, sizes);
}

void narrayGetInBuffer(narray* na, int index[], void* recv)
{
	assert(na);
	assert(recv);
	
	npointer p = na->map(index, na);
	MPI_Get(recv, na->objSize, MPI_CHAR, p.id, p.offset, na->objSize, MPI_CHAR, na->win);
}

void narrayGetLineInBuffer(narray* na, int index[], void* recv, int size)
{
	assert(na);
	assert(recv);
	assert(index);
	assert(size > 0);
	assert(size <= na->sizes[na->dimsCount - 1]);
	
	int* index2 = malloc(sizeof(int) * na->dimsCount);
	int i;
	
	for(i=0; i<na->dimsCount; i++)
		index2[i] = index[i];
	index2[na->dimsCount - 1] = index[na->dimsCount - 1] + (size - 1);
	
	npointer p1 = na->map(index, na);
	npointer p2 = na->map(index2, na);
	
	if(p1.id == p2.id)
	{
		MPI_Get(recv, na->objSize * size, MPI_CHAR, p1.id, p1.offset, na->objSize * size, MPI_CHAR, na->win);
	}
	else
	{
		printf("extended get\n");
		
		int sndOffset = 0;
		int sndLength = 1;
		
		for(i=1; i<size; i++)
		{
			index2[na->dimsCount - 1] = index[na->dimsCount - 1] + i;
			p2 = na->map(index2, na);
			
			
			if(p1.id == p2.id)
			{
				sndLength++;
			}
			else
			{
				MPI_Get(recv + sndOffset * na->objSize, na->objSize * sndLength, MPI_CHAR, p1.id, p1.offset, na->objSize * sndLength, 	MPI_CHAR, na->win);
				//printf("got %d elems with offset = %d from %d:%d\n", sndLength, sndOffset, p1.id, p1.offset);
				p1 = na->map(index2, na);
				sndOffset += sndLength;
				sndLength = 1;
				
			}
		}
		
		MPI_Get(recv + sndOffset * na->objSize, na->objSize * sndLength, MPI_CHAR, p1.id, p1.offset, na->objSize * sndLength, 	MPI_CHAR, na->win);
		//printf("got %d elems with offset = %d from %d:%d\n", sndLength, sndOffset, p1.id, p1.offset);
	}
	
	free(index2);
}

void narrayGetBlockInBuffer(narray* na, int index[], void* recv, int sizes[])
{
	assert(na);
	assert(recv);
	assert(index);
	assert(sizes != NULL);
	
	int i;
	int j;
	
	for(i=0; i<na->dimsCount; i++)
		assert((sizes[i] > 0) && (sizes[i] <= na->sizes[i]));
	
	int* index2 = malloc(sizeof(int) * na->dimsCount);
	int linesCount = 1;
	int lineOffset = 0; // in terms of objects, not bytes
	
	for(i=0; i<na->dimsCount; i++)
	{
		index2[i] = index[i];
		linesCount *= sizes[i];
	}
	linesCount /= sizes[na->dimsCount - 1];
	
	for(i=0; i<linesCount; i++)
	{
		//make index2 point to line[i]
		int lineNum = i;
		
		for(j = na->dimsCount-2 ; j >= 0; j--)
		{
			index2[j] = (lineNum % sizes[j]);
			lineNum = lineNum / sizes[j];
		}
		index2[na->dimsCount - 1] = 0;
		
		for(j=0; j<na->dimsCount; j++)
		{
			index2[j] += index[j];
		}
		
		narrayGetLineInBuffer(na, index2, recv + lineOffset*na->objSize, sizes[na->dimsCount - 1]);
		lineOffset += sizes[na->dimsCount - 1];
	}
	
	free(index2);
}

void narrayFence(narray* na)
{
	assert(na);
	MPI_Win_fence(0, na->win);
}

void narrayBarrier(narray* na)
{
	assert(na);
	MPI_Barrier(na->comm);
}
