#include "ntree.h"


/*!
  \fn void ntreeFromNative(ntree * t)
  \brief Creates tree close to hardware. Has only one root with chils (one level deep tree)

  Реализация специфична для данной платформы
*/
void ntreeFromNative(ntree * t)
{
	assert(t);
	
	ntreeFromRange(t, MPI_COMM_WORLD, 0, NULL);
	
	t->topo.type = Tnone;
	t->topo.obj = NULL;
}

/*!
  \fn void ntreeFromRange(ntree * t, MPI_Comm comm, int pid, ntree * parent)
  \brief Creates one level tree close to hardware using communicator \a comm, root \a pid and parent tree \a parent

  Родитель фактически задан двумя способами - через ранк \a pid и дерево \a parent
*/
void ntreeFromRange(ntree * t, MPI_Comm comm, int pid, ntree * parent)
{
	assert(t);

	int i;

	MPI_Comm_dup(comm, &(t->comm));
	MPI_Comm_rank(t->comm, &(t->id));
	t->pid = pid;
	t->parent = parent;

	MPI_Comm_size(t->comm, &(t->num));

	/*t->nodes=malloc(sizeof(int) * t->num);
	for(i=0;i<t->num;i++)
		t->nodes[i]=i;*/

	t->topo.type = Tnone;
	t->topo.obj = NULL;

}

/*!
  \fn void ntreeFromfgrid2(ntree * t, fgrid2* fg)
  \brief Creates one level tree from two-dimensional grid

  Создается двухуровневое дерево, в первый уровень входит первый столбец (Y == 0), а на нижнем уровне - все строки.
*/
void ntreeFromfgrid2(ntree * t, fgrid2* fg)
{
	assert(t);
	assert(fg);
	ntree * pt = NULL;
	if(fg->y==0)
	{
		pt = malloc(sizeof(ntree));
		ntreeFromRange(pt, fg->yLine, 0, NULL);
		pt->topo.type = Tfgrid2;
		pt->topo.obj = fg;
	}

	ntreeFromRange(t, fg->xLine, 0, pt);
	t->topo.type = Tfgrid2;
	t->topo.obj = fg;
}

/*!
  \fn void ntreeFromfgrid2Trans(ntree * t,fgrid2* fg)
  \brief Creates one level tree from two-dimensional grid

  Создается двухуровневое дерево, в первый уровень входит первый строка (X == 0), а на нижнем уровне - все столбцы.
*/
void ntreeFromfgrid2Trans(ntree * t, fgrid2* fg)
{
	assert(t);
	assert(fg);
	
	ntree * pt = NULL;
	if(fg->x==0)
	{
		pt = malloc(sizeof(ntree));
		
		ntreeFromRange(pt, fg->xLine, 0, NULL);
		pt->topo.type = Tfgrid2;
		pt->topo.obj = fg;
	}
	
	ntreeFromRange(t, fg->yLine, 0, pt);
	
	t->topo.type = Tfgrid2;
	t->topo.obj = fg;
	
}

/*!
  \fn void ntreeFromfgrid2Param(ntree * t, fgrid2* fg, int orient, int toproot, int bottomroot)
  \brief Creates one level tree from two-dimensional grid, using special parameters

  Если \a orient == 0, то на первом уровне - столбец (на нижнем - строки), если нет - то на первом уровне строка (а на нижнем - столбцы).
  \a bottomroot - номер столбца (\a orient == 0) или строки (\a orient != 0), в которой находятся корни для нижнего уровня.
  \a toproot - позиция корневого процесса в этом столбце (или строке соответственно).
*/
void ntreeFromfgrid2Param(ntree * t, fgrid2* fg, int orient, int toproot, int bottomroot)
{
	assert(t);
	assert(fg);
	ntree * pt = NULL;
	
	//top bottom work
	if(!orient)
	{
		if(fg->y==bottomroot)
		{
			pt=malloc(sizeof(ntree));
			ntreeFromRange(pt, fg->yLine, toproot, NULL);
			pt->topo.type = Tfgrid2;
			pt->topo.obj = fg;
		}
		
		ntreeFromRange(t, fg->xLine, bottomroot, pt);
	}
	else
	{
		if(fg->x==bottomroot)
		{
			pt=malloc(sizeof(ntree));
			ntreeFromRange(pt, fg->xLine, toproot, NULL);
			pt->topo.type = Tfgrid2;
			pt->topo.obj = fg;
		}
		
		ntreeFromRange(t, fg->yLine, bottomroot, pt);
	}
	
	t->topo.type = Tfgrid2;
	t->topo.obj = fg;
}

