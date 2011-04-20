#include "fgridn.h"

void fgridnCreateSubDim(fgridn * g, int* dims)
{
	assert(g);
	assert(dims);
	
	fgridnCreateSubDimMask(g, fgridnBoolArrayToMask(g, dims));
}

void fgridnCreateSubDimMask(fgridn * g, unsigned int dims)
{
	assert(g);
	
	if(g->subDimsSelf[dims] != -1)
		return;
	
	int flag=0;
	int i;
	int d=1;
	
	for(i=0; i < g->dimsCount; i++)
		if(dims & 1<<i)
		{
			flag += g->index[i] * d;
			d *= g->sizes[i];
		}
	
	MPI_Comm_split(g->comm, flag, 0, &(g->subDims[dims]));
	MPI_Comm_rank(g->subDims[dims], &(g->subDimsSelf[dims]));
}

int fgridn_native(int* index, fgridn *g)
{
	assert(index);
	assert(g);
	
	int i;
	int result = 0;
	for(i=0; i < (g->dimsCount - 1); i++)
	{
		result += index[i];
		result *= g->sizes[i+1];
	}
	result += index[i];
	
	return result;
}

void fgridn_reverse(fgridn *g)
{
	assert(g);
	
	int* sizes = g->sizes;
	int dimCount = g->dimsCount;
	
	int i;
	g->index = malloc(sizeof(int) * dimCount);
	
	int id = g->id;
	
	for(i = dimCount-1 ; i >= 0; i--)
	{
		g->index[i] = (id % sizes[i]);
		id = id / sizes[i];
	}
}


void fgridnFromRange(fgridn * g, MPI_Comm comm, int dimsCount, int* sizes, int(*map)(int*, fgridn *), void(*reverse)(fgridn *))
{
	assert(g);
	assert(sizes);
	assert(dimsCount>0);

	int i;
	g->totalSize = 1;
	g->dimsCount = dimsCount;
	g->sizes = malloc(sizeof(int) * dimsCount);
	for(i=0; i<dimsCount; i++)
	{
		assert(sizes[i]>0);
		g->totalSize *= sizes[i];
		g->sizes[i] = sizes[i];
	}
	
	int numNodes;
	MPI_Comm_size(comm,&numNodes);
	assert(g->totalSize == numNodes);


	MPI_Comm_dup(comm, &(g->comm));
	MPI_Comm_rank(g->comm, &(g->id));

	g->topo.obj=NULL;
	g->topo.type=Tnone;

	g->reverse=reverse;
	g->reverse(g);
	g->map=map;
	
	g->subdimsCount = 1<<(dimsCount);
	g->subDims = malloc(sizeof(MPI_Comm) * g->subdimsCount);
	g->subDimsSelf = malloc(sizeof(int) * g->subdimsCount);
	for(i=0; i < g->subdimsCount; i++)
	{
		g->subDimsSelf[i] = -1;
		//printf("here %d\n", g->subdimsCount);
	}
	
	fgridnCreateSubDimMask(g, 0);
}

void fgridnFromNative(fgridn * g)
{
	int numNodes;
	MPI_Comm_size(MPI_COMM_WORLD,&numNodes);
	fgridnFromRange(g, MPI_COMM_WORLD, 1, &numNodes, (int(*)(int*, fgridn *)) (fgridn_native), (void(*)(fgridn *)) (fgridn_reverse));
}


void fgridnSlice(fgridn * g,fgridn * ng, int dim, int sliceIndex)
{
	assert(g);
	assert(ng);
	assert((dim >= 0) && (dim < g->dimsCount));
	
	assert(sliceIndex > 0);
	assert(sliceIndex < g->sizes[dim]);
	
	int i;
	ng->dimsCount = g->dimsCount;
	ng->sizes = malloc(sizeof(int) * ng->dimsCount);
	for(i=0; i < g->dimsCount; i++)
		if(i == dim)
		{
			if(g->index[i] < sliceIndex)
				ng->sizes[i] = sliceIndex;
			else
				ng->sizes[i] = g->sizes[i] - sliceIndex;
		}
		else
		{
			ng->sizes[i] = g->sizes[i];
		}
	MPI_Comm_split(g->comm, g->index[dim] <sliceIndex, 0, &(ng->comm));
	MPI_Comm_rank(ng->comm, &(ng->id));
	
	ng->totalSize = 1;
	for(i=0; i<ng->dimsCount; i++)
	{
		assert(ng->sizes[i]>0);
		ng->totalSize *= ng->sizes[i];
	}
	
	ng->topo.obj = g;
	ng->topo.type = Tfgridn;
	
	ng->reverse = g->reverse;
	ng->reverse(ng);
	ng->map = g->map;
	
	
	ng->subdimsCount = g->subdimsCount;
	ng->subDims = malloc(sizeof(MPI_Comm) * ng->subdimsCount);
	ng->subDimsSelf = malloc(sizeof(int) * ng->subdimsCount);
	
	for(i=0; i< ng->subdimsCount; i++)
	{
		ng->subDimsSelf[i] = -1;
		if(g->subDimsSelf[i] != -1)
			fgridnCreateSubDimMask(ng, i);
	}
}

