#include "ntree.h"

void ntreeFromNative(ntree * t)
{
	assert(t);
	
	ntreeFromRange(t,MPI_COMM_WORLD,0,NULL);
	
	t->topo.type=Tnone;
	t->topo.obj=NULL;
}

void ntreeFromfgrid2Trans(ntree * t,fgrid2* fg)
{
	assert(t);
	assert(fg);
	
	ntree * pt=NULL;
	if(fg->x==0)
	{
		pt=malloc(sizeof(ntree));
		
		ntreeFromRange(pt,fg->xLine,0,NULL);
		pt->topo.type=Tfgrid2;
		pt->topo.obj=fg;
	}
	
	ntreeFromRange(t,fg->yLine,0,pt);
	
	t->topo.type=Tfgrid2;
	t->topo.obj=fg;
	
}

void ntreeFromfgrid2Param(ntree * t,fgrid2* fg,int orient,int toproot, int bottomroot)
{
	assert(t);
	assert(fg);
	ntree * pt=NULL;
	
	//top bottom work
	if(!orient)
	{
		if(fg->y==bottomroot)
		{
			pt=malloc(sizeof(ntree));
			ntreeFromRange(pt,fg->yLine,toproot,NULL);
			pt->topo.type=Tfgrid2;
			pt->topo.obj=fg;
		}
		
		ntreeFromRange(t,fg->xLine,bottomroot,pt);
	}
	else
	{
		if(fg->x==bottomroot)
		{
			pt=malloc(sizeof(ntree));
			ntreeFromRange(pt,fg->xLine,toproot,NULL);
			pt->topo.type=Tfgrid2;
			pt->topo.obj=fg;
		}
		
		ntreeFromRange(t,fg->yLine,bottomroot,pt);
	}
	
	t->topo.type=Tfgrid2;
	t->topo.obj=fg;
}

void ntreeFromfgrid2(ntree * t,fgrid2* fg)
{
	assert(t);
	assert(fg);
	ntree * pt=NULL;
	if(fg->y==0)
	{
		pt=malloc(sizeof(ntree));
		ntreeFromRange(pt,fg->yLine,0,NULL);
		pt->topo.type=Tfgrid2;
		pt->topo.obj=fg;
	}
	
	ntreeFromRange(t,fg->xLine,0,pt);
	t->topo.type=Tfgrid2;
	t->topo.obj=fg;
}

void ntreeFromfgrid3(ntree * t,fgrid3* fg)
{
	assert(t);
	assert(fg);
	ntree * pt=NULL;
	ntree * ppt=NULL;
	
	
	if((fg->y==0) && (fg->z==0))
	{
		ppt=malloc(sizeof(ntree));
		ntreeFromRange(ppt,fg->yzLine,0,NULL);
		ppt->topo.type=Tfgrid3;
		ppt->topo.obj=fg;
	}
	
	if(fg->z==0)
	{
		pt=malloc(sizeof(ntree));
		ntreeFromRange(pt,fg->xzLine,0,ppt);
		pt->topo.type=Tfgrid3;
		pt->topo.obj=fg;
	}
	
	ntreeFromRange(t,fg->xyLine,0,pt);
	t->topo.type=Tfgrid3;
	t->topo.obj=fg;
}

void ntreeFromRange(ntree * t,MPI_Comm comm,int pid, ntree * parent)
{
	assert(t);

	int i;
	
	MPI_Comm_dup(comm, &(t->comm));
	MPI_Comm_rank(t->comm,&(t->id));
	t->pid=pid;
	t->parent=parent;
	
	MPI_Comm_size(t->comm,&(t->num));
	
	/*t->nodes=malloc(sizeof(int) * t->num);
	for(i=0;i<t->num;i++)
		t->nodes[i]=i;*/
	
	t->topo.type=Tnone;
	t->topo.obj=NULL;
	
}

void ntreeFree(ntree * t)
{
	assert(t);
	if(t->parent!=NULL)
	{
		ntreeFree(t->parent);
		free(t->parent);
	}
	MPI_Comm_free(&(t->comm));
	t->id=-1;
	t->pid=-1;
	t->num=-1;
	t->topo.type=Tnone;
	t->topo.obj=NULL;
}

void ntreeBCast(ntree * t,void* buffer, int count, MPI_Datatype datatype)
{
	assert(t);
	assert(buffer);
	assert(count>0);
	if((t->id==t->pid)&&(t->parent!=NULL))
	{
		ntreeBCast(t->parent,buffer,count,datatype);
	}
	MPI_Barrier(t->comm);
	MPI_Bcast(buffer, count, datatype, t->pid, t->comm );
	//printf("%sbcast called\n",((t->id==t->pid))?"root ":"");
}

void ntreeReduce(ntree * t,void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op)
{
	assert(t);
	assert(sendbuf);
	assert(recvbuf);
	assert(count>0);
	
	
	if((t->id==t->pid)&&(t->parent!=NULL))
	{
		int dataTypeSize;
		void* tempbuf;
		MPI_Type_size(datatype,&dataTypeSize);
		MPI_Alloc_mem( count*dataTypeSize, MPI_INFO_NULL, &tempbuf );
		
		MPI_Reduce(sendbuf, tempbuf, count, datatype, op, t->pid, t->comm);
		MPI_Barrier(t->comm);
		//printf("MPI_Reduce called root\n");
		ntreeReduce(t->parent,tempbuf,recvbuf,count,datatype,op);
		MPI_Free_mem(tempbuf);
	}
	else
	{
		MPI_Reduce(sendbuf, recvbuf, count, datatype, op, t->pid, t->comm);
		MPI_Barrier(t->comm);
		//printf("MPI_Reduce called\n");
	}
	
}

int ntreeGather(ntree * t,void *sendbuf, int sendcount, MPI_Datatype datatype, void **recvbuf)
{
	assert(t);
	assert(sendbuf);
	assert(recvbuf);
	assert(sendcount>0);
	
	int numNodes;
	int dataTypeSize;
	MPI_Type_size(datatype,&dataTypeSize);
	MPI_Comm_size(t->comm,&numNodes);
	
	if(t->id==t->pid)
		*recvbuf=malloc(numNodes*sendcount*dataTypeSize);
	
	MPI_Gather(sendbuf, sendcount, datatype, *recvbuf, /*numNodes**/sendcount, datatype, t->pid, t->comm);
	MPI_Barrier(t->comm);
	//printf("here(%d)\n",t->id);
	if((t->id==t->pid)&&(t->parent!=NULL))
	{
		void *oldrecvbuf=*recvbuf;
		int res;
		
		res = ntreeGather(t->parent,oldrecvbuf,numNodes*sendcount,datatype,recvbuf);
		free(oldrecvbuf);
		return res;
	}
	
	return numNodes*sendcount;
}

 void ntreeScatter(ntree * t,void *sendbuf, int sendcount, MPI_Datatype datatype, void *recvbuf)
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
		ntreeScatter(t->parent,sendbuf,sendcount*numNodes,datatype,tempbuf);
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

int ntreeIsMegaRoot(ntree * t)
{
	assert(t);
	while(t->parent!=NULL)
		t=t->parent;
	return t->id==t->pid;
}

void ntreeBarrier(ntree * t)
{
	assert(t);
	if(t->parent!=NULL)
		ntreeBarrier(t->parent);
	MPI_Barrier(t->comm);
}