/*!
  \fn void ntreeFromfgrid3(ntree * t, fgrid3* fg)
  \brief Creates three level tree from three-dimensional grid

  Главный корень имеет координаты (0, 0, 0)
  На втором уровне грань, у которой Z == 0
  На третьем уровне строка, у которой Z == 0 и
*/
void ntreeFromfgrid3(ntree * t, fgrid3* fg)
{
	assert(t);
	assert(fg);
	ntree * pt = NULL;
	ntree * ppt = NULL;
	
	if((fg->y==0) && (fg->z==0))
	{
		ppt = malloc(sizeof(ntree));
		ntreeFromRange(ppt, fg->yzLine, 0, NULL);
		ppt->topo.type = Tfgrid3;
		ppt->topo.obj = fg;
	}
	
	if(fg->z==0)
	{
		pt = malloc(sizeof(ntree));
		ntreeFromRange(pt, fg->xzLine, 0, ppt);
		pt->topo.type = Tfgrid3;
		pt->topo.obj = fg;
	}
	
	ntreeFromRange(t, fg->xyLine, 0, pt);
	t->topo.type = Tfgrid3;
	t->topo.obj = fg;
}

void ntreeFromfgridn(ntree* t, fgridn* fg, unsigned int dims[], int dimRoots[], int dimCount)
{
	assert(t);
	assert(fg);
	assert(dims);
	assert(dimRoots);
	assert(dimCount > 0);
	
	int i;
	ntree * temp = NULL;
	ntree * parent = NULL;
	
	unsigned int* inversedDims = malloc(sizeof(unsigned int) * dimCount);
	unsigned int fullMask = 0;
	
	for(i=0; i<dimCount; i++)
	{
		inversedDims[i] = fg->dimsMask ^ dims[i];
		if(fg->subDimsSelf[inversedDims[i]] == -1)
		{
			fgridnCreateSubDimMask(fg, inversedDims[i]); //mb repirt error instead
		}
	}
	
	int* isLevelRoot = malloc(sizeof(int) * dimCount);
	int flag = 1;
	for(i=dimCount-1; i>=0; i--)
	{
		isLevelRoot[i] = flag && (fg->subDimsSelf[inversedDims[i]] == dimRoots[i]);
		flag = isLevelRoot[i];
		int num;
		MPI_Comm_size(fg->subDims[inversedDims[i]], &num);
		//printf("%2d %s\tisLevelRoot[%2d] = %2d\t%2d ? %2d\t num = %2d\n", fg->id, fg->indexString, i, isLevelRoot[i], fg->subDimsSelf[inversedDims[i]], dimRoots[i], num);
	}
	
	for(i=1; i<dimCount; i++)
	{
		if(!isLevelRoot[i])
			continue;
		
		temp = malloc(sizeof(ntree));
		ntreeFromRange(temp, fg->subDims[inversedDims[i-1]], dimRoots[i-1], parent);
		temp->topo.type = Tfgridn;
		temp->topo.obj = fg;
		parent = temp;
	}
	
	ntreeFromRange(t, fg->subDims[inversedDims[dimCount-1]], dimRoots[dimCount-1], parent);
	t->topo.type = Tfgridn;
	t->topo.obj = fg;
	
	free(isLevelRoot);
}

/*!
  \fn void ntreeFree(ntree * t)
  \brief Frees tree \a t. Fills it's field with -1.

*/
void ntreeFree(ntree * t)
{
	assert(t);
	if(t->parent!=NULL)
	{
		ntreeFree(t->parent);
		free(t->parent);
	}
	MPI_Comm_free(&(t->comm));
	t->id = -1;
	t->pid = -1;
	t->num = -1;
	t->topo.type = Tnone;
	t->topo.obj = NULL;
}

/*!
  \fn void ntreeBCast(ntree * t,void* buffer, int count, MPI_Datatype datatype)
  \brief Broadcasts data over the tree

  \param t Tree to broadcast data
  \param buffer Data itself
  \param count Number of elements
  \param datatype Type of data

  This function is a wrapper over MPI broadcast
*/
void ntreeBCast(ntree * t, void* buffer, int count, MPI_Datatype datatype)
{
	assert(t);
	assert(buffer);
	assert(count>0);
	if((t->id==t->pid)&&(t->parent!=NULL))
	{
		ntreeBCast(t->parent, buffer, count, datatype);
	}
	MPI_Barrier(t->comm);
	MPI_Bcast(buffer, count, datatype, t->pid, t->comm );
	//printf("%sbcast called\n",((t->id==t->pid))?"root ":"");
}