void fgridnSliceLinear(fgridn * g,fgridn * ng, int* sliceSteps)
{
	assert(g);
	assert(ng);
	assert(sliceSteps);
	
	int i;
	ng->dimsCount = g->dimsCount;
	ng->totalSize = 1;
	ng->sizes = malloc(sizeof(int) * ng->dimsCount);
	for(i=0; i < g->dimsCount; i++)
	{
		assert(sliceSteps[i] > 0);
		assert((g->sizes[i] % sliceSteps[i]) == 0);
		ng->sizes[i] = sliceSteps[i];
		ng->totalSize *= ng->sizes[i];
	}
	
	
	int flag = 0;
	int d = 1;
	for(i=0; i < g->dimsCount; i++)
	{
		flag += (g->index[i] / sliceSteps[i])*d;
		//flag *= d;
		d *= g->sizes[i] / sliceSteps[i];
	}
	
	MPI_Comm_split(g->comm, flag, 0, &(ng->comm));
	MPI_Comm_rank(ng->comm, &(ng->id));
	
	ng->topo.obj=g;
	ng->topo.type=Tfgridn;
	
	ng->reverse = g->reverse;
	ng->reverse(ng);
	ng->map = g->map;
	
	ng->subdimsCount = g->subdimsCount;
	ng->subDims = malloc(sizeof(MPI_Comm) * ng->subdimsCount);
	ng->subDimsSelf = malloc(sizeof(int) * ng->subdimsCount);
	
	for(i=0; i< ng->subdimsCount; i++)
	{
		ng->subDimsSelf[i] = -1;
		if(g->subDimsSelf[i] != -1)
			fgridnCreateSubDimMask(ng, i);
	}
}

void fgridnSliceParts(fgridn * g, fgridn * ng, int** parts, int* size)
{
	assert(g);
	assert(ng);
	assert(parts);
	assert(size);
	
	int i, j, tmpSum=0;
	for(i=0; i < g->dimsCount; i++)
	{
		assert(parts[i]);
		assert(size[i]>0);
		tmpSum = 0;
		for(j=0; j< size[i]; j++)
			tmpSum += parts[i][j];
		assert(tmpSum == g->sizes[i]);
	}
	
	ng->dimsCount = g->dimsCount;
	ng->totalSize = 1;
	ng->sizes = malloc(sizeof(int) * ng->dimsCount);
	int* colors = malloc(sizeof(int) * g->dimsCount);
	
	for(i=0; i < g->dimsCount; i++)
	{
		colors[i]=0;
		tmpSum = 0;
		for(j=0; j < size[i]; j++)
		{
			tmpSum += parts[i][j];
			if(tmpSum > g->index[i])
			{
				colors[i] = j;
				break;
			}
		}
		
	}
	
	int resultColor = 0;
	int d = 1;
	for(i=0; i < g->dimsCount; i++)
	{
		
		resultColor += colors[i]*d;
		d *= size[i];
		
		ng->sizes[i] = parts[i][colors[i]];
		ng->totalSize *= ng->sizes[i];
	}
	
	MPI_Comm_split(g->comm, resultColor, 0, &(ng->comm));
	MPI_Comm_rank(ng->comm,&(ng->id));
	
	ng->topo.obj=g;
	ng->topo.type=Tfgridn;
	
	ng->reverse=g->reverse;
	ng->reverse(ng);
	ng->map=g->map;
	
	ng->subdimsCount = g->subdimsCount;
	ng->subDims = malloc(sizeof(MPI_Comm) * ng->subdimsCount);
	ng->subDimsSelf = malloc(sizeof(int) * ng->subdimsCount);
	
	for(i=0; i< ng->subdimsCount; i++)
	{
		ng->subDimsSelf[i] = -1;
		if(g->subDimsSelf[i] != -1)
			fgridnCreateSubDimMask(ng, i);
	}
	
	free(colors);
}

void fgridnDivide(fgridn * g,fgridn * ng, int dim, int divideStep)
{
	assert(g);
	assert(ng);
	assert((dim >= 0) && (dim < g->dimsCount));
	assert((g->sizes[dim] % divideStep) == 0);
	
	int i;
	ng->dimsCount = g->dimsCount + 1;
	ng->sizes = malloc(sizeof(int) * ng->dimsCount);
	
	for(i=0; i < g->dimsCount; i++)
		if(i == dim)
		{
			if(g->index[i] < sliceIndex)
				ng->sizes[i] = sliceIndex;
			else
				ng->sizes[i] = g->sizes[i] - sliceIndex;
		}
		else
		{
			ng->sizes[i] = g->sizes[i];
		}
}

void fgridnFree(fgridn * g)
{
	assert(g);
	
	int i;
	for(i=0; i < g->subdimsCount; i++)
		if(g->subDimsSelf[i] != -1)
		{
			//printf("SIZES %d, %d\n", g->sizes[0], g->sizes[1]);
			MPI_Comm_free(&(g->subDims[i]));
			g->subDimsSelf[i] = -1;
		}
	
	g->id = -1;
	free(g->sizes);
	g->sizes = 0;
	g->totalSize = -1;
	free(g->index);
	g->index = 0;
	
	g->map = NULL;
	g->reverse = NULL;
	
	g->topo.type = Tnone;
	g->topo.obj = NULL;
	
	MPI_Comm_free(&(g->comm));
}


void fgridnBarrier(fgridn * g)
{
	assert(g);
	MPI_Barrier(g->comm);
}

void fgridnBarrierSub(fgridn * g, int* dims)
{
	fgridnBarrierSubMask(g, fgridnBoolArrayToMask(g, dims));
}

void fgridnBarrierSubMask(fgridn * g, unsigned int dims)
{
	assert(g);
		
	if(g->subDimsSelf[dims] != -1)
		MPI_Barrier(g->subDims[dims]);
}

unsigned int fgridnBoolArrayToMask(fgridn * g, int* dims)
{
	assert(g);
	assert(dims);
	
	int i;
	unsigned int mask=0;
	for(i=0; i < g->dimsCount; i++)
		if(dims[i])
			mask = mask | 1<<i;
	
		return mask;
}