/*!
  \fn void ntreeReduce(ntree * t, void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op)
  \brief Computes reduce operation over the tree

  \param t Target tree
  \param sendbuf Input data
  \param recvbuf Output data
  \param count Number of elements
  \param datatype Type of data
  \param op MPI reduce operation (i.e. sum, mul etc.)

  This function is a wrapper over MPI reduce
*/
void ntreeReduce(ntree * t, void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op)
{
	assert(t);
	assert(sendbuf);
	assert(recvbuf);
	assert(count>0);
	
	
	if((t->id==t->pid)&&(t->parent!=NULL))
	{
		int dataTypeSize;
		void* tempbuf;
		MPI_Type_size(datatype, &dataTypeSize);
		MPI_Alloc_mem( count*dataTypeSize, MPI_INFO_NULL, &tempbuf );
		
		MPI_Reduce(sendbuf, tempbuf, count, datatype, op, t->pid, t->comm);
		MPI_Barrier(t->comm);
		//printf("MPI_Reduce called root\n");
		ntreeReduce(t->parent, tempbuf, recvbuf, count, datatype, op);
		MPI_Free_mem(tempbuf);
	}
	else
	{
		MPI_Reduce(sendbuf, recvbuf, count, datatype, op, t->pid, t->comm);
		MPI_Barrier(t->comm);
		//printf("MPI_Reduce called\n");
	}
	
}

/*!
  \fn int ntreeGather(ntree * t, void *sendbuf, int sendcount, MPI_Datatype datatype, void **recvbuf)
  \brief Gathers data from tree

  \param t Target tree
  \param sendbuf Input data
  \param sendcount Number of elements
  \param datatype Type of data
  \param recvbuf Output data

  This function is a wrapper over MPI gather
*/
int ntreeGather(ntree * t, void *sendbuf, int sendcount, MPI_Datatype datatype, void **recvbuf)
{
	assert(t);
	assert(sendbuf);
	assert(recvbuf);
	assert(sendcount>0);
	
	int numNodes;
	int dataTypeSize;
	MPI_Type_size(datatype, &dataTypeSize);
	MPI_Comm_size(t->comm, &numNodes);
	
	if(t->id==t->pid)
		*recvbuf=malloc(numNodes*sendcount*dataTypeSize);
	
	MPI_Gather(sendbuf, sendcount, datatype, *recvbuf, /*numNodes**/sendcount, datatype, t->pid, t->comm);
	MPI_Barrier(t->comm);
	//printf("here(%d)\n",t->id);
	if((t->id==t->pid)&&(t->parent!=NULL))
	{
		void *oldrecvbuf = *recvbuf;
		int res;
		
		res = ntreeGather(t->parent, oldrecvbuf, numNodes*sendcount, datatype, recvbuf);
		free(oldrecvbuf);
		return res;
	}
	
	return numNodes*sendcount;
}

/*!
  \fn void ntreeScatter(ntree * t, void *sendbuf, int sendcount, MPI_Datatype datatype, void *recvbuf)
  \brief Scatters data over tree

  \param t Target tree
  \param sendbuf Input data
  \param sendcount Number of elements
  \param datatype Type of data
  \param recvbuf Output data

  This function is a wrapper over MPI scatter
*/
void ntreeScatter(ntree * t, void *sendbuf, int sendcount, MPI_Datatype datatype, void *recvbuf)
{
	assert(t);
	assert(sendbuf || (t->id!=t->pid));
	assert(recvbuf);
	assert(sendcount>0);
	
	int numNodes;
	int dataTypeSize;
	MPI_Type_size(datatype,&dataTypeSize);
	MPI_Comm_size(t->comm,&numNodes);
	
	if ((t->id==t->pid)&&(t->parent!=NULL))
	{
		void *tempbuf=NULL;//=malloc(numNodes*sendcount * dataTypeSize);
		MPI_Alloc_mem( numNodes*sendcount * dataTypeSize, MPI_INFO_NULL, &tempbuf );
		ntreeScatter(t->parent, sendbuf, sendcount*numNodes, datatype, tempbuf);
		MPI_Barrier(t->comm);
		MPI_Scatter(tempbuf, sendcount, datatype, recvbuf, sendcount, datatype, t->pid, t->comm);
		MPI_Free_mem(tempbuf);
	}
	else
	{
		MPI_Barrier(t->comm);
		MPI_Scatter(sendbuf, sendcount, datatype, recvbuf, sendcount, datatype, t->pid, t->comm);
	}
	
}

/*!
  \fn int ntreeIsMegaRoot(ntree * t)
  \brief Returns 1 if t is mega root

*/
int ntreeIsMegaRoot(ntree * t)
{
	assert(t);
	while(t->parent!=NULL)
		t = t->parent;
	return t->id==t->pid;
}

/*!
 \fn void ntreeBarrier(ntree * t)
 \brief Locks barrier on the whole tree \a t

*/
void ntreeBarrier(ntree * t)
{
	assert(t);
	if(t->parent!=NULL)
		ntreeBarrier(t->parent);
	MPI_Barrier(t->comm);
}
